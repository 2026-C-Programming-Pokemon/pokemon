# CLAUDE.md

이 파일은 이 레포에서 Claude Code 가 작업할 때 참고하는 컨텍스트 메모입니다.

## 프로젝트 개요

- 이 레포는 **C 콘솔 포켓몬 배틀 게임** + **FastAPI 기반 LLM 프록시 서버(`app/`)** 로 구성됩니다.
- 게임 본체 핵심 코드:
  - [pokemon.c](pokemon.c): 실행 화면, 입력, 배경음악 제어, `main`
  - [pokemon.h](pokemon.h): 공용 타입, 상수, 함수 선언
  - [dogam/](dogam/): 포켓몬 도감 데이터, 타입 상성, 기본 능력치 계산
  - [skill/](skill/): 기술 후보/고정 기술 데이터, 기술 배치
  - [battlelogic/](battlelogic/): 데미지, 상태이상, 턴 진행, AI 기술 선택
  - [entry/](entry/): 플레이어/상대 트레이너 엔트리 구성
  - [score/](score/): 점수/리더보드 저장
- 프록시 서버:
  - [app/](app/): FastAPI 로 구현된 OpenAI 호환 `/v1/chat/completions` 프록시
  - 업스트림 API 키 은닉 + HMAC/timestamp/nonce/rate-limit + 프로토콜 버전 +
    본문 엄격 스키마 검증으로 "규격에 맞는 요청만" 통과
- 통신 규격:
  - [docs/proxy-protocol.md](docs/proxy-protocol.md) — 클라이언트/프록시 공통 단일 기준
- 스프라이트 변환 도구:
  - [tools/ascii_converter/](tools/ascii_converter/) — PNG 를 유니코드 ASCII 아트로 변환
- 외부 LLM 호출 모듈:
  - [llm/](llm/) — OpenAI 호환 Chat Completions 호출. **외부 라이브러리 의존성 없음**:
    HTTP 전송은 POSIX 소켓으로 직접 구현(평문 HTTP 만), HMAC 서명은 공개 도메인 코드 사용.
  - [llm/llm.c](llm/llm.c) — 직접 작성: 소켓 HTTP 클라이언트, 프롬프트 조립, 서명 부착.
  - [llm/sha256.{c,h}](llm/), [llm/hmac_sha256.{c,h}](llm/) — **외부 공개 도메인(Unlicense)
    코드를 그대로 가져온 것**. 각 파일 머리말에 출처 URL/커밋이 적혀 있음. 직접 작성 아님.

## 현재 브랜치 컨텍스트

- `feat/api-proxy-server`
  - `app/` 프록시 서버 추가
  - Unix 배경음악 폴백 안정화 반영
- `feat/#1-ascii-art-sprite`
  - 스프라이트/ASCII 아트 작업 브랜치

브랜치 성격이 다르므로, 프록시 서버 작업은 `feat/api-proxy-server` 기준으로 이어가는 것이 맞습니다.

## 빌드 / 실행

### 게임 본체

```sh
make          # pokemon 실행 파일 생성
make run      # 빌드 후 실행
make clean    # 산출물 정리
```

- 컴파일러: `cc` (gcc/clang 어느 쪽이든 c99 지원이면 됨). Windows 는 CMake 사용.
- **외부 라이브러리 의존성 없음**. `make` 만 하면 빌드됩니다 (libcurl 등 설치 불필요).
  - HTTP 전송은 Berkeley 소켓으로 직접 구현. POSIX 와 Windows(winsock) 차이는
    [llm/llm.c](llm/llm.c) 상단의 `#ifdef _WIN32` 플랫폼 계층에서 흡수하므로 함수
    본문은 한 벌뿐 — "LLM 켠/끈 빌드" 같은 컴파일 분기는 없습니다.
  - CMake 빌드는 Windows 에서 `ws2_32` 를 자동 링크합니다.

### 프록시 서버

```sh
cd app
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements-dev.txt
python -m app.main
```

테스트:

```sh
cd app
PYTHONPATH=.. python -m pytest tests -q
```

## LLM 모듈 메모

공개 API 는 [llm/llm.h](llm/llm.h) 참고.

```c
#include "llm.h"

llm_init();

/* 범용 호출 */
char out[1024];
if (llm_generate("한 줄 나레이션 만들어줘.", out, sizeof(out)) == 0)
    printf("%s\n", out);

/* 기술 선택 헬퍼 — situation 에 상황+기술목록을 주면 1..N 숫자를 돌려준다.
   battlelogic 의 chooseAIMove 가 이 함수를 쓴다. */
int picked = llm_choose_move(situation, move_count);   /* 성공: 1..move_count, 실패: -1 */

llm_cleanup();
```

게임 연결: 적 AI 기술 선택([battlelogic.c](battlelogic/battlelogic.c) `chooseAIMove`)이
`llm_choose_move` 를 호출합니다. LLM 이 1~N 숫자를 주면 그 기술을, 실패하면 무작위로 고릅니다.

**전송은 평문 HTTP 만** 지원합니다 (소켓으로 직접 구현, TLS 미지원). 따라서 https
엔드포인트(OpenAI 직격)는 못 부르고, `app/` 프록시를 평문 HTTP 로 경유하는 것이 기본입니다.

동작 모드 두 가지:

- **프록시 모드 (기본/권장)**: `POKEMON_CLIENT_SECRET` 이 설정되면 자동 활성화. `app/`
  프록시 통신 규격에 맞춰 HMAC 서명 헤더를 붙임. 서명은 공개 도메인 SHA256/HMAC
  코드(`llm/sha256.*`, `llm/hmac_sha256.*`)로 계산하며 `app/security.py` 와 동일 결과.
- **직결 모드**: 로컬 Ollama 같은 OpenAI 호환 **평문 HTTP** 엔드포인트를 직접 호출.
  `OPENAI_API_KEY` 가 있으면 Authorization 으로 보냄.

런타임 설정 (환경 변수):

| 변수 | 필수 | 기본값 | 설명 |
|---|---|---|---|
| `LLM_BASE_URL` | no | 로컬 프록시 | OpenAI 호환 엔드포인트 (`http://` 만). 프록시 모드면 프록시 URL |
| `LLM_MODEL` | no | `gpt-4o-mini` | 사용할 모델 ID |
| `POKEMON_CLIENT_SECRET` | 프록시 모드 | - | 설정 시 프록시 모드 + HMAC 서명 활성화 |
| `POKEMON_CLIENT_ID` | no | `pokemon-c-client/1.0` | 클라이언트 식별자 |
| `OPENAI_API_KEY` | 직결 모드 | - | 직결 모드에서 Authorization 으로 사용 |

설계 원칙:

- LLM 호출 실패는 게임을 멈추지 않는다
- API 키는 코드에 하드코딩하지 않는다
- 응답 JSON 파싱은 OpenAI Chat Completions 포맷에 맞춘 단순 추출이다
- 클라이언트가 공개 배포될 경우, 업스트림 키는 직접 넣지 말고 `app/` 프록시를 경유한다
- 프롬프트 설계는 `llm/` 가 소유한다 (예: `llm_choose_move` 의 기술 선택 system 프롬프트)
- LLM 호출이 실패해도 게임 진행이 막히면 안 된다 — `chooseAIMove` 는 무작위로 폴백한다
- 외부 의존성을 새로 끌어오지 않는다. 암호 같은 표준 알고리즘이 필요하면 검증된
  공개 도메인 코드를 `llm/` 에 그대로 vendoring 하고 머리말에 출처를 남긴다
- 프록시 통신 규격의 단일 기준은 [docs/proxy-protocol.md](docs/proxy-protocol.md)

## 배경음악 관련 메모

- Windows: PowerShell `MediaPlayer` 기반 루프 재생
- Unix/macOS: 사용 가능한 플레이어를 탐색해서 재생
  - `ffplay`, `mpv`, `mpg123`, `cvlc`, `vlc`, (macOS 한정) `afplay`
- 플레이어가 없으면 음악은 **조용히 비활성화**되어야 한다
- 없는 플레이어를 무한 재시도해서 CPU 를 태우는 busy loop 는 금지
- 음악 프로세스 종료는 가능하면 개별 PID + 프로세스 그룹 기준으로 정리

## 프록시 서버 작업 원칙

- `app/README.md` 와 `docs/proxy-protocol.md` 를 먼저 읽고 맞춰서 수정할 것
- 통신 규격(헤더 집합, 서명 canonical, 본문 스키마)을 바꾸면 `docs/proxy-protocol.md`,
  `app/` (security/schema/config), `llm/llm.c` 를 함께 고치고 버전을 검토할 것
- OpenAI 호환 요청/응답 형태를 함부로 깨지 말 것
- 보안 관련 기본값은 보수적으로 유지할 것
  - 예: `TRUST_X_FORWARDED_FOR=false` 기본 유지
- 로그에 비밀값(`Authorization`, 서명값, 쿠키)을 남기지 말 것
- 테스트가 있으면 수정 후 반드시 다시 돌릴 것

## 작업 규칙

- 답변은 한국어로 (사용자 선호). 코드/주석은 한국어 주석 + 영어 식별자.
- 새 기능은 별도 브랜치에서 작업하고 PR 로 머지.
- 빌드 산출물 (`pokemon`, `*.o`) 과 Python 캐시/가상환경은 커밋하지 않는다.
