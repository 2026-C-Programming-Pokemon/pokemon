#ifndef POKEMON_LLM_H
#define POKEMON_LLM_H

#include <stddef.h>

/*
 * 포켓몬 배틀용 LLM 호출 모듈.
 *
 * 외부 라이브러리 의존성이 없습니다. HTTP 전송은 POSIX 소켓으로 직접 구현했고,
 * HMAC-SHA256 은 공개 도메인 코드(llm/sha256.*, llm/hmac_sha256.*)를 씁니다.
 * 따라서 추가 설치 없이 컴파일만 하면 동작합니다.
 *
 * 단, 이 클라이언트는 평문 HTTP 만 지원합니다 (HTTPS/TLS 미지원). OpenAI 같은
 * https 엔드포인트를 직접 부를 수 없으므로, app/ 의 보안 프록시를 경유하는 것이
 * 기본 사용법입니다. 프록시는 같은 머신/LAN 에서 http 로 접근하거나, 외부 노출이
 * 필요하면 nginx/Caddy 가 TLS 를 종단하도록 앞단에 둡니다.
 *
 * 두 가지 동작 모드:
 *
 *  1) 프록시 모드 — app/ 의 FastAPI 보안 프록시를 경유 (기본/권장).
 *     - POKEMON_CLIENT_SECRET 환경변수가 설정돼 있으면 자동으로 켜집니다.
 *     - 요청 본문에 HMAC-SHA256 서명을 붙여 통신 규격(docs/proxy-protocol.md)
 *       을 따릅니다. 업스트림 키는 프록시가 채우므로 클라이언트엔 없습니다.
 *
 *  2) 직결 모드 — OpenAI 호환 평문 HTTP 엔드포인트를 직접 호출 (예: 로컬 Ollama).
 *     - OPENAI_API_KEY 가 있으면 Authorization 헤더로 보냅니다.
 *
 * 환경 변수:
 *   LLM_BASE_URL          OpenAI 호환 엔드포인트 (http://). 기본: 로컬 프록시.
 *   LLM_MODEL             모델 ID (기본: gpt-4o-mini).
 *   POKEMON_CLIENT_SECRET 설정 시 프록시 모드 + HMAC 서명 활성화.
 *   POKEMON_CLIENT_ID     클라이언트 식별자 (기본: pokemon-c-client/1.0).
 *   OPENAI_API_KEY        직결 모드에서 Authorization 으로 사용.
 *
 * 키가 없거나 호출이 실패하면 함수가 -1 을 돌려주고, 호출 측은 LLM 없이도
 * 게임이 정상 동작하도록 폴백 메시지를 써야 합니다.
 */

/* 초기화 훅. 현재 구현은 전역 상태가 없어 아무 일도 하지 않지만, API 호환과
 * 향후 확장(예: Windows winsock 초기화)을 위해 남겨둡니다. 시작 시 한 번 호출. */
int llm_init(void);

/* 정리 훅. 종료 직전에 한 번 호출합니다. */
void llm_cleanup(void);

/* LLM 사용 가능 여부 (API 키 또는 LLM_BASE_URL 이 설정되어 있는지). */
int llm_is_available(void);

/*
 * 동기 호출. prompt 를 user 메시지로 보내고 out_buffer 에 응답 텍스트를 채웁니다.
 * 성공 시 0, 실패 시 -1. 응답은 항상 NUL 종료되며 out_size-1 길이로 잘립니다.
 */
int llm_generate(const char *prompt, char *out_buffer, size_t out_size);

/*
 * 배틀 기술 선택 헬퍼.
 *
 * situation 은 현재 배틀 상황과 1번부터 move_count 번까지의 기술 목록을 적은,
 * 사람이 읽을 수 있는 문자열입니다. 내부에서 "1~move_count 중 숫자 하나만
 * 출력하라"는 system 프롬프트를 붙여 LLM 에 묻고, 응답에서 그 숫자를 파싱합니다.
 *
 * 성공 시 고른 기술 번호(1..move_count), 실패 시 -1 을 반환합니다.
 * 실패하면(키 없음 / 통신 실패 / 응답 파싱 실패) 호출 측이 직접 무작위 선택
 * 등으로 폴백해야 합니다.
 */
int llm_choose_move(const char *situation, int move_count);

#endif /* POKEMON_LLM_H */
