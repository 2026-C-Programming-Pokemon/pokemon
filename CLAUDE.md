# CLAUDE.md

이 파일은 이 레포에서 Claude Code 가 작업할 때 참고하는 컨텍스트 메모입니다.

## 프로젝트 개요

- C 언어로 작성한 1세대 포켓몬 TUI 배틀 과제.
- 핵심 게임 코드:
  - [pokemon.c](pokemon.c): 실행 화면, 입력, `main`
  - [pokemon.h](pokemon.h): 공용 타입, 상수, 함수 선언
  - [dogam/](dogam/): 포켓몬 도감 데이터, 타입 상성, 기본 능력치 계산
  - [skill/](skill/): 기술 후보/고정 기술 데이터, 기술 배치
  - [battlelogic/](battlelogic/): 데미지, 상태이상, 턴 진행, AI 기술 선택
  - [entry/](entry/): 플레이어/상대 트레이너 엔트리 구성
- 스프라이트 변환 도구: [tools/ascii_converter/](tools/ascii_converter/) — Python 으로 PNG 를 다크 하프블록 유니코드 아트로 변환하여 `generated/*.inc` 에 저장.
- 외부 LLM 호출 모듈: [llm/](llm/) — libcurl 로 OpenAI Chat Completions API 호출.

## 빌드 / 실행

```sh
make          # pokemon 실행 파일 생성
make run      # 빌드 후 실행
make clean    # 산출물 정리
```

- 컴파일러: `cc` (gcc/clang 어느 쪽이든 c99 지원이면 됨).
- 의존성: `libcurl` (OpenAI API 호출용).
  - Debian/Ubuntu: `apt-get install libcurl4-openssl-dev build-essential`
  - Alpine: `apk add curl-dev build-base`
  - macOS: 시스템에 기본 포함.

## LLM 모듈 사용법

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
| `OPENAI_API_KEY` | yes | - | 없으면 `llm_is_available()` 가 0 을 돌려주고 호출 시 -1. |
| `LLM_MODEL` | no | `gpt-4o-mini` | 사용할 모델 ID. |

설계 원칙:

- LLM 호출 실패는 게임을 멈추지 않는다. 호출 측은 항상 폴백 메시지를 준비한다 (예: [pokemon.c](pokemon.c) 의 `printBattleIntro`).
- API 키는 코드에 하드코딩하지 않는다 (환경 변수만).
- 응답 JSON 파싱은 OpenAI Chat Completions 응답에 한정된 단순 추출 ([llm/llm.c](llm/llm.c) 의 `extract_text`). 다른 공급자로 옮기면 다시 작성 필요.

## 현재 LLM 연결 지점

- 없음. 나레이션 용도는 모두 제거됨. LLM 은 "판단" 용도 (예: 기술 선택, 교체 결정 등) 로만 사용할 예정.
- 인터페이스 ([llm/llm.h](llm/llm.h)) 와 통신 모듈 ([llm/llm.c](llm/llm.c)) 은 그대로 유지. 호출 측에서 `llm_generate` 를 부르면 됨.

## 관련 이슈

- 이슈 #1: 포켓몬 스프라이트 → ASCII 아트 (브랜치 `feat/#1-ascii-art-sprite` 에서 진행).
- 이슈 #2: C 언어에서 LLM 호출 경로 — 본 모듈이 이 이슈에 해당.

## 작업 규칙

- 답변은 한국어로 (사용자 선호). 코드/주석은 한국어 주석 + 영어 식별자.
- 새 기능은 별도 브랜치 (`feat/#<issue>-<slug>`) 에서 작업하고 PR 로 머지.
- 빌드 산출물 (`pokemon`, `*.o`) 은 커밋하지 않는다 ([.gitignore](.gitignore) 참고).
