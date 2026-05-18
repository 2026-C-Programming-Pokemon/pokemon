# Pokemon LLM Proxy 통신 규격 v1

C 게임 클라이언트(`llm/`)와 FastAPI 프록시(`app/`) 사이의 요청 규격입니다.
양쪽 구현(`llm/llm.c`, `app/app.py`)은 이 문서를 단일 기준으로 삼습니다.

## 1. 개요

- 엔드포인트: `POST {PROXY_BASE}/v1/chat/completions` (OpenAI Chat Completions 호환)
- 전송: C 클라이언트는 **평문 HTTP** 로 프록시에 접근합니다 (TLS 미지원). 같은
  머신/LAN 에서는 그대로 쓰고, 인터넷 노출 시 프록시 앞단(nginx/Caddy/Cloudflare)이
  TLS 를 종단하도록 둡니다.
- 프록시는 **규격에 맞는 요청만** 통과시키고, 어긋나면 업스트림 호출 전에 거절합니다.

## 2. 필수 헤더

`REQUIRE_SIGNATURE=true` (기본)일 때 모든 요청은 아래 헤더를 전부 가져야 합니다.

| 헤더 | 값 | 비고 |
|---|---|---|
| `Content-Type` | `application/json` | |
| `User-Agent` | `pokemon-c-client/<버전>` 으로 시작 | `EXPECTED_USER_AGENT_PREFIX` 와 prefix 매치 |
| `X-Pokemon-Protocol` | `1` | 프로토콜 버전. 불일치/누락 시 거절 |
| `X-Pokemon-Client-Id` | `pokemon-c-client/<버전>` | 클라이언트 식별자 |
| `X-Pokemon-Timestamp` | unix epoch 초 (10진 문자열) | freshness window 안이어야 함 |
| `X-Pokemon-Nonce` | hex 문자열, 16~128자 | 요청마다 새로 생성, 재사용 금지 |
| `X-Pokemon-Signature` | 아래 §4 의 hex(HMAC-SHA256) | |

`Authorization` 헤더는 **보내지 않습니다.** 업스트림 키는 프록시가 주입합니다.

## 3. 본문 스키마 (엄격)

본문은 UTF-8 JSON 객체이며, **아래 필드만** 허용됩니다. 모르는 최상위 필드가
하나라도 있으면 `400 unexpected_field` 로 거절됩니다.

| 필드 | 필수 | 타입 | 제약 |
|---|---|---|---|
| `model` | 필수 | string | `ALLOWED_MODELS` 화이트리스트에 포함 |
| `messages` | 필수 | array | 1개 이상, `MAX_MESSAGES`(기본 16) 이하 |
| `max_tokens` | 필수 | integer | `1 ~ MAX_COMPLETION_TOKENS`(기본 1024) |
| `temperature` | 선택 | number | `0.0 ~ 2.0` |
| `stream` | 선택 | boolean | |

`messages` 의 각 원소는 `role` 과 `content` **두 키만** 가진 객체입니다.

- `role`: `"system"`, `"user"`, `"assistant"` 중 하나
- `content`: 비어 있지 않은 string

## 4. 서명 스킴

```
body_hash = sha256_hex(<요청 본문 바이트 그대로>)
canonical = "{timestamp}\n{nonce}\n{body_hash}"
signature = hex( HMAC_SHA256(POKEMON_CLIENT_SECRET, canonical) )
```

- `body_hash`, `signature` 는 소문자 hex.
- 서명 대상은 **직렬화된 본문 바이트 원본**입니다. 한 바이트라도 다르면 검증 실패.
- 공유 비밀 `POKEMON_CLIENT_SECRET` 은 클라이언트와 프록시가 동일 값을 가집니다.

## 5. 프록시 검증 순서

1. Content-Length / 실제 본문 크기 ≤ `MAX_BODY_BYTES`
2. `User-Agent` prefix (옵션)
3. `X-Pokemon-Protocol` == `EXPECTED_PROTOCOL_VERSION`
4. 서명 헤더 4종 존재
5. `X-Pokemon-Timestamp` freshness window
6. `X-Pokemon-Nonce` 리플레이 캐시
7. HMAC 서명 일치
8. `X-Pokemon-Client-Id` 화이트리스트 (옵션)
9. per-IP / per-client 레이트리밋
10. **본문 스키마 엄격 검증** (§3)
11. 업스트림 포워딩

## 6. 에러 응답

```json
{ "error": { "code": "<코드>", "message": "<설명>" } }
```

| 상태 | 대표 코드 |
|---|---|
| 400 | `bad_json`, `unexpected_field`, `missing_field`, `bad_messages`, `bad_max_tokens`, `bad_temperature`, `bad_stream`, `model_not_allowed` |
| 401 | `bad_protocol_version`, `missing_signature_headers`, `stale_timestamp`, `bad_nonce`, `signature_mismatch`, `nonce_replayed` |
| 403 | `user_agent_rejected`, `client_id_rejected` |
| 413 | `body_too_large` |
| 429 | `rate_limited_ip`, `rate_limited_client` |
| 502/503/504 | `upstream_error`, `upstream_unconfigured`, `upstream_timeout` |

## 7. 버전 정책

`X-Pokemon-Protocol` 값을 올려야 하는 경우(비호환 변경): 본문 스키마의
필수 필드 추가/제거, 서명 canonical 형식 변경, 헤더 집합 변경.
하위 호환 변경(선택 필드 추가 등)은 버전을 올리지 않습니다.

## 8. 레퍼런스 구현

- 클라이언트: `llm/llm.c` — POSIX 소켓 평문 HTTP + 서명 헤더 부착 (직접 작성)
- 클라이언트 암호: `llm/sha256.*`, `llm/hmac_sha256.*` — 공개 도메인 코드 vendoring
  (h5p9sl/hmac_sha256, Unlicense; 각 파일 머리말에 출처 명시)
- 서버: `app/app.py`, `app/security.py`, `app/schema.py`
- 디버그 클라이언트: `app/example_client.py`
