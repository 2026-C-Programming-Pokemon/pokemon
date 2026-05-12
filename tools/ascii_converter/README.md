# ASCII Converter

포켓몬 스프라이트 이미지를 터미널용 문자 아트로 변환하기 위한 보조 도구 폴더입니다.

## 현재 기준 방향
이 프로젝트에서는 여러 실험 끝에 **더 어두운 반블록 렌더링(dark halfblock)** 을 기본 기준으로 잡습니다.

이유:
- 포켓몬 형체가 비교적 잘 보임
- 밝은 포켓몬이 배경에 날아가는 현상이 줄어듦
- TUI에 붙였을 때 크기 대비 식별성이 괜찮음

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

기본 추천 실행 예시:

```sh
python3 tools/ascii_converter/convert_sprite.py 025.png --mode halfblocks --width 64
```

현재는 스크립트 내부 임계값을 더 어둡게 조정해둔 상태라,
`halfblocks + width 64` 조합을 기준안으로 보면 됩니다.

## 출력
- `output/<이름>.halfblocks.dark.w64.txt` 같은 사람이 보는 결과
- `generated/<이름>.halfblocks.dark.w64.inc` 같은 C 포함용 결과

## 메모
- 순수 ASCII, 점자, 컬러 반블록 등은 실험했지만 현재는 정리 대상입니다.
- 필요하면 나중에 다시 실험 브랜치에서 분기할 수 있습니다.
