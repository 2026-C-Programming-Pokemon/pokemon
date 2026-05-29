"""security 모듈 단위 테스트."""
from __future__ import annotations

import time

from app.security import (
    NonceCache,
    SlidingWindowRateLimiter,
    compute_signature,
    constant_time_equal,
    sha256_hex,
)


def test_compute_signature_stable():
    sig1 = compute_signature("secret", "1700000000", "nonce-abc", b'{"a":1}')
    sig2 = compute_signature("secret", "1700000000", "nonce-abc", b'{"a":1}')
    assert sig1 == sig2
    assert len(sig1) == 64  # sha256 hex


def test_compute_signature_changes_on_body():
    sig1 = compute_signature("secret", "1700000000", "nonce", b'{"a":1}')
    sig2 = compute_signature("secret", "1700000000", "nonce", b'{"a":2}')
    assert sig1 != sig2


def test_constant_time_equal():
    assert constant_time_equal("aaaa", "aaaa")
    assert not constant_time_equal("aaaa", "aaab")


def test_sha256_hex_known():
    assert sha256_hex(b"") == (
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    )


def test_nonce_cache_replay():
    cache = NonceCache(ttl_s=60)
    assert cache.check_and_store("n1") is True
    assert cache.check_and_store("n1") is False
    assert cache.check_and_store("n2") is True


def test_nonce_cache_expires():
    cache = NonceCache(ttl_s=1)
    base = time.time()
    assert cache.check_and_store("n1", now=base) is True
    # 동일 시각 → 리플레이.
    assert cache.check_and_store("n1", now=base) is False
    # ttl 경과 후 → 다시 받아준다.
    assert cache.check_and_store("n1", now=base + 2) is True


def test_rate_limiter_blocks_after_limit():
    rl = SlidingWindowRateLimiter(per_minute=3, burst=0)
    base = time.time()
    assert rl.hit("ip", now=base) is True
    assert rl.hit("ip", now=base) is True
    assert rl.hit("ip", now=base) is True
    assert rl.hit("ip", now=base) is False


def test_rate_limiter_window_slides():
    rl = SlidingWindowRateLimiter(per_minute=2, burst=0)
    base = 1000.0
    assert rl.hit("ip", now=base) is True
    assert rl.hit("ip", now=base) is True
    assert rl.hit("ip", now=base) is False
    # 1분 + 1초 후엔 다시 가능.
    assert rl.hit("ip", now=base + 61) is True


def test_rate_limiter_zero_means_unlimited():
    rl = SlidingWindowRateLimiter(per_minute=0, burst=0)
    for _ in range(1000):
        assert rl.hit("ip") is True
