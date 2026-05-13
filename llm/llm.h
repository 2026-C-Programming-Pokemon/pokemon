#ifndef POKEMON_LLM_H
#define POKEMON_LLM_H

#include <stddef.h>

/*
 * 포켓몬 배틀용 LLM 호출 모듈.
 *
 * - Anthropic Messages API 를 libcurl 로 호출합니다.
 * - 환경 변수 ANTHROPIC_API_KEY 를 사용합니다.
 * - 모델은 LLM_MODEL 환경 변수로 덮어쓸 수 있고, 없으면 claude-haiku-4-5 입니다.
 * - 키가 없거나 호출이 실패하면 함수가 -1 을 돌려주고, 호출 측은 LLM
 *   없이도 게임이 정상 동작하도록 폴백 메시지를 써야 합니다.
 */

/* 전역 libcurl 초기화. 프로그램 시작 시 한 번 호출합니다. */
int llm_init(void);

/* 전역 libcurl 정리. 프로그램 종료 직전에 한 번 호출합니다. */
void llm_cleanup(void);

/* LLM 사용 가능 여부 (API 키가 설정되어 있는지). */
int llm_is_available(void);

/*
 * 동기 호출. prompt 를 보내고 out_buffer 에 응답 텍스트를 채워 넣습니다.
 * 성공 시 0, 실패 시 -1 을 반환합니다.
 * 응답은 항상 NUL 종료되며 out_size-1 길이로 잘립니다.
 */
int llm_generate(const char *prompt, char *out_buffer, size_t out_size);

#endif /* POKEMON_LLM_H */
