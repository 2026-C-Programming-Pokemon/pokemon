"""환경변수 기반 설정 로더. 프로세스 부팅 시 1회만 읽고 캐시한다."""
from __future__ import annotations

import os
from dataclasses import dataclass, field
from functools import lru_cache


def _env_str(name: str, default: str = "") -> str:
    value = os.environ.get(name)
    return value if value is not None and value != "" else default


def _env_int(name: str, default: int) -> int:
    raw = os.environ.get(name)
    if raw is None or raw == "":
        return default
    try:
        return int(raw)
    except ValueError:
        return default


def _env_float(name: str, default: float) -> float:
    raw = os.environ.get(name)
    if raw is None or raw == "":
        return default
    try:
        return float(raw)
    except ValueError:
        return default


def _env_list(name: str) -> list[str]:
    raw = os.environ.get(name, "")
    return [item.strip() for item in raw.split(",") if item.strip()]


def _env_bool(name: str, default: bool) -> bool:
    raw = os.environ.get(name)
    if raw is None or raw == "":
        return default
    return raw.strip().lower() in {"1", "true", "yes", "on"}


@dataclass(frozen=True)
class Settings:
    upstream_api_key: str
    upstream_base_url: str
    upstream_timeout_s: float

    bind_host: str
    bind_port: int

    client_shared_secret: str
    freshness_window_s: int
    nonce_cache_ttl_s: int
    max_body_bytes: int

    rate_limit_per_ip_per_min: int
    rate_limit_per_client_per_min: int
    rate_limit_burst: int

    allowed_models: list[str] = field(default_factory=list)
    allowed_client_ids: list[str] = field(default_factory=list)
    user_agent_prefix: str = ""
    trust_x_forwarded_for: bool = False

    log_level: str = "INFO"
    require_signature: bool = True


@lru_cache(maxsize=1)
def get_settings() -> Settings:
    """환경변수에서 설정을 로드한다. 부팅 후 1회 호출."""
    return Settings(
        upstream_api_key=_env_str("UPSTREAM_API_KEY"),
        upstream_base_url=_env_str(
            "UPSTREAM_BASE_URL", "https://api.openai.com/v1"
        ).rstrip("/"),
        upstream_timeout_s=_env_float("UPSTREAM_TIMEOUT_S", 60.0),
        bind_host=_env_str("PROXY_BIND_HOST", "0.0.0.0"),
        bind_port=_env_int("PROXY_BIND_PORT", 8080),
        client_shared_secret=_env_str("POKEMON_CLIENT_SECRET"),
        freshness_window_s=_env_int("REQUEST_FRESHNESS_WINDOW_S", 60),
        nonce_cache_ttl_s=_env_int("NONCE_CACHE_TTL_S", 300),
        max_body_bytes=_env_int("MAX_BODY_BYTES", 64 * 1024),
        rate_limit_per_ip_per_min=_env_int("RATE_LIMIT_PER_IP_PER_MIN", 30),
        rate_limit_per_client_per_min=_env_int("RATE_LIMIT_PER_CLIENT_PER_MIN", 60),
        rate_limit_burst=_env_int("RATE_LIMIT_BURST", 10),
        allowed_models=_env_list("ALLOWED_MODELS"),
        allowed_client_ids=_env_list("ALLOWED_CLIENT_IDS"),
        user_agent_prefix=_env_str("EXPECTED_USER_AGENT_PREFIX"),
        trust_x_forwarded_for=_env_bool("TRUST_X_FORWARDED_FOR", False),
        log_level=_env_str("LOG_LEVEL", "INFO").upper(),
        require_signature=_env_bool("REQUIRE_SIGNATURE", True),
    )
