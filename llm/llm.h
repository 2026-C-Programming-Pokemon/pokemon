#ifndef POKEMON_LLM_H
#define POKEMON_LLM_H

#include <stddef.h>

/*
 * 외부 LLM (OpenAI 호환 Chat Completions API) 을 호출하는 작은 모듈.
 *
 * - 빌드/링크 의존성 없음. 호출은 시스템 curl 바이너리를 통한다.
 * - 환경변수: OPENAI_API_KEY, LLM_BASE_URL, LLM_MODEL, LLM_TIMEOUT_MS.
 * - 키가 없거나 curl 이 없거나 호출이 실패하면 -1 을 돌려준다.
 *   호출 측은 -1 일 때 휴리스틱으로 폴백한다.
 */

/* OPENAI_API_KEY 나 LLM_BASE_URL 이 설정되어 있으면 1, 아니면 0. */
int llm_is_available(void);

/* prompt 를 보내고 응답 텍스트를 out 에 채운다. 성공 0, 실패 -1. */
int llm_generate(const char *prompt, char *out, size_t out_size);

/* 배틀 AI 전용: 응답에서 "1..move_count" 범위 첫 정수를 0-based 로 돌려준다. */
int llm_pick_move_index(const char *prompt, int move_count, int *out_index);

#endif
