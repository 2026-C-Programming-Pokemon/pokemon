"""인바운드 인증 / 리플레이 방지 / 레이트리밋.

서명 스킴 (클라이언트와 동일하게 계산):
    canonical = f"{timestamp}\n{nonce}\n{sha256_hex(body)}"
    signature = HMAC_SHA256(shared_secret, canonical)  # hex 인코딩

헤더 (모두 필수):
    X-Pokemon-Client-Id    클라이언트 식별자 (예: "pokemon-c-client/1.0")
    X-Pokemon-Timestamp    unix epoch 초 (문자열)
    X-Pokemon-Nonce        충분히 무작위인 nonce (16바이트 이상 hex 권장)
    X-Pokemon-Signature    위 hex(HMAC-SHA256) 값
"""
from __future__ import annotations

import hashlib
import hmac
import threading
import time
from collections import OrderedDict, deque
from dataclasses import dataclass

HEADER_CLIENT_ID = "X-Pokemon-Client-Id"
HEADER_TIMESTAMP = "X-Pokemon-Timestamp"
HEADER_NONCE = "X-Pokemon-Nonce"
HEADER_SIGNATURE = "X-Pokemon-Signature"


@dataclass(frozen=True)
class AuthError:
    status: int
    code: str
    message: str


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def compute_signature(secret: str, timestamp: str, nonce: str, body: bytes) -> str:
    """클라이언트도 동일하게 계산하는 정규화된 서명 값."""
    canonical = f"{timestamp}\n{nonce}\n{sha256_hex(body)}".encode("utf-8")
    return hmac.new(secret.encode("utf-8"), canonical, hashlib.sha256).hexdigest()


def constant_time_equal(a: str, b: str) -> bool:
    return hmac.compare_digest(a.encode("utf-8"), b.encode("utf-8"))


class NonceCache:
    """본 적 있는 nonce 를 TTL 동안 기억해 리플레이를 차단.

    in-process 메모리 캐시. 한 프록시 인스턴스 범위에서만 유효하므로
    여러 프록시를 띄울 거면 sticky session 이나 외부 캐시(redis 등) 가 필요.
    """

    def __init__(self, ttl_s: int) -> None:
        self._ttl = ttl_s
        self._lock = threading.Lock()
        # value: 만료 unix 초
        self._seen: OrderedDict[str, float] = OrderedDict()

    def check_and_store(self, nonce: str, now: float | None = None) -> bool:
        """nonce 가 처음 보이는 거면 True, 이미 본 거면 False."""
        if not nonce:
            return False
        ts = now if now is not None else time.time()
        with self._lock:
            self._evict(ts)
            if nonce in self._seen:
                return False
            self._seen[nonce] = ts + self._ttl
            return True

    def _evict(self, now: float) -> None:
        # OrderedDict 의 삽입순서는 만료시각 순서와 거의 같음.
        # 아닌 경우라도 진행하면 그만큼 다음 호출에서 정리됨.
        while self._seen:
            nonce, expires = next(iter(self._seen.items()))
            if expires <= now:
                self._seen.popitem(last=False)
            else:
                break

    def __len__(self) -> int:
        with self._lock:
            return len(self._seen)


class SlidingWindowRateLimiter:
    """키별 sliding window 카운터 (1분 window).

    버스트는 별도의 토큰 버켓 없이 단일 윈도 카운트로만 잡는다.
    """

    def __init__(self, per_minute: int, burst: int) -> None:
        # per_minute=0 이면 무제한.
        self.per_minute = max(0, per_minute)
        self.burst = max(0, burst)
        self._lock = threading.Lock()
        self._hits: dict[str, deque[float]] = {}

    def hit(self, key: str, now: float | None = None) -> bool:
        if self.per_minute == 0:
            return True
        ts = now if now is not None else time.time()
        cutoff = ts - 60.0
        with self._lock:
            dq = self._hits.setdefault(key, deque())
            while dq and dq[0] < cutoff:
                dq.popleft()
            # 1분 누적 + 즉시 버스트 모두 만족해야 통과.
            if len(dq) >= self.per_minute + self.burst:
                return False
            dq.append(ts)
            return True

    def reset(self) -> None:
        with self._lock:
            self._hits.clear()
