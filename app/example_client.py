"""테스트/디버깅용 시그니처 계산 + 호출 헬퍼.

C 클라이언트가 향후 동일한 서명 스킴을 구현할 때 참고하는 reference 입니다.
프로덕션에서는 사용하지 마세요.

사용 예:
    POKEMON_CLIENT_SECRET=... \\
    PROXY_URL=http://127.0.0.1:8080/v1/chat/completions \\
    python -m app.example_client "한 줄 나레이션"
"""
from __future__ import annotations

import os
import secrets
import sys
import time

import httpx

from .security import PROTOCOL_VERSION, compute_signature


def make_headers(secret: str, body: bytes, client_id: str) -> dict[str, str]:
    ts = str(int(time.time()))
    nonce = secrets.token_hex(16)  # 32자 hex
    sig = compute_signature(secret, ts, nonce, body)
    return {
        "Content-Type": "application/json",
        "User-Agent": "pokemon-c-client/0.1",
        "X-Pokemon-Protocol": PROTOCOL_VERSION,
        "X-Pokemon-Client-Id": client_id,
        "X-Pokemon-Timestamp": ts,
        "X-Pokemon-Nonce": nonce,
        "X-Pokemon-Signature": sig,
    }


def main(argv: list[str]) -> int:
    secret = os.environ.get("POKEMON_CLIENT_SECRET", "")
    if not secret:
        print("POKEMON_CLIENT_SECRET 환경변수가 필요합니다.", file=sys.stderr)
        return 2
    url = os.environ.get("PROXY_URL", "http://127.0.0.1:8080/v1/chat/completions")
    model = os.environ.get("LLM_MODEL", "gpt-4o-mini")
    prompt = " ".join(argv[1:]) or "한 줄 나레이션 만들어줘."

    import json

    body = json.dumps(
        {
            "model": model,
            "max_tokens": 128,
            "messages": [{"role": "user", "content": prompt}],
        },
        ensure_ascii=False,
    ).encode("utf-8")

    headers = make_headers(secret, body, client_id="pokemon-c-client/0.1")

    with httpx.Client(timeout=30.0) as client:
        r = client.post(url, content=body, headers=headers)
        print(r.status_code)
        print(r.text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
