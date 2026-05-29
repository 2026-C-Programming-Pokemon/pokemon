# pokemon

C 로 만든 1세대 포켓몬 TUI 배틀 프로젝트입니다.

## 빌드 / 실행

```sh
make              # 빌드 (LLM 포함, libcurl 필요)
make LLM=0        # 빌드 (LLM 없이, 의존성 없음)
make run          # 빌드 + 실행
make clean        # 산출물 정리
```

LLM 쪽은 OpenAI 호환 `/v1/chat/completions` 엔드포인트를 바라봅니다.
기본값은 OpenAI지만, `LLM_BASE_URL` 로 Ollama 등 자체 호환 백엔드를 붙일 수 있습니다.

## 게임 LLM 설정

```sh
cp .env.example .env      # 열어서 OPENAI_API_KEY 채우기
sudo apt install -y libcurl4-openssl-dev   # Debian/Ubuntu 기준
make run
```

다른 OS 의 libcurl 패키지:
- Alpine: `curl-dev`
- Fedora: `libcurl-devel`
- macOS: 기본 포함
- Windows: WSL 권장

`.env` 대신 `export OPENAI_API_KEY=sk-...` 로 넣어도 됩니다.
키가 없거나 호출이 실패하면 호출 측에서 폴백 처리해야 합니다.

| 변수 | 기본값 | 설명 |
|---|---|---|
| `OPENAI_API_KEY` | - | OpenAI 사용 시 필수. 로컬 백엔드면 비울 수 있음. |
| `LLM_BASE_URL` | `https://api.openai.com/v1/chat/completions` | OpenAI 호환 엔드포인트. 예: `http://localhost:11434/v1/chat/completions` |
| `LLM_MODEL` | `gpt-4o-mini` | 모델 ID |
| `POKEMON_LLM_AI` | - | `1` 로 설정 시 상대 트레이너의 기술 선택을 LLM 에게 맡김 (실패시 휴리스틱 폴백) |
| `LLM_TIMEOUT_MS` | `30000` | LLM 호출 타임아웃 (ms). 배틀 중엔 짧게 잡는 편이 좋음 |

[.env.example](.env.example) 에 OpenAI / Ollama 백엔드 템플릿이 있습니다.

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
- [llm/](llm/) — libcurl 기반 LLM 호출 모듈 (OpenAI 호환)
- [score/](score/) — 리더보드/점수 저장
- [sound/](sound/) — 배경음악 파일
- [tools/ascii_converter/](tools/ascii_converter/) — PNG → 유니코드 ASCII 변환기 (Python)
