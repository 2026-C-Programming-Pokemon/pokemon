# ASCII Converter

포켓몬 스프라이트 이미지를 터미널용 문자 아트로 변환하기 위한 보조 도구 폴더입니다.

## 목적
- 이미지 기반 포켓몬 스프라이트를 터미널용 문자 아트로 변환
- 특히 포켓몬 도트 스프라이트에 맞게 **반블록 / 블록 문자 기반 출력**을 우선 사용
- 변환 결과를 C 코드에 붙이기 쉬운 형태로 저장
- 런타임 의존성이 아니라, 사전 생성용 도구로 사용

## 폴더 구조
- `input/` : 원본 스프라이트 이미지 넣는 곳
- `bulk_input/` : 일괄 다운로드한 스프라이트 모음
- `output/` : 사람이 확인하기 쉬운 `.txt` 결과
- `generated/` : C에 옮기기 좋은 생성 결과물

## 사용법

Pillow가 필요합니다.

```sh
python3 -m pip install Pillow
```

기본 추천 모드는 `halfblocks` 입니다.

```sh
python3 tools/ascii_converter/convert_sprite.py pikachu.png --mode halfblocks --width 48
```

다른 위치의 이미지도 직접 지정할 수 있습니다.

```sh
python3 tools/ascii_converter/convert_sprite.py ./sprite.png --mode color-halfblocks --width 48
```

기본 출력:
- `output/<이미지이름>.<mode>.txt` : 사람이 읽기 쉬운 결과
- `generated/<이미지이름>.<mode>.inc` : C 코드에 붙이기 쉬운 `static const char *...[]` 조각

## 옵션

```sh
python3 tools/ascii_converter/convert_sprite.py --help
```

주요 옵션:
- `--mode ascii` : 전통적인 명암 기반 ASCII 모드
- `--mode blocks` : 단순 블록 문자 기반 모드
- `--mode halfblocks` : `▀`, `▄`, `█` 기반 반블록 모드 (추천)
- `--mode color-halfblocks` : ANSI 색 + 반블록 모드
- `--width` : 결과 가로 문자 수
- `--chars` : ASCII 모드에서 사용할 문자 목록
- `--invert` : 문자 밝기 매핑 반전
- `--output` : 사람이 읽는 `.txt` 출력 경로 지정
- `--generated` : C용 생성 파일 경로 지정
- `--c-name` : C 변수 이름 지정

## 메모
- 순수 ASCII보다 반블록/컬러 반블록이 포켓몬 도트 스프라이트에 더 잘 맞습니다.
- 실제 TUI 반영 전에는 `output/` 결과를 먼저 눈으로 확인하는 것을 권장합니다.
