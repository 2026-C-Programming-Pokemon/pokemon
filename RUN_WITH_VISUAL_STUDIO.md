# Visual Studio / VS Code 실행 방법

이 프로젝트는 C 콘솔 프로그램입니다. 가장 안정적인 실행 방법은 CMake로 빌드하는 것입니다.

## 1) Visual Studio 2022에서 실행

1. Visual Studio Installer에서 **Desktop development with C++** 워크로드를 설치합니다.
2. Visual Studio를 열고 **File > Open > Folder...** 를 눌러 이 폴더(`pokemon-fin2`)를 엽니다.
3. 상단 실행 대상이 `pokemon.exe`로 잡히면 그대로 실행합니다.
4. 안 잡히면 메뉴에서 **Project > Configure Cache** 또는 CMake 구성을 새로고침합니다.

외부 라이브러리 설치는 필요 없습니다. LLM 모듈의 소켓 통신은 Windows(winsock)·
POSIX 양쪽을 지원하며, CMake 가 Windows 에서 `ws2_32` 를 자동으로 링크합니다.
LLM 설정이 없거나 호출이 실패하면 게임이 알아서 폴백하므로 키 없이도 실행됩니다.

## 2) VS Code에서 실행

VS Code 터미널에서 아래 명령어를 입력합니다.

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\pokemon.exe
```

만약 `Debug` 폴더가 없으면 아래도 시도하세요.

```powershell
.\build\pokemon.exe
```

## 3) 리눅스/WSL/Git Bash에서 실행

```bash
cmake -S . -B build
cmake --build build
./build/pokemon
```

## 참고

- 이 프로젝트의 README에는 `make` 명령어가 있지만, Windows 초보자 기준으로는 CMake 방식이 더 편합니다.
- LLM 모듈은 외부 라이브러리 의존성이 없습니다. 게임 안에서 LLM 을 실제로 쓰려면
  `LLM_BASE_URL` / `POKEMON_CLIENT_SECRET` 환경변수를 설정하세요(없어도 게임은 동작).
