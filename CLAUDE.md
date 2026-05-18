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
  - 업스트림 API 키 은닉 + HMAC/timestamp/nonce/rate-limit 보호 목적
- 스프라이트 변환 도구:
  - [tools/ascii_converter/](tools/ascii_converter/) — PNG 를 유니코드 ASCII 아트로 변환
- 외부 LLM 호출 모듈:
  - [llm/](llm/) — libcurl 로 OpenAI 호환 Chat Completions API 호출

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

- 컴파일러: `cc` (gcc/clang 어느 쪽이든 c99 지원이면 됨)
- 의존성: `libcurl` (LLM 활성 빌드 시)
  - Debian/Ubuntu: `apt-get install libcurl4-openssl-dev build-essential`
  - Alpine: `apk add curl-dev build-base`
  - macOS: 시스템 기본 포함

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
#include "llm/llm.h"

llm_init();
char out[1024];
if (llm_generate("한 줄 나레이션 만들어줘.", out, sizeof(out)) == 0) {
    printf("%s\n", out);
}
llm_cleanup();
```

런타임 설정 (환경 변수):

| 변수 | 필수 | 기본값 | 설명 |
|---|---|---|---|
| `OPENAI_API_KEY` | yes | - | 기본 OpenAI 직결 모드에서 필요 |
| `LLM_BASE_URL` | no | `https://api.openai.com/v1/chat/completions` | OpenAI 호환 엔드포인트. 자체 프록시/로컬 백엔드 연결 가능 |
| `LLM_MODEL` | no | `gpt-4o-mini` | 사용할 모델 ID |

설계 원칙:

- LLM 호출 실패는 게임을 멈추지 않는다
- API 키는 코드에 하드코딩하지 않는다
- 응답 JSON 파싱은 OpenAI Chat Completions 포맷에 맞춘 단순 추출이다
- 클라이언트가 공개 배포될 경우, 업스트림 키는 직접 넣지 말고 `app/` 프록시를 경유하는 방향이 우선이다

## 배경음악 관련 메모

- Windows: PowerShell `MediaPlayer` 기반 루프 재생
- Unix/macOS: 사용 가능한 플레이어를 탐색해서 재생
  - `ffplay`, `mpv`, `mpg123`, `cvlc`, `vlc`, (macOS 한정) `afplay`
- 플레이어가 없으면 음악은 **조용히 비활성화**되어야 한다
- 없는 플레이어를 무한 재시도해서 CPU 를 태우는 busy loop 는 금지
- 음악 프로세스 종료는 가능하면 개별 PID + 프로세스 그룹 기준으로 정리

## 프록시 서버 작업 원칙

- `app/README.md` 를 먼저 읽고 맞춰서 수정할 것
- OpenAI 호환 요청/응답 형태를 함부로 깨지 말 것
- 보안 관련 기본값은 보수적으로 유지할 것
  - 예: `TRUST_X_FORWARDED_FOR=false` 기본 유지
- 로그에 비밀값(`Authorization`, 서명값, 쿠키)을 남기지 말 것
- 테스트가 있으면 수정 후 반드시 다시 돌릴 것

## 작업 규칙

- 답변은 한국어로 (사용자 선호). 코드/주석은 한국어 주석 + 영어 식별자.
- 새 기능은 별도 브랜치에서 작업하고 PR 로 머지.
- 빌드 산출물 (`pokemon`, `*.o`) 과 Python 캐시/가상환경은 커밋하지 않는다.
