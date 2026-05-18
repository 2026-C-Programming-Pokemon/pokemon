"""FastAPI 프록시 본체.

엔드포인트:
    GET  /healthz                       → liveness 체크
    POST /v1/chat/completions           → 업스트림으로 포워딩 (OpenAI 호환)

상세 요청 검증 순서:
    1. Content-Length / 본문 크기 캡
    2. (옵션) User-Agent prefix 체크
    3. 시그니처 헤더 4종 존재 확인
    4. 타임스탬프 freshness window 검증
    5. nonce 리플레이 검증
    6. HMAC 서명 일치 검증
    7. 클라이언트 ID 화이트리스트 (옵션)
    8. per-IP / per-client rate limit
    9. JSON 파싱 + (옵션) 모델 화이트리스트
   10. 업스트림 포워딩, 응답 그대로 반환 (stream 도 지원)
"""
from __future__ import annotations

import contextlib
import json
import logging
import time
from typing import Any, AsyncIterator

import httpx
from fastapi import FastAPI, Request, Response
from fastapi.responses import JSONResponse, StreamingResponse

from .config import Settings, get_settings
from .security import (
    HEADER_CLIENT_ID,
    HEADER_NONCE,
    HEADER_SIGNATURE,
    HEADER_TIMESTAMP,
    NonceCache,
    SlidingWindowRateLimiter,
    compute_signature,
    constant_time_equal,
)

logger = logging.getLogger("pokemon_proxy")


def _err(status: int, code: str, message: str) -> JSONResponse:
    return JSONResponse(
        status_code=status,
        content={"error": {"code": code, "message": message}},
    )


def _client_ip(request: Request, settings: Settings) -> str:
    # 기본값은 socket peer IP. 신뢰 가능한 리버스 프록시 뒤에서만
    # X-Forwarded-For 사용을 명시적으로 켠다.
    if settings.trust_x_forwarded_for:
        xff = request.headers.get("x-forwarded-for")
        if xff:
            return xff.split(",")[0].strip()
    if request.client is not None:
        return request.client.host
    return "unknown"


def _redact_header_for_log(name: str, value: str) -> str:
    lname = name.lower()
    if lname in {"authorization", "x-pokemon-signature", "cookie"}:
        return "<redacted>"
    return value


def create_app(settings: Settings | None = None) -> FastAPI:
    settings = settings or get_settings()

    logging.basicConfig(
        level=getattr(logging, settings.log_level, logging.INFO),
        format="%(asctime)s %(levelname)s %(name)s %(message)s",
    )

    if not settings.upstream_api_key:
        logger.warning(
            "UPSTREAM_API_KEY 가 비어 있습니다. 업스트림 호출이 실패합니다."
        )
    if settings.require_signature and not settings.client_shared_secret:
        logger.error(
            "POKEMON_CLIENT_SECRET 이 비어 있습니다. 서명 검증을 강제 활성화한 "
            "상태이므로 모든 요청이 401 로 거절됩니다."
        )

    ip_limiter = SlidingWindowRateLimiter(
        settings.rate_limit_per_ip_per_min, settings.rate_limit_burst
    )
    client_limiter = SlidingWindowRateLimiter(
        settings.rate_limit_per_client_per_min, settings.rate_limit_burst
    )
    nonce_cache = NonceCache(settings.nonce_cache_ttl_s)

    http_client = httpx.AsyncClient(
        timeout=httpx.Timeout(settings.upstream_timeout_s),
        limits=httpx.Limits(max_connections=32, max_keepalive_connections=8),
    )

    @contextlib.asynccontextmanager
    async def _lifespan(_app: FastAPI) -> AsyncIterator[None]:
        try:
            yield
        finally:
            await _app.state.http_client.aclose()

    app = FastAPI(
        title="pokemon-llm-proxy",
        version="0.1.0",
        docs_url=None,  # 외부 노출 시 스키마 페이지 자동 노출 방지.
        redoc_url=None,
        openapi_url=None,
        lifespan=_lifespan,
    )

    # 핸들러에서 참조할 수 있게 state 에 부착.
    app.state.settings = settings
    app.state.ip_limiter = ip_limiter
    app.state.client_limiter = client_limiter
    app.state.nonce_cache = nonce_cache
    app.state.http_client = http_client

    @app.get("/healthz")
    async def healthz() -> dict[str, Any]:
        return {"status": "ok", "service": "pokemon-llm-proxy"}

    @app.post("/v1/chat/completions")
    async def chat_completions(request: Request) -> Response:
        return await _handle_chat_completions(request, app)

    return app


async def _handle_chat_completions(request: Request, app: FastAPI) -> Response:
    settings: Settings = app.state.settings
    ip_limiter: SlidingWindowRateLimiter = app.state.ip_limiter
    client_limiter: SlidingWindowRateLimiter = app.state.client_limiter
    nonce_cache: NonceCache = app.state.nonce_cache
    http_client: httpx.AsyncClient = app.state.http_client

    ip = _client_ip(request, settings)

    # 1) 본문 크기 캡 — Content-Length 가 거짓일 수도 있으니 실제 읽은 길이도 본다.
    declared_len = request.headers.get("content-length")
    if declared_len is not None:
        try:
            if int(declared_len) > settings.max_body_bytes:
                return _err(413, "body_too_large", "request body too large")
        except ValueError:
            return _err(400, "bad_content_length", "invalid content-length")

    body = await request.body()
    if len(body) > settings.max_body_bytes:
        return _err(413, "body_too_large", "request body too large")
    if not body:
        return _err(400, "empty_body", "request body required")

    # 2) User-Agent prefix (옵션). 클라이언트 바이너리 식별 컨벤션.
    if settings.user_agent_prefix:
        ua = request.headers.get("user-agent", "")
        if not ua.startswith(settings.user_agent_prefix):
            logger.info("ua_mismatch ip=%s ua=%r", ip, ua[:64])
            return _err(403, "user_agent_rejected", "unexpected user-agent")

    # 3-6) 서명 검증.
    if settings.require_signature:
        sig_err = _verify_signature(request, body, settings, nonce_cache)
        if sig_err is not None:
            status, code, message = sig_err
            logger.info("sig_reject ip=%s code=%s", ip, code)
            return _err(status, code, message)

    # 7) 클라이언트 ID 화이트리스트 (옵션).
    client_id = request.headers.get(HEADER_CLIENT_ID, "")
    if settings.allowed_client_ids and client_id not in settings.allowed_client_ids:
        return _err(403, "client_id_rejected", "client id not allowed")

    # 8) 레이트리밋.
    now = time.time()
    if not ip_limiter.hit(ip, now=now):
        return _err(429, "rate_limited_ip", "too many requests from this IP")
    rate_key = client_id or ip
    if not client_limiter.hit(rate_key, now=now):
        return _err(429, "rate_limited_client", "too many requests for this client")

    # 9) JSON 파싱 + 모델 화이트리스트.
    try:
        payload = json.loads(body)
    except json.JSONDecodeError:
        return _err(400, "bad_json", "request body must be valid json")
    if not isinstance(payload, dict):
        return _err(400, "bad_json", "request body must be a json object")

    model = payload.get("model")
    if settings.allowed_models:
        if not isinstance(model, str) or model not in settings.allowed_models:
            return _err(400, "model_not_allowed", "requested model is not allowed")

    is_stream = bool(payload.get("stream"))

    # 10) 포워딩.
    if not settings.upstream_api_key:
        return _err(503, "upstream_unconfigured", "upstream key not configured")

    upstream_url = f"{settings.upstream_base_url}/chat/completions"
    upstream_headers = {
        "Authorization": f"Bearer {settings.upstream_api_key}",
        "Content-Type": "application/json",
        "Accept": request.headers.get("accept", "application/json"),
    }

    logger.info(
        "forward ip=%s client_id=%s model=%s body_bytes=%d stream=%s",
        ip,
        client_id or "-",
        model if isinstance(model, str) else "-",
        len(body),
        is_stream,
    )

    if is_stream:
        return await _forward_stream(http_client, upstream_url, upstream_headers, body)
    return await _forward_buffered(http_client, upstream_url, upstream_headers, body)


def _verify_signature(
    request: Request,
    body: bytes,
    settings: Settings,
    nonce_cache: NonceCache,
) -> tuple[int, str, str] | None:
    """검증 통과시 None, 실패시 (status, code, message)."""
    secret = settings.client_shared_secret
    if not secret:
        return (503, "server_misconfigured", "signing not configured")

    ts = request.headers.get(HEADER_TIMESTAMP, "")
    nonce = request.headers.get(HEADER_NONCE, "")
    signature = request.headers.get(HEADER_SIGNATURE, "")
    if not ts or not nonce or not signature:
        return (401, "missing_signature_headers", "signature headers required")

    try:
        ts_int = int(ts)
    except ValueError:
        return (401, "bad_timestamp", "timestamp must be unix seconds")

    now = int(time.time())
    if abs(now - ts_int) > settings.freshness_window_s:
        return (401, "stale_timestamp", "timestamp outside freshness window")

    # nonce 는 충분히 길어야 함. (16바이트 hex == 32자 권장. 최소 16자.)
    if len(nonce) < 16 or len(nonce) > 128:
        return (401, "bad_nonce", "nonce length invalid")

    expected = compute_signature(secret, ts, nonce, body)
    if not constant_time_equal(expected, signature):
        return (401, "signature_mismatch", "signature did not verify")

    # 서명 검증이 끝난 다음에 nonce 를 등록 (서명 실패시엔 nonce 소모 안 시킴).
    if not nonce_cache.check_and_store(nonce):
        return (401, "nonce_replayed", "nonce already used")

    return None


async def _forward_buffered(
    client: httpx.AsyncClient,
    url: str,
    headers: dict[str, str],
    body: bytes,
) -> Response:
    try:
        upstream = await client.post(url, content=body, headers=headers)
    except httpx.TimeoutException:
        return _err(504, "upstream_timeout", "upstream timed out")
    except httpx.HTTPError as exc:
        logger.warning("upstream_error %s", type(exc).__name__)
        return _err(502, "upstream_error", "upstream request failed")

    # content-type 만 전달, hop-by-hop 헤더는 버린다.
    out_headers = {}
    ct = upstream.headers.get("content-type")
    if ct:
        out_headers["content-type"] = ct
    return Response(
        content=upstream.content,
        status_code=upstream.status_code,
        headers=out_headers,
    )


async def _forward_stream(
    client: httpx.AsyncClient,
    url: str,
    headers: dict[str, str],
    body: bytes,
) -> StreamingResponse:
    # SSE 스트림용 패스스루. 업스트림 응답이 닫힐 때까지 청크 단위 중계.
    req = client.build_request("POST", url, content=body, headers=headers)
    upstream = await client.send(req, stream=True)

    async def gen():
        try:
            async for chunk in upstream.aiter_raw():
                if chunk:
                    yield chunk
        finally:
            await upstream.aclose()

    media_type = upstream.headers.get("content-type", "text/event-stream")
    return StreamingResponse(
        gen(), status_code=upstream.status_code, media_type=media_type
    )


# uvicorn pokemon_proxy:app 처럼 모듈 로딩 시 곧장 쓸 수 있는 기본 인스턴스.
app = create_app()
