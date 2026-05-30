/*
 * 시스템에 깔린 curl 바이너리를 호출해서 LLM 응답을 받아오는 모듈.
 * 외부 라이브러리 의존성이 없다.
 *
 * 흐름:
 *   1. JSON 본문을 임시 파일에 쓴다.
 *   2. system() 으로 curl 을 실행해서 응답을 다른 임시 파일에 쓴다.
 *   3. 응답 JSON 에서 content 텍스트만 뽑아 호출 측에 돌려준다.
 *
 * 본문은 파일로 주고받기 때문에 셸 이스케이프 걱정이 없다.
 */

#include "llm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <process.h>      /* _getpid */
#  define getpid    _getpid
#  define NULL_DEV  "NUL"
#  define TMP_ENV   "TEMP"
#  define TMP_FALLBACK "."
#  define PATH_SEP "\\"
#else
#  include <unistd.h>       /* getpid */
#  define NULL_DEV  "/dev/null"
#  define TMP_ENV   "TMPDIR"
#  define TMP_FALLBACK "/tmp"
#  define PATH_SEP "/"
#endif

#define LLM_DEFAULT_URL   "https://api.openai.com/v1/chat/completions"
#define LLM_DEFAULT_MODEL "gpt-4o-mini"
#define LLM_MAX_TOKENS    256

int llm_is_available(void)
{
    const char *key = getenv("OPENAI_API_KEY");
    const char *url = getenv("LLM_BASE_URL");
    return (key && key[0]) || (url && url[0]);
}

/* JSON 문자열에 들어갈 수 있게 ", \, 개행만 이스케이프. 우리 프롬프트엔
 * 그 외 제어문자가 들어올 일이 없다. */
static void json_escape(const char *in, char *out, size_t out_size)
{
    size_t o = 0;
    for (size_t i = 0; in[i] && o + 2 < out_size; i++) {
        char c = in[i];
        if (c == '"' || c == '\\')      { out[o++] = '\\'; out[o++] = c; }
        else if (c == '\n')             { out[o++] = '\\'; out[o++] = 'n'; }
        else if (c == '\r')             { out[o++] = '\\'; out[o++] = 'r'; }
        else                            { out[o++] = c; }
    }
    out[o] = '\0';
}

/* 응답 JSON 에서 "content":"..." 의 ... 부분만 뽑아낸다. \u 같은 비주류
 * 이스케이프는 '?' 로 치환. */
static int extract_content(const char *json, char *out, size_t out_size)
{
    const char *p = strstr(json, "\"content\"");
    if (!p) return -1;
    p += 9;
    while (*p == ' ' || *p == '\t' || *p == ':' || *p == '\n' || *p == '\r') p++;
    if (*p != '"') return -1;
    p++;

    size_t o = 0;
    while (*p && *p != '"' && o + 1 < out_size) {
        if (*p == '\\' && p[1]) {
            switch (p[1]) {
                case 'n': out[o++] = '\n'; break;
                case 't': out[o++] = '\t'; break;
                case 'r': out[o++] = '\r'; break;
                case 'u': out[o++] = '?'; p += 4; break;  /* \uXXXX 건너뜀 */
                default:  out[o++] = p[1]; break;          /* ", \, /, b, f 포함 */
            }
            p += 2;
        } else {
            out[o++] = *p++;
        }
    }
    out[o] = '\0';
    return (*p == '"') ? 0 : -1;
}

/* OS 별 임시 디렉터리에 "llm_<pid>_<태그>" 형식 경로를 만든다. */
static void temp_path(char *out, size_t out_size, const char *tag)
{
    const char *dir = getenv(TMP_ENV);
    if (!dir || !dir[0]) dir = TMP_FALLBACK;
    snprintf(out, out_size, "%s%sllm_%d_%s", dir, PATH_SEP, (int)getpid(), tag);
}

int llm_generate(const char *prompt, char *out, size_t out_size)
{
    if (!out || out_size == 0) return -1;
    out[0] = '\0';

    const char *key   = getenv("OPENAI_API_KEY");
    const char *url   = getenv("LLM_BASE_URL");
    const char *model = getenv("LLM_MODEL");
    if (!url   || !url[0])   url   = LLM_DEFAULT_URL;
    if (!model || !model[0]) model = LLM_DEFAULT_MODEL;

    /* OpenAI 직결인데 키가 없으면 호출할 의미가 없다. */
    if (strstr(url, "api.openai.com") && (!key || !key[0])) return -1;

    long timeout_s = 30;
    const char *ts = getenv("LLM_TIMEOUT_MS");
    if (ts && ts[0]) {
        long ms = strtol(ts, NULL, 10);
        if (ms > 0) timeout_s = (ms + 999) / 1000;
    }

    /* 본문 직렬화. 이스케이프는 길이가 최대 2배까지 부풀어날 수 있다. */
    size_t esc_cap = strlen(prompt) * 2 + 16;
    char *escaped = malloc(esc_cap);
    if (!escaped) return -1;
    json_escape(prompt, escaped, esc_cap);

    size_t body_cap = esc_cap + 256;
    char *body = malloc(body_cap);
    if (!body) { free(escaped); return -1; }
    snprintf(body, body_cap,
        "{\"model\":\"%s\",\"max_tokens\":%d,"
        "\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
        model, LLM_MAX_TOKENS, escaped);
    free(escaped);

    /* 본문/응답 임시 파일. */
    char req_path[512], resp_path[512];
    temp_path(req_path,  sizeof(req_path),  "req.json");
    temp_path(resp_path, sizeof(resp_path), "resp.json");

    FILE *rf = fopen(req_path, "wb");
    if (!rf) { free(body); return -1; }
    fwrite(body, 1, strlen(body), rf);
    fclose(rf);
    free(body);

    /* curl 명령어 조립. */
    char cmd[4096];
    if (key && key[0]) {
        snprintf(cmd, sizeof(cmd),
            "curl -sS -X POST "
            "-H \"content-type: application/json\" "
            "-H \"Authorization: Bearer %s\" "
            "--data-binary @\"%s\" --max-time %ld -o \"%s\" \"%s\" 2>%s",
            key, req_path, timeout_s, resp_path, url, NULL_DEV);
    } else {
        snprintf(cmd, sizeof(cmd),
            "curl -sS -X POST "
            "-H \"content-type: application/json\" "
            "--data-binary @\"%s\" --max-time %ld -o \"%s\" \"%s\" 2>%s",
            req_path, timeout_s, resp_path, url, NULL_DEV);
    }

    int sys_rc = system(cmd);
    remove(req_path);

    int result = -1;
    if (sys_rc == 0) {
        FILE *resp = fopen(resp_path, "rb");
        if (resp) {
            char buf[8192];
            size_t n = fread(buf, 1, sizeof(buf) - 1, resp);
            buf[n] = '\0';
            fclose(resp);
            if (extract_content(buf, out, out_size) == 0) result = 0;
        }
    }
    remove(resp_path);
    return result;
}

int llm_pick_move_index(const char *prompt, int move_count, int *out_index)
{
    if (!out_index || move_count <= 0 || move_count > 9) return -1;

    char resp[256];
    if (llm_generate(prompt, resp, sizeof(resp)) != 0) return -1;

    /* 응답에서 첫 자릿수 한 글자만 본다. */
    for (size_t i = 0; resp[i]; i++) {
        if (resp[i] >= '1' && resp[i] <= '9') {
            int n = resp[i] - '0';
            if (n <= move_count) { *out_index = n - 1; return 0; }
            return -1;
        }
    }
    return -1;
}
