# pokemon-llm-proxy

C 클라이언트 (`llm/llm.c`) 가 부르는 OpenAI 호환 `/v1/chat/completions` 엔드포인트를
공개 ARM 서버에서 프록시합니다. 업스트림 API 키는 절대 외부로 나가지 않습니다.

> 보안 한계 고지: 이 프록시는 "포켓몬 게임 프로그램만 호출하도록" 강제하기 위한 여러
> 서버측 통제를 갖추지만, **공개 배포되는 바이너리에 들어있는 공유 비밀로는 완벽한
> 신원 증명을 만들 수 없습니다**. 결정적인 공격자는 바이너리에서 비밀을 추출해 동일한
> 요청을 재현할 수 있습니다. 본 프록시의 목표는 그 비용을 충분히 높이고, 정상 트래픽
> 외의 패턴을 가시화/억제하는 것입니다. 강한 신원이 필요하다면 서버측 사용자 계정
> 또는 단기 토큰 발급(별도의 인증 서비스) 으로 가야 합니다.

## 보호 계층

서버에서 다음 검사를 순서대로 적용합니다:

1. **Content-Length / 본문 크기 캡** (`MAX_BODY_BYTES`)
2. **User-Agent 접두사** (옵션, `EXPECTED_USER_AGENT_PREFIX`)
3. **HMAC-SHA256 서명 검증**
   - 헤더: `X-Pokemon-Client-Id`, `X-Pokemon-Timestamp`, `X-Pokemon-Nonce`, `X-Pokemon-Signature`
   - 정규형식: `signature = HMAC_SHA256(secret, f"{ts}\n{nonce}\n{sha256_hex(body)}")`
4. **타임스탬프 freshness** (`REQUEST_FRESHNESS_WINDOW_S`)
5. **Nonce 리플레이 캐시** (`NONCE_CACHE_TTL_S`, 인-프로세스 메모리)
6. **클라이언트 ID 화이트리스트** (옵션, `ALLOWED_CLIENT_IDS`)
7. **IP / 클라이언트별 sliding window 레이트리밋** (1분 단위)
8. **모델 화이트리스트** (옵션, `ALLOWED_MODELS`)
9. **업스트림으로 포워딩**, 비스트리밍/스트리밍 모두 지원

서명/Authorization/Cookie 헤더는 로그에 절대 남기지 않습니다.

## 빠른 시작 (ARM 서버, Ubuntu/Debian)

```sh
sudo apt-get update
sudo apt-get install -y python3 python3-venv

cd app
python3 -m venv .venv
. .venv/bin/activate
pip install -U pip
pip install -r requirements.txt

cp .env.example .env
$EDITOR .env   # UPSTREAM_API_KEY 와 POKEMON_CLIENT_SECRET 채우기

# 셸 환경에 .env 를 로드.
set -a; . ./.env; set +a

python -m app.main
# 또는: uvicorn app.app:app --host 0.0.0.0 --port 8080
```

리포 루트(`pokemon/`) 에서 모듈 경로(`app.app:app`) 로 실행합니다.

라즈베리파이 같은 aarch64 보드에서도 휠이 깔끔하게 떨어집니다.

## 환경변수

| 변수 | 기본값 | 설명 |
|---|---|---|
| `UPSTREAM_API_KEY` | (필수) | 업스트림(OpenAI 호환) 키. 클라이언트에는 절대 노출 X. |
| `UPSTREAM_BASE_URL` | `https://api.openai.com/v1` | 트레일링 슬래시 없이. `/chat/completions` 가 자동 부착. |
| `UPSTREAM_TIMEOUT_S` | `60` | 업스트림 전체 타임아웃 (초). |
| `PROXY_BIND_HOST` | `0.0.0.0` | 바인딩 호스트. |
| `PROXY_BIND_PORT` | `8080` | 바인딩 포트. |
| `POKEMON_CLIENT_SECRET` | (필수) | 클라이언트와 공유하는 HMAC 키. |
| `REQUEST_FRESHNESS_WINDOW_S` | `60` | 타임스탬프 허용 범위 (초). |
| `NONCE_CACHE_TTL_S` | `300` | nonce 캐시 TTL (초). |
| `MAX_BODY_BYTES` | `65536` | 본문 최대 바이트. |
| `RATE_LIMIT_PER_IP_PER_MIN` | `30` | IP 당 1분 허용 횟수. `0` = 무제한. |
| `RATE_LIMIT_PER_CLIENT_PER_MIN` | `60` | client-id 당 1분 허용 횟수. |
| `RATE_LIMIT_BURST` | `10` | 단일 윈도 안에서의 여유 버퍼. |
| `ALLOWED_MODELS` | (없음) | 쉼표 구분 모델 화이트리스트. 비우면 미적용. |
| `ALLOWED_CLIENT_IDS` | (없음) | 쉼표 구분 client-id 화이트리스트. |
| `EXPECTED_USER_AGENT_PREFIX` | (없음) | UA 접두사 강제. 비우면 미적용. |
| `TRUST_X_FORWARDED_FOR` | `false` | `true` 일 때만 `X-Forwarded-For` 첫 IP를 신뢰. 신뢰 가능한 리버스 프록시 뒤에서만 켜기. |
| `LOG_LEVEL` | `INFO` | 로깅 레벨. |
| `REQUIRE_SIGNATURE` | `true` | `false` 면 HMAC 검증 우회 (로컬 디버그용만). |

## 엔드포인트

- `GET /healthz` — liveness.
- `POST /v1/chat/completions` — OpenAI 호환. `stream: true` 도 지원.

에러 응답 포맷:

```json
{ "error": { "code": "signature_mismatch", "message": "signature did not verify" } }
```

상태 코드: `400, 401, 403, 413, 429, 502, 503, 504`.

## 클라이언트(C 측) 통합 가이드

기존 `llm/llm.c` 는 `LLM_BASE_URL` 만으로 OpenAI 호환 엔드포인트를 바라보게 되어
있습니다. 본 프록시를 가리키려면:

```sh
export LLM_BASE_URL=https://your-arm-server.example.com/v1/chat/completions
# OPENAI_API_KEY 는 비워둬도 됨 — 프록시가 채워줌.
```

하지만 **현재 C 코드는 HMAC 시그니처 헤더를 만들지 않으므로**, 프록시의
`REQUIRE_SIGNATURE=true` 상태에선 401 이 반환됩니다. 추후 C 클라이언트에 다음 단계를
추가해야 합니다 (별도 작업):

1. 컴파일 타임/배포 시 주입한 `POKEMON_CLIENT_SECRET` 을 메모리에 보관.
2. 요청 전송 직전:
   - `body` = JSON 직렬화된 페이로드
   - `ts` = unix 초 문자열
   - `nonce` = 16바이트 이상 무작위 hex
   - `body_hash` = `sha256_hex(body)`
   - `signature` = hex(`HMAC_SHA256(secret, ts + "\n" + nonce + "\n" + body_hash)`)
3. 다음 헤더를 추가:
   - `X-Pokemon-Client-Id: pokemon-c-client/<version>`
   - `X-Pokemon-Timestamp: <ts>`
   - `X-Pokemon-Nonce: <nonce>`
   - `X-Pokemon-Signature: <signature>`
   - `User-Agent: pokemon-c-client/<version>` (서버에서 prefix 매치)

레퍼런스 구현: `app/example_client.py` 의 `make_headers` 와 `app/security.py` 의
`compute_signature`. C 측은 OpenSSL `HMAC()` / `SHA256()` 또는 mbedtls 로 동일하게
구현할 수 있습니다 (라이브러리 추가 필요).

## 운영 메모

- 단일 프로세스 인-프로세스 nonce/레이트리밋이라, 여러 워커로 띄우면 보호가 약해집니다.
  ARM 보드에서는 `--workers 1` 권장. 수평 확장이 필요해지면 redis 기반으로 교체.
- 기본값은 소켓 peer IP 를 사용합니다. `X-Forwarded-For` 는 `TRUST_X_FORWARDED_FOR=true`
  일 때만 읽습니다.
- 리버스 프록시(nginx/Caddy) 뒤에 둘 경우에만 `TRUST_X_FORWARDED_FOR=true` 로 켜고,
  반드시 신뢰 가능한 프록시만 그 헤더를 세팅하도록 설정하세요.
- TLS 는 이 앱에서 종단하지 않습니다. nginx/Caddy/Cloudflare 등으로 처리.
- 비밀(특히 `UPSTREAM_API_KEY`, `POKEMON_CLIENT_SECRET`) 은 환경변수 / systemd
  EnvironmentFile / 시크릿 매니저로만 주입하세요. 로그/응답엔 절대 노출되지 않습니다.

## 테스트

```sh
cd app
python -m venv .venv && . .venv/bin/activate
pip install -r requirements-dev.txt
PYTHONPATH=.. python -m pytest tests -q
```

테스트는 업스트림을 `httpx.MockTransport` 로 모킹하므로 네트워크가 없어도 돕니다.

## Docker (선택)

```sh
docker build -f app/deploy/Dockerfile -t pokemon-llm-proxy:dev .
docker run --rm -p 8080:8080 --env-file app/.env pokemon-llm-proxy:dev
```

## systemd (선택)

`app/deploy/pokemon-llm-proxy.service` 참고. `EnvironmentFile=` 로 `/etc/pokemon-llm-proxy.env`
를 가리키게 하고 그 파일 권한은 `0600` 으로.
