# CLAUDE.md

이 파일은 이 레포에서 Claude Code 가 작업할 때 참고하는 컨텍스트 메모입니다.

## 프로젝트 개요

- C 언어로 작성한 1세대 포켓몬 TUI 배틀 과제.
- 핵심 게임 코드: [pokemon.c](pokemon.c) (단일 파일, 1세대 81마리 + 기술 데이터 + 배틀 로직).
- 스프라이트 변환 도구: [tools/ascii_converter/](tools/ascii_converter/) — Python 으로 PNG 를 다크 하프블록 유니코드 아트로 변환하여 `generated/*.inc` 에 저장.
- 외부 LLM 호출 모듈: [llm/](llm/) — libcurl 로 Anthropic Messages API 호출.

## 빌드 / 실행

```sh
make          # pokemon 실행 파일 생성
make run      # 빌드 후 실행
make clean    # 산출물 정리
```

- 컴파일러: `cc` (gcc/clang 어느 쪽이든 c99 지원이면 됨).
- 의존성: `libcurl` (Anthropic API 호출용).
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
| `ANTHROPIC_API_KEY` | yes | - | 없으면 `llm_is_available()` 가 0 을 돌려주고 호출 시 -1. |
| `LLM_MODEL` | no | `claude-haiku-4-5` | 사용할 모델 ID. |

설계 원칙:

- LLM 호출 실패는 게임을 멈추지 않는다. 호출 측은 항상 폴백 메시지를 준비한다 (예: [pokemon.c](pokemon.c) 의 `printBattleIntro`).
- API 키는 코드에 하드코딩하지 않는다 (환경 변수만).
- 응답 JSON 파싱은 Anthropic Messages 응답에 한정된 단순 추출 ([llm/llm.c](llm/llm.c) 의 `extract_text`). 다른 공급자로 옮기면 다시 작성 필요.

## 현재 LLM 연결 지점

- [pokemon.c](pokemon.c) `printBattleIntro` — 1대1 배틀 시작 시 오프닝 한 줄을 LLM 으로 생성. 시범 적용.
- 확장 후보: `attackPokemon` 의 효과 굉장했을 때 대사, `battleOneOnOne` 의 승리 선언 대사.

## 관련 이슈

- 이슈 #1: 포켓몬 스프라이트 → ASCII 아트 (브랜치 `feat/#1-ascii-art-sprite` 에서 진행).
- 이슈 #2: C 언어에서 LLM 호출 경로 — 본 모듈이 이 이슈에 해당.

## 작업 규칙

- 답변은 한국어로 (사용자 선호). 코드/주석은 한국어 주석 + 영어 식별자.
- 새 기능은 별도 브랜치 (`feat/#<issue>-<slug>`) 에서 작업하고 PR 로 머지.
- 빌드 산출물 (`pokemon`, `*.o`) 은 커밋하지 않는다 ([.gitignore](.gitignore) 참고).
