# Visual Studio / VS Code 실행 방법

이 프로젝트는 C 콘솔 프로그램입니다. 가장 안정적인 실행 방법은 CMake로 빌드하는 것입니다.

## 1) Visual Studio 2022에서 실행

1. Visual Studio Installer에서 **Desktop development with C++** 워크로드를 설치합니다.
2. Visual Studio를 열고 **File > Open > Folder...** 를 눌러 이 폴더(`pokemon-fin2`)를 엽니다.
3. 상단 실행 대상이 `pokemon.exe`로 잡히면 그대로 실행합니다.
4. 안 잡히면 메뉴에서 **Project > Configure Cache** 또는 CMake 구성을 새로고침합니다.

아래 명령은 LLM 기능을 끈 상태(`POKEMON_ENABLE_LLM=OFF`)로 빌드하므로 키 설정 없이 실행됩니다.
LLM 모듈의 HTTP 전송은 POSIX 소켓을 쓰므로 네이티브 Windows(MSVC)에서는 켤 수 없습니다.
LLM 기능까지 쓰려면 아래 3) 의 리눅스/WSL 방식으로 빌드하세요.

## 2) VS Code에서 실행

VS Code 터미널에서 아래 명령어를 입력합니다.

```powershell
cmake -S . -B build -DPOKEMON_ENABLE_LLM=OFF
cmake --build build --config Debug
.\build\Debug\pokemon.exe
```

만약 `Debug` 폴더가 없으면 아래도 시도하세요.

```powershell
.\build\pokemon.exe
```

## 3) 리눅스/WSL/Git Bash에서 실행

```bash
cmake -S . -B build -DPOKEMON_ENABLE_LLM=OFF
cmake --build build
./build/pokemon
```

## 참고

- 이 프로젝트의 README에는 `make` 명령어가 있지만, Windows 초보자 기준으로는 CMake 방식이 더 편합니다.
- LLM 모듈은 외부 라이브러리 의존성이 없지만 POSIX 소켓을 쓰므로, 네이티브 Windows 에서는
  `POKEMON_ENABLE_LLM=OFF` 가 안전합니다. LLM 까지 쓰려면 WSL/리눅스에서 `ON` 으로 빌드하세요.
