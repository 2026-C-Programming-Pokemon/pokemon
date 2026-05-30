# CLAUDE.md

이 파일은 이 레포에서 Claude Code 가 작업할 때 참고하는 컨텍스트 메모입니다.

## 프로젝트 개요

C 콘솔 1세대 포켓몬 배틀 게임. LLM 은 OpenAI API 직결로 호출한다 (별도 프록시 없음).

- [pokemon.c](pokemon.c): 실행 화면, 입력, 배경음악 제어, `main`
- [pokemon.h](pokemon.h): 공용 타입, 상수, 함수 선언
- [dogam/](dogam/): 포켓몬 도감 데이터, 타입 상성, 기본 능력치 계산
- [skill/](skill/): 기술 후보/고정 기술 데이터, 기술 배치
- [battlelogic/](battlelogic/): 데미지, 상태이상, 턴 진행, AI 기술 선택 (LLM/휴리스틱)
- [entry/](entry/): 플레이어/상대 트레이너 엔트리 구성
- [score/](score/): 점수/리더보드 저장
- [llm/](llm/): libcurl 로 OpenAI 호환 Chat Completions API 호출
- [tools/ascii_converter/](tools/ascii_converter/): PNG → 유니코드 ASCII 아트 변환 (Python)
- [docs/llm-ai.md](docs/llm-ai.md): LLM AI 사용법 / 검증 결과

## 빌드 / 실행

```sh
make          # pokemon 실행 파일 생성 (libcurl 필요)
make LLM=0    # libcurl 없이 빌드 (LLM 스텁)
make run      # 빌드 후 실행. stderr 는 llm.log 로 자동 분리
make clean    # 산출물 정리
```

또는 CMake (Windows 권장 — 기본 LLM OFF):
```sh
cmake -S . -B build && cmake --build build
```

- 컴파일러: `cc` (gcc/clang 어느 쪽이든 c99)
- 의존성 (LLM 활성 빌드 시): `libcurl`
  - Debian/Ubuntu: `apt install libcurl4-openssl-dev`
  - Alpine: `apk add curl-dev`
  - macOS: 시스템 기본 포함

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

배틀 AI 전용:
```c
int idx;
if (llm_pick_move_index(prompt, n_moves, &idx) == 0) {
    /* idx 가 0..n_moves-1 범위 보장 */
}
```

런타임 설정 (환경 변수):

| 변수 | 기본값 | 설명 |
|---|---|---|
| `OPENAI_API_KEY` | - | OpenAI 직결 시 필요 |
| `LLM_BASE_URL` | `https://api.openai.com/v1/chat/completions` | OpenAI 호환 엔드포인트 (Ollama 등) |
| `LLM_MODEL` | `gpt-4o-mini` | 모델 ID |
| `LLM_TIMEOUT_MS` | `30000` | 호출 타임아웃 (ms). 배틀 중엔 짧게 |
| `POKEMON_LLM_AI` | - | `1` 이면 상대 트레이너 기술 선택을 LLM 에 맡김 |
| `POKEMON_LLM_AI_DEBUG` | - | `1`=선택 로그, `2`=프롬프트도 |
| `POKEMON_LLM_AI_EXPLAIN` | (기본 ON) | LLM 한테 이유 한 줄 + 답 받기. `0` 으로 끔 |

설계 원칙:

- LLM 호출 실패는 게임을 멈추지 않는다 → 휴리스틱 폴백
- API 키는 코드에 하드코딩하지 않는다 (`.env` 또는 환경변수)
- 응답 JSON 파싱은 OpenAI Chat Completions 포맷에 맞춘 단순 추출

## 배경음악 관련 메모

- Windows: PowerShell `MediaPlayer` 기반 루프 재생
- Unix/macOS: 사용 가능한 플레이어를 탐색해서 재생
  - `ffplay`, `mpv`, `mpg123`, `cvlc`, `vlc`, (macOS 한정) `afplay`
- 플레이어가 없으면 음악은 **조용히 비활성화**되어야 한다
- 없는 플레이어를 무한 재시도해서 CPU 를 태우는 busy loop 는 금지
- 음악 프로세스 종료는 가능하면 개별 PID + 프로세스 그룹 기준으로 정리

## 작업 규칙

- 답변은 한국어로 (사용자 선호). 코드/주석은 한국어 주석 + 영어 식별자.
- 새 기능은 별도 브랜치에서 작업하고 PR 로 머지.
- 빌드 산출물 (`pokemon`, `*.o`, `llm.log`) 은 커밋하지 않는다.
