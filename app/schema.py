"""요청 본문 엄격 스키마 검증.

docs/proxy-protocol.md §3 의 규격을 코드로 옮긴 것. 프록시는 "규격에 맞는
요청만" 업스트림으로 보내야 하므로, 모르는 필드/엉뚱한 타입/허용 범위 밖 값을
업스트림 호출 전에 거른다.

OpenAI 가 받아주는 임의 파라미터(tools, logprobs, n, ... )를 통째로 패스스루하지
않는 것이 핵심이다. 게임 클라이언트가 실제로 쓰는 필드만 화이트리스트로 통과시킨다.
"""
from __future__ import annotations

from typing import Any

# 본문 최상위에서 허용하는 필드. 이 외의 키가 하나라도 있으면 거절.
_ALLOWED_TOP_LEVEL = {"model", "messages", "max_tokens", "temperature", "stream"}
_REQUIRED_TOP_LEVEL = {"model", "messages", "max_tokens"}

# messages 원소에서 허용하는 키와 role 값.
_ALLOWED_MESSAGE_KEYS = {"role", "content"}
_ALLOWED_ROLES = {"system", "user", "assistant"}

# 검증 실패 시 반환하는 튜플: (http_status, error_code, message).
SchemaError = tuple[int, str, str]


def validate_chat_body(
    payload: Any,
    *,
    max_messages: int,
    max_completion_tokens: int,
) -> SchemaError | None:
    """통과하면 None, 어긋나면 (status, code, message) 를 돌려준다.

    model 의 화이트리스트(ALLOWED_MODELS) 검증은 호출 측에서 따로 하므로
    여기서는 model 이 문자열인지까지만 본다.
    """
    if not isinstance(payload, dict):
        return (400, "bad_json", "request body must be a json object")

    # 1) 모르는 최상위 필드 거절.
    unexpected = set(payload.keys()) - _ALLOWED_TOP_LEVEL
    if unexpected:
        name = sorted(unexpected)[0]
        return (400, "unexpected_field", f"field not allowed: {name}")

    # 2) 필수 필드 존재.
    missing = _REQUIRED_TOP_LEVEL - set(payload.keys())
    if missing:
        name = sorted(missing)[0]
        return (400, "missing_field", f"required field missing: {name}")

    # 3) model
    if not isinstance(payload["model"], str) or not payload["model"]:
        return (400, "bad_model", "model must be a non-empty string")

    # 4) messages
    err = _validate_messages(payload["messages"], max_messages)
    if err is not None:
        return err

    # 5) max_tokens
    max_tokens = payload["max_tokens"]
    # bool 은 int 의 서브클래스라 명시적으로 배제.
    if isinstance(max_tokens, bool) or not isinstance(max_tokens, int):
        return (400, "bad_max_tokens", "max_tokens must be an integer")
    if max_tokens < 1 or max_tokens > max_completion_tokens:
        return (
            400,
            "bad_max_tokens",
            f"max_tokens must be between 1 and {max_completion_tokens}",
        )

    # 6) temperature (선택)
    if "temperature" in payload:
        temp = payload["temperature"]
        if isinstance(temp, bool) or not isinstance(temp, (int, float)):
            return (400, "bad_temperature", "temperature must be a number")
        if temp < 0.0 or temp > 2.0:
            return (400, "bad_temperature", "temperature must be between 0.0 and 2.0")

    # 7) stream (선택)
    if "stream" in payload and not isinstance(payload["stream"], bool):
        return (400, "bad_stream", "stream must be a boolean")

    return None


def _validate_messages(messages: Any, max_messages: int) -> SchemaError | None:
    if not isinstance(messages, list) or not messages:
        return (400, "bad_messages", "messages must be a non-empty array")
    if len(messages) > max_messages:
        return (400, "bad_messages", f"too many messages (max {max_messages})")

    for idx, msg in enumerate(messages):
        if not isinstance(msg, dict):
            return (400, "bad_messages", f"messages[{idx}] must be an object")
        extra = set(msg.keys()) - _ALLOWED_MESSAGE_KEYS
        if extra:
            name = sorted(extra)[0]
            return (400, "bad_messages", f"messages[{idx}] has unexpected key: {name}")
        if "role" not in msg or "content" not in msg:
            return (400, "bad_messages", f"messages[{idx}] needs role and content")
        if msg["role"] not in _ALLOWED_ROLES:
            return (400, "bad_messages", f"messages[{idx}] has invalid role")
        if not isinstance(msg["content"], str) or not msg["content"]:
            return (
                400,
                "bad_messages",
                f"messages[{idx}] content must be a non-empty string",
            )

    return None
