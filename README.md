# pokemon

C 로 만든 1세대 포켓몬 TUI 배틀 (AI 로 싸지름).

> 현재 `main` 은 LLM 연동 확인용 데모입니다. 게임 루프/입력은 미구현이고, 1대1 자동 배틀 한 판만 돌고 종료합니다.

## 명령어

```sh
make              # 빌드 (LLM 포함, libcurl 필요)
make LLM=0        # 빌드 (LLM 없이, 의존성 없음)
make run          # 빌드 + 실행
make clean        # 산출물 정리
```

## 처음 한 번 (LLM 켜고 쓸 때)

```sh
cp .env.example .env      # 열어서 OPENAI_API_KEY 채우기
sudo apt install -y libcurl4-openssl-dev   # Debian/Ubuntu 기준
make run
```

다른 OS 의 libcurl 패키지: Alpine `curl-dev`, Fedora `libcurl-devel`, macOS 는 기본 포함, Windows 는 WSL 권장.

`.env` 안 쓰고 `export OPENAI_API_KEY=sk-...` 해도 됩니다. 키가 없거나 호출 실패 시 폴백 메시지로 자동 대체.

| 변수 | 기본값 | 설명 |
|---|---|---|
| `OPENAI_API_KEY` | - | OpenAI 사용 시 필수. Ollama 로컬은 비워둬도 됨. |
| `LLM_BASE_URL` | `https://api.openai.com/v1/chat/completions` | OpenAI 호환 엔드포인트. Ollama: `http://localhost:11434/v1/chat/completions` |
| `LLM_MODEL` | `gpt-4o-mini` | 모델 ID. Ollama 면 `gemma3:4b` 등. |

[.env.example](.env.example) 에 OpenAI / Ollama / 호환 백엔드별 설정 템플릿이 있습니다.

## 구조

- [pokemon.c](pokemon.c) — 실행 화면, 입력, `main`.
- [pokemon.h](pokemon.h) — 공용 타입, 상수, 함수 선언.
- [dogam/](dogam/) — 포켓몬 도감 데이터, 타입 상성, 기본 능력치 계산.
- [skill/](skill/) — 기술 후보/고정 기술 데이터, 기술 배치.
- [battlelogic/](battlelogic/) — 데미지, 상태이상, 턴 진행, AI 기술 선택.
- [entry/](entry/) — 플레이어/상대 트레이너 엔트리 구성.
- [llm/](llm/) — LLM 호출 모듈. 현재 OpenAI Chat Completions. `LLM_DISABLED` 매크로로 스텁 빌드 가능 (Ollama / 다른 공급자 붙일 때 이 인터페이스만 유지).
- [tools/ascii_converter/](tools/ascii_converter/) — PNG → 유니코드 ASCII 변환기 (Python).
