# ASCII Art Plan

## 방향
- 런타임 C 이미지 처리 대신, 사전 생성형 Python 도구 사용
- 생성된 ASCII 결과를 C 코드에 정적 문자열로 포함

## 흐름
1. 스프라이트 이미지를 `tools/ascii_converter/input/`에 넣기
2. Python 스크립트로 ASCII 텍스트 생성
3. 결과를 `output/`에서 확인
4. C 코드에 옮기기 좋은 포맷은 `generated/`에 저장

## 이유
- C 본체는 배틀/TUI 로직에 집중
- 변환 도구는 Python으로 빠르게 개발 가능
- 과제 제출 시 구조가 단순하고 유지보수 쉬움
