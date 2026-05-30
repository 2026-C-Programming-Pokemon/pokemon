# pokemon

C 로 만든 1세대 포켓몬 콘솔 배틀 게임.

## 실행하기

### Windows

Visual Studio 2022 (C++ 개발 워크로드) 만 깔려 있으면 됩니다.

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\pokemon.exe
```

또는 Visual Studio 에서 폴더 열고 그대로 실행.

### macOS / Linux

```sh
make run
```

> 외부 라이브러리 의존성 없음. LLM 은 시스템에 깔린 `curl` 바이너리를 쓰며,
> Windows 10 1803+, macOS, 대부분의 Linux 배포판에 기본 포함되어 있습니다.

## 조작

- 번호 키로 메뉴 / 기술 선택
- 사천왕을 차례로 쓰러뜨리면 클리어

## AI 를 LLM 한테 맡기기 (선택)

상대 트레이너의 기술 선택을 ChatGPT 같은 LLM 에 맡길 수 있습니다.
설정 안 하면 기본 AI 로 동작합니다.

1. `.env.example` 을 복사해서 `.env` 만들고 `OPENAI_API_KEY` 채우기
2. 실행 전에 `POKEMON_LLM_AI=1` 환경변수 켜기

| 변수 | 설명 |
|---|---|
| `OPENAI_API_KEY` | OpenAI 키 |
| `POKEMON_LLM_AI` | `1` 이면 LLM 이 기술 선택 |
| `LLM_BASE_URL` | OpenAI 말고 Ollama 등 다른 백엔드 쓰고 싶을 때 |
| `LLM_MODEL` | 기본 `gpt-4o-mini` |
| `LLM_TIMEOUT_MS` | 한 턴 기다리는 최대 시간 (ms) |

LLM 호출이 실패하거나 `curl` 이 없으면 자동으로 기본 AI 로 돌아갑니다.
자세한 내용은 [docs/llm-ai.md](docs/llm-ai.md).

## 폴더

- `pokemon.c` / `pokemon.h` — 메인 루프, 입출력
- `dogam/` — 포켓몬 도감, 타입 상성
- `skill/` — 기술 데이터
- `battlelogic/` — 데미지, 상태이상, AI
- `entry/` — 트레이너 구성
- `llm/` — LLM 호출 (시스템 curl 경유)
- `score/` — 점수판
- `sound/` — 배경음악
