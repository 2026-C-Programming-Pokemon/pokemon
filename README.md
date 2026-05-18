# pokemon

C 로 만든 1세대 포켓몬 TUI 배틀 프로젝트입니다. 게임 본체는 C 콘솔 앱이고, 외부 LLM 키를 직접 배포하지 않기 위해 FastAPI 기반 프록시 서버를 `app/` 에 별도로 두고 있습니다.

> 현재 `feat/api-proxy-server` 브랜치에는 **FastAPI LLM 프록시 서버**와 **Unix 배경음악 폴백 안정화**가 포함되어 있습니다.

## 현재 구성

### 1) 게임 본체 (C)

```sh
make              # 빌드 (LLM 포함)
make LLM=0        # 빌드 (LLM 모듈을 스텁으로)
make run          # 빌드 + 실행
make clean        # 산출물 정리
```

**외부 라이브러리 의존성이 없습니다** — `make` 만 하면 빌드됩니다. HTTP 전송은
POSIX 소켓으로 직접 구현했고(평문 HTTP 만), HMAC 서명은 공개 도메인 코드를 씁니다.

LLM 쪽은 OpenAI 호환 `/v1/chat/completions` 엔드포인트를 평문 HTTP 로 호출합니다.
`LLM_BASE_URL` 로 로컬 Ollama 또는 `app/` 프록시 서버를 붙입니다. HTTPS 가 필요하면
프록시 앞단에 nginx/Caddy 를 두어 TLS 를 종단하세요.

### 2) LLM 프록시 서버 (FastAPI)

```sh
cd app
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements-dev.txt
python -m app.main
```

프록시는 다음 목적을 가집니다:
- 업스트림 API 키를 클라이언트에 배포하지 않기
- OpenAI 호환 `/v1/chat/completions` 제공
- HMAC 서명 / timestamp / nonce / rate limit 기반 남용 방지
- 프로토콜 버전 + 본문 엄격 스키마로 "규격에 맞는 요청만" 통과
- 클라이언트 식별 규칙(`User-Agent`, client id) 강제

자세한 설정은 [app/README.md](app/README.md), 통신 규격은
[docs/proxy-protocol.md](docs/proxy-protocol.md) 참고.

## 게임 LLM 설정

```sh
cp .env.example .env      # 열어서 LLM_BASE_URL / POKEMON_CLIENT_SECRET 채우기
make run
```

별도 라이브러리 설치가 필요 없습니다. Windows 는 소켓 API 가 달라 WSL 을 권장하며,
WSL 이 없으면 `make LLM=0` 로 LLM 없이 빌드할 수 있습니다.

`.env` 대신 `export LLM_BASE_URL=...` 로 넣어도 됩니다.
키가 없거나 호출이 실패하면 호출 측에서 폴백 처리해야 합니다.

| 변수 | 기본값 | 설명 |
|---|---|---|
| `LLM_BASE_URL` | 로컬 프록시 `http://127.0.0.1:8080/v1/chat/completions` | OpenAI 호환 엔드포인트. **`http://` 만 지원** (평문 HTTP). 예: 로컬 Ollama 또는 `app/` 프록시 |
| `POKEMON_CLIENT_SECRET` | - | 설정 시 프록시 모드(HMAC 서명) 활성화. 프록시와 같은 값 |
| `POKEMON_CLIENT_ID` | `pokemon-c-client/1.0` | 클라이언트 식별자 |
| `LLM_MODEL` | `gpt-4o-mini` | 모델 ID |
| `OPENAI_API_KEY` | - | 직결 모드에서 Authorization 으로 사용 |

[.env.example](.env.example) 에 프록시 / Ollama 백엔드 템플릿이 있습니다.

## 배경음악 메모

- Windows: PowerShell `MediaPlayer` 기반으로 루프 재생
- Unix/macOS: 사용 가능한 플레이어를 순서대로 탐색해서 재생
  - `ffplay` → `mpv` → `mpg123` → `cvlc` / `vlc`
  - macOS 에서만 `afplay` 폴백 사용
- **플레이어가 하나도 없으면 음악은 조용히 비활성화됩니다.**
- 없는 플레이어를 무한 재시도해서 CPU 를 태우던 문제를 방지하도록 안정화했습니다.

## 구조

- [pokemon.c](pokemon.c) — 실행 화면, 입력, 음악 재생, `main`
- [pokemon.h](pokemon.h) — 공용 타입, 상수, 함수 선언
- [dogam/](dogam/) — 포켓몬 도감 데이터, 타입 상성, 기본 능력치 계산
- [skill/](skill/) — 기술 후보/고정 기술 데이터, 기술 배치
- [battlelogic/](battlelogic/) — 데미지, 상태이상, 턴 진행, AI 기술 선택
- [entry/](entry/) — 플레이어/상대 트레이너 엔트리 구성
- [llm/](llm/) — LLM 호출 모듈 (OpenAI 호환). 의존성 없는 POSIX 소켓 HTTP + 공개 도메인 HMAC 서명
- [score/](score/) — 리더보드/점수 저장
- [sound/](sound/) — 배경음악 파일
- [app/](app/) — FastAPI 기반 LLM 프록시 서버
- [tools/ascii_converter/](tools/ascii_converter/) — PNG → 유니코드 ASCII 변환기 (Python)
