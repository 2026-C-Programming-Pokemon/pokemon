"""uvicorn 진입점.

사용:
    python -m app.main             # 환경변수의 PROXY_BIND_HOST / _PORT 사용
    uvicorn app.app:app --host 0.0.0.0 --port 8080
"""
from __future__ import annotations

import uvicorn

from .config import get_settings


def main() -> None:
    s = get_settings()
    uvicorn.run(
        "app.app:app",
        host=s.bind_host,
        port=s.bind_port,
        log_level=s.log_level.lower(),
        access_log=True,
        # ARM 보드에서 멀티 워커는 메모리 비용이 크므로 1 권장. 필요 시 --workers 로 조절.
    )


if __name__ == "__main__":
    main()
