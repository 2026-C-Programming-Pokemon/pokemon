"""FastAPI 프록시 단위 테스트.

테스트 전략:
- 업스트림은 httpx MockTransport 로 가짜 OpenAI 응답을 돌려준다.
- 정상 흐름, 서명 누락, 타임스탬프 stale, nonce 리플레이, 본문 크기, 모델 화이트리스트,
  레이트리밋, User-Agent 거절 케이스를 모두 검증.
"""
from __future__ import annotations

import json
import os
import time
from typing import Iterator

import httpx
import pytest
from fastapi.testclient import TestClient

# 테스트 환경: 설정 캐시가 깨끗하게 시작하도록 모듈 import 전에 env 를 깐다.
TEST_ENV = {
    "UPSTREAM_API_KEY": "sk-fake-upstream-key",
    "UPSTREAM_BASE_URL": "http://upstream.invalid/v1",
    "POKEMON_CLIENT_SECRET": "test-secret-not-real",
    "PROXY_BIND_HOST": "127.0.0.1",
    "PROXY_BIND_PORT": "0",
    "REQUEST_FRESHNESS_WINDOW_S": "60",
    "NONCE_CACHE_TTL_S": "300",
    "MAX_BODY_BYTES": "4096",
    "RATE_LIMIT_PER_IP_PER_MIN": "5",
    "RATE_LIMIT_PER_CLIENT_PER_MIN": "5",
    "RATE_LIMIT_BURST": "0",
    "ALLOWED_MODELS": "gpt-4o-mini,gpt-4o",
    "EXPECTED_USER_AGENT_PREFIX": "pokemon-c-client",
    "TRUST_X_FORWARDED_FOR": "false",
    "LOG_LEVEL": "WARNING",
}


@pytest.fixture(autouse=True)
def _patch_env(monkeypatch: pytest.MonkeyPatch) -> Iterator[None]:
    for k, v in TEST_ENV.items():
        monkeypatch.setenv(k, v)
    # 캐시된 설정/앱 모듈을 매 테스트에서 다시 로드.
    from app import app as app_module
    from app import config as config_module

    config_module.get_settings.cache_clear()
    # app 모듈 안의 모듈-레벨 app 인스턴스는 import 시점에 생성되어 있으므로
    # 새 설정으로 신규 인스턴스를 만들어 사용.
    fresh_app = app_module.create_app(config_module.get_settings())
    monkeypatch.setattr(app_module, "app", fresh_app)
    yield


def _make_app_with_upstream(handler):
    """업스트림 응답을 흉내내는 httpx MockTransport 를 끼운 새 앱."""
    from app import app as app_module
    from app import config as config_module

    settings = config_module.get_settings()
    application = app_module.create_app(settings)

    transport = httpx.MockTransport(handler)
    # AsyncClient 를 mock transport 로 교체.
    new_client = httpx.AsyncClient(transport=transport, timeout=5.0)
    # 기존 클라이언트는 깔끔하게 닫아두진 못해도 테스트 한정이라 무시.
    application.state.http_client = new_client
    return application


def _signed_headers(body: bytes, ts: int | None = None, nonce: str | None = None):
    from app.security import compute_signature

    ts_str = str(ts if ts is not None else int(time.time()))
    n = nonce or "0123456789abcdef0123456789abcdef"
    sig = compute_signature(os.environ["POKEMON_CLIENT_SECRET"], ts_str, n, body)
    return {
        "Content-Type": "application/json",
        "User-Agent": "pokemon-c-client/0.1 test",
        "X-Pokemon-Client-Id": "pokemon-c-client/0.1",
        "X-Pokemon-Timestamp": ts_str,
        "X-Pokemon-Nonce": n,
        "X-Pokemon-Signature": sig,
    }


def _body(model: str = "gpt-4o-mini", prompt: str = "hi") -> bytes:
    return json.dumps(
        {
            "model": model,
            "max_tokens": 16,
            "messages": [{"role": "user", "content": prompt}],
        }
    ).encode("utf-8")


def _fake_upstream_ok(request: httpx.Request) -> httpx.Response:
    assert request.headers["authorization"].startswith("Bearer ")
    assert request.url.path == "/v1/chat/completions"
    return httpx.Response(
        200,
        json={
            "id": "chatcmpl-fake",
            "object": "chat.completion",
            "choices": [
                {
                    "index": 0,
                    "message": {"role": "assistant", "content": "ok"},
                    "finish_reason": "stop",
                }
            ],
        },
    )


def test_healthz_ok():
    from app import app as app_module

    with TestClient(app_module.app) as c:
        r = c.get("/healthz")
        assert r.status_code == 200
        assert r.json()["status"] == "ok"


def test_forward_buffered_ok():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body()
        r = c.post(
            "/v1/chat/completions", content=body, headers=_signed_headers(body)
        )
        assert r.status_code == 200, r.text
        j = r.json()
        assert j["choices"][0]["message"]["content"] == "ok"


def test_missing_signature_rejected():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body()
        r = c.post(
            "/v1/chat/completions",
            content=body,
            headers={
                "Content-Type": "application/json",
                "User-Agent": "pokemon-c-client/0.1",
            },
        )
        assert r.status_code == 401
        assert r.json()["error"]["code"] == "missing_signature_headers"


def test_stale_timestamp_rejected():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body()
        headers = _signed_headers(body, ts=int(time.time()) - 600)
        r = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r.status_code == 401
        assert r.json()["error"]["code"] == "stale_timestamp"


def test_signature_tamper_rejected():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body()
        headers = _signed_headers(body)
        # 본문만 슬쩍 바꿔치기.
        tampered = body.replace(b"hi", b"HI")
        r = c.post("/v1/chat/completions", content=tampered, headers=headers)
        assert r.status_code == 401
        assert r.json()["error"]["code"] == "signature_mismatch"


def test_replay_rejected():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body()
        headers = _signed_headers(body)
        r1 = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r1.status_code == 200
        # 동일 nonce 재사용 → 거절.
        r2 = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r2.status_code == 401
        assert r2.json()["error"]["code"] == "nonce_replayed"


def test_body_too_large():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        big_prompt = "x" * (4096 + 100)
        body = _body(prompt=big_prompt)
        headers = _signed_headers(body)
        r = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r.status_code == 413
        assert r.json()["error"]["code"] == "body_too_large"


def test_model_not_in_allowlist():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body(model="gpt-5-illegal")
        headers = _signed_headers(body)
        r = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r.status_code == 400
        assert r.json()["error"]["code"] == "model_not_allowed"


def test_user_agent_prefix_enforced():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        body = _body()
        headers = _signed_headers(body)
        headers["User-Agent"] = "curl/8.5"
        r = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r.status_code == 403
        assert r.json()["error"]["code"] == "user_agent_rejected"


def test_rate_limit_kicks_in():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        # 매 호출 nonce 만 바꿔서 시그니처가 유효한 새 요청을 5번 + 1번 시도.
        for i in range(5):
            body = _body(prompt=f"prompt-{i}")
            n = f"{i:032x}"
            r = c.post("/v1/chat/completions", content=body, headers=_signed_headers(body, nonce=n))
            assert r.status_code == 200, (i, r.status_code, r.text)
        body = _body(prompt="prompt-overflow")
        r = c.post(
            "/v1/chat/completions",
            content=body,
            headers=_signed_headers(body, nonce="ffffffffffffffffffffffffffffffff"),
        )
        assert r.status_code == 429
        assert r.json()["error"]["code"] in {"rate_limited_ip", "rate_limited_client"}


def test_xff_ignored_by_default_and_remote_ip_rate_limit_applies():
    application = _make_app_with_upstream(_fake_upstream_ok)
    with TestClient(application) as c:
        # 서로 다른 XFF를 보내도 기본값(false)에서는 socket peer IP 기준으로 묶여야 한다.
        for i in range(5):
            body = _body(prompt=f"xff-{i}")
            headers = _signed_headers(body, nonce=f"{i:032x}")
            headers["X-Forwarded-For"] = f"203.0.113.{i+1}"
            r = c.post("/v1/chat/completions", content=body, headers=headers)
            assert r.status_code == 200, (i, r.status_code, r.text)

        body = _body(prompt="xff-overflow")
        headers = _signed_headers(body, nonce="ffffffffffffffffffffffffffffffff")
        headers["X-Forwarded-For"] = "198.51.100.99"
        r = c.post("/v1/chat/completions", content=body, headers=headers)
        assert r.status_code == 429
        assert r.json()["error"]["code"] in {"rate_limited_ip", "rate_limited_client"}



def test_upstream_error_maps_to_502():
    def boom(request: httpx.Request) -> httpx.Response:
        raise httpx.ConnectError("nope", request=request)

    application = _make_app_with_upstream(boom)
    with TestClient(application) as c:
        body = _body()
        r = c.post("/v1/chat/completions", content=body, headers=_signed_headers(body))
        assert r.status_code == 502
        assert r.json()["error"]["code"] == "upstream_error"
