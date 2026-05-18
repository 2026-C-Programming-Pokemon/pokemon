"""schema 모듈 단위 테스트 (본문 엄격 스키마 검증)."""
from __future__ import annotations

from app.schema import validate_chat_body

LIMITS = {"max_messages": 16, "max_completion_tokens": 1024}


def _ok_body(**over):
    body = {
        "model": "gpt-4o-mini",
        "max_tokens": 64,
        "messages": [{"role": "user", "content": "hi"}],
    }
    body.update(over)
    return body


def test_valid_body_passes():
    assert validate_chat_body(_ok_body(), **LIMITS) is None


def test_valid_with_system_message_passes():
    body = _ok_body(
        messages=[
            {"role": "system", "content": "중계 아나운서"},
            {"role": "user", "content": "피카츄가 공격"},
        ]
    )
    assert validate_chat_body(body, **LIMITS) is None


def test_optional_temperature_and_stream_pass():
    assert validate_chat_body(_ok_body(temperature=0.7, stream=True), **LIMITS) is None


def test_non_object_rejected():
    err = validate_chat_body(["not", "a", "dict"], **LIMITS)
    assert err is not None and err[1] == "bad_json"


def test_unexpected_top_level_field_rejected():
    err = validate_chat_body(_ok_body(tools=[]), **LIMITS)
    assert err is not None and err[1] == "unexpected_field"


def test_missing_required_field_rejected():
    body = _ok_body()
    del body["max_tokens"]
    err = validate_chat_body(body, **LIMITS)
    assert err is not None and err[1] == "missing_field"


def test_empty_messages_rejected():
    err = validate_chat_body(_ok_body(messages=[]), **LIMITS)
    assert err is not None and err[1] == "bad_messages"


def test_too_many_messages_rejected():
    msgs = [{"role": "user", "content": "x"} for _ in range(17)]
    err = validate_chat_body(_ok_body(messages=msgs), **LIMITS)
    assert err is not None and err[1] == "bad_messages"


def test_message_extra_key_rejected():
    body = _ok_body(messages=[{"role": "user", "content": "hi", "name": "ash"}])
    err = validate_chat_body(body, **LIMITS)
    assert err is not None and err[1] == "bad_messages"


def test_message_bad_role_rejected():
    body = _ok_body(messages=[{"role": "root", "content": "hi"}])
    err = validate_chat_body(body, **LIMITS)
    assert err is not None and err[1] == "bad_messages"


def test_message_empty_content_rejected():
    body = _ok_body(messages=[{"role": "user", "content": ""}])
    err = validate_chat_body(body, **LIMITS)
    assert err is not None and err[1] == "bad_messages"


def test_max_tokens_must_be_int():
    err = validate_chat_body(_ok_body(max_tokens="64"), **LIMITS)
    assert err is not None and err[1] == "bad_max_tokens"


def test_max_tokens_bool_rejected():
    # bool 은 int 서브클래스라 따로 막아야 한다.
    err = validate_chat_body(_ok_body(max_tokens=True), **LIMITS)
    assert err is not None and err[1] == "bad_max_tokens"


def test_max_tokens_over_cap_rejected():
    err = validate_chat_body(_ok_body(max_tokens=99999), **LIMITS)
    assert err is not None and err[1] == "bad_max_tokens"


def test_max_tokens_zero_rejected():
    err = validate_chat_body(_ok_body(max_tokens=0), **LIMITS)
    assert err is not None and err[1] == "bad_max_tokens"


def test_temperature_out_of_range_rejected():
    err = validate_chat_body(_ok_body(temperature=3.0), **LIMITS)
    assert err is not None and err[1] == "bad_temperature"


def test_stream_must_be_bool():
    err = validate_chat_body(_ok_body(stream="yes"), **LIMITS)
    assert err is not None and err[1] == "bad_stream"
