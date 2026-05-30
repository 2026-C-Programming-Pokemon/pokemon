/*
 * LLM 호출 모듈. 외부 라이브러리 의존성이 없다.
 *
 * 시스템에 깔린 curl 바이너리를 system() 으로 호출해서 OpenAI 호환
 * Chat Completions 엔드포인트에 POST 한다.
 *
 * curl 은 Windows 10 1803+, macOS, 거의 모든 Linux 배포판에 기본 설치되어
 * 있다. 없으면 호출이 실패하고 게임은 휴리스틱으로 폴백한다.
 *
 * 요청 본문과 응답은 임시 파일로 주고받는다 (셸 이스케이프 회피).
 */

#include "llm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <process.h>     /* _getpid */
#  define LLM_GETPID _getpid
#else
#  include <unistd.h>      /* getpid, unlink */
#  define LLM_GETPID getpid
#endif

#define LLM_DEFAULT_URL   "https://api.openai.com/v1/chat/completions"
#define LLM_DEFAULT_MODEL "gpt-4o-mini"
#define LLM_MAX_TOKENS    256

int llm_init(void)    { return 0; }
void llm_cleanup(void) { }

int llm_is_available(void)
{
    const char *key = getenv("OPENAI_API_KEY");
    const char *url = getenv("LLM_BASE_URL");
    if (key != NULL && key[0] != '\0') return 1;
    if (url != NULL && url[0] != '\0') return 1;
    return 0;
}

/* JSON 문자열 안에 들어갈 수 있게 입력을 이스케이프한다. */
static void json_escape(const char *in, char *out, size_t out_size)
{
    size_t o = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        unsigned char c = (unsigned char)in[i];
        const char *seq = NULL;
        char unicode_buf[8];
        size_t need;

        switch (c) {
            case '"':  seq = "\\\""; break;
            case '\\': seq = "\\\\"; break;
            case '\n': seq = "\\n";  break;
            case '\r': seq = "\\r";  break;
            case '\t': seq = "\\t";  break;
            case '\b': seq = "\\b";  break;
            case '\f': seq = "\\f";  break;
            default:
                if (c < 0x20) {
                    snprintf(unicode_buf, sizeof(unicode_buf), "\\u%04x", c);
                    seq = unicode_buf;
                }
                break;
        }

        if (seq != NULL) {
            need = strlen(seq);
            if (o + need + 1 >= out_size) break;
            memcpy(out + o, seq, need);
            o += need;
        } else {
            if (o + 1 + 1 >= out_size) break;
            out[o++] = (char)c;
        }
    }
    out[o] = '\0';
}

/* 응답 JSON 에서 choices[0].message.content 값을 뽑는다. */
static int extract_text(const char *json, char *out, size_t out_size)
{
    const char *p = strstr(json, "\"message\"");
    if (p == NULL) return -1;

    p = strstr(p, "\"content\"");
    if (p == NULL) return -1;

    p = strchr(p + 9, ':');
    if (p == NULL) return -1;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p != '"') return -1;
    p++;

    size_t o = 0;
    while (*p != '\0') {
        if (*p == '\\' && *(p + 1) != '\0') {
            char esc = *(p + 1);
            char decoded;
            switch (esc) {
                case '"':  decoded = '"';  break;
                case '\\': decoded = '\\'; break;
                case '/':  decoded = '/';  break;
                case 'n':  decoded = '\n'; break;
                case 'r':  decoded = '\r'; break;
                case 't':  decoded = '\t'; break;
                case 'b':  decoded = '\b'; break;
                case 'f':  decoded = '\f'; break;
                case 'u':
                    if (o + 1 >= out_size) break;
                    out[o++] = '?';
                    p += 6;
                    continue;
                default:   decoded = esc;  break;
            }
            if (o + 1 >= out_size) break;
            out[o++] = decoded;
            p += 2;
        } else if (*p == '"') {
            out[o] = '\0';
            return 0;
        } else {
            if (o + 1 >= out_size) break;
            out[o++] = *p;
            p++;
        }
    }
    out[o] = '\0';
    return -1;
}

/* 임시 파일 경로를 만든다. 같은 프로세스가 동시에 여러 번 호출돼도 안 겹치게
 * pid + 카운터를 섞는다. */
static void temp_path(char *out, size_t out_size, const char *suffix, int seq)
{
#ifdef _WIN32
    const char *dir = getenv("TEMP");
    if (dir == NULL || dir[0] == '\0') dir = ".";
    snprintf(out, out_size, "%s\\llm_%d_%d_%s", dir, (int)LLM_GETPID(), seq, suffix);
#else
    const char *dir = getenv("TMPDIR");
    if (dir == NULL || dir[0] == '\0') dir = "/tmp";
    snprintf(out, out_size, "%s/llm_%d_%d_%s", dir, (int)LLM_GETPID(), seq, suffix);
#endif
}

int llm_generate(const char *prompt, char *out_buffer, size_t out_size)
{
    if (out_buffer == NULL || out_size == 0) return -1;
    out_buffer[0] = '\0';

    const char *api_key = getenv("OPENAI_API_KEY");
    const char *url     = getenv("LLM_BASE_URL");
    if (url == NULL || url[0] == '\0') url = LLM_DEFAULT_URL;

    /* OpenAI 직결인데 키가 없으면 의미가 없다. */
    if (strstr(url, "api.openai.com") != NULL && (api_key == NULL || api_key[0] == '\0')) {
        return -1;
    }

    const char *model = getenv("LLM_MODEL");
    if (model == NULL || model[0] == '\0') model = LLM_DEFAULT_MODEL;

    long timeout_ms = 30000;
    const char *to_env = getenv("LLM_TIMEOUT_MS");
    if (to_env != NULL && to_env[0] != '\0') {
        long v = strtol(to_env, NULL, 10);
        if (v > 0) timeout_ms = v;
    }

    /* 본문 직렬화 */
    size_t prompt_len = strlen(prompt);
    size_t esc_size = prompt_len * 6 + 16;
    char *escaped = (char *)malloc(esc_size);
    if (escaped == NULL) return -1;
    json_escape(prompt, escaped, esc_size);

    size_t body_cap = esc_size + 512;
    char *body = (char *)malloc(body_cap);
    if (body == NULL) { free(escaped); return -1; }
    snprintf(body, body_cap,
        "{\"model\":\"%s\",\"max_tokens\":%d,"
        "\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
        model, LLM_MAX_TOKENS, escaped);
    free(escaped);

    /* 본문/응답 임시 파일 */
    static int seq = 0;
    int my_seq = ++seq;
    char body_path[512], resp_path[512];
    temp_path(body_path, sizeof(body_path), "req.json", my_seq);
    temp_path(resp_path, sizeof(resp_path), "resp.json", my_seq);

    FILE *bf = fopen(body_path, "wb");
    if (bf == NULL) { free(body); return -1; }
    fwrite(body, 1, strlen(body), bf);
    fclose(bf);
    free(body);

    /* curl 명령어 조립. 본문은 파일에서 읽으므로 셸 이스케이프 걱정이 없다.
     * URL/키는 우리가 제어하는 값이라 안전하지만, 일단 큰따옴표로 감싸 둔다. */
    char cmd[4096];
    if (api_key != NULL && api_key[0] != '\0') {
        snprintf(cmd, sizeof(cmd),
            "curl -sS -X POST "
            "-H \"content-type: application/json\" "
            "-H \"Authorization: Bearer %s\" "
            "--data-binary @\"%s\" "
            "--max-time %ld "
            "-o \"%s\" "
            "\"%s\"",
            api_key, body_path, (timeout_ms + 999) / 1000, resp_path, url);
    } else {
        snprintf(cmd, sizeof(cmd),
            "curl -sS -X POST "
            "-H \"content-type: application/json\" "
            "--data-binary @\"%s\" "
            "--max-time %ld "
            "-o \"%s\" "
            "\"%s\"",
            body_path, (timeout_ms + 999) / 1000, resp_path, url);
    }

    /* Windows 에서 콘솔 창이 깜빡이지 않게 stderr 도 묻는다. 디버깅 어려워지면
     * LLM_VERBOSE=1 로 살릴 수 있게 한다. */
    const char *verbose = getenv("LLM_VERBOSE");
    char full_cmd[4352];
    if (verbose == NULL || verbose[0] == '\0') {
#ifdef _WIN32
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>NUL", cmd);
#else
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>/dev/null", cmd);
#endif
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
    }

    int rc = system(full_cmd);
    remove(body_path);

    int result = -1;
    if (rc == 0) {
        FILE *rf = fopen(resp_path, "rb");
        if (rf != NULL) {
            fseek(rf, 0, SEEK_END);
            long len = ftell(rf);
            fseek(rf, 0, SEEK_SET);
            if (len > 0 && len < (1 << 20)) {
                char *resp = (char *)malloc((size_t)len + 1);
                if (resp != NULL) {
                    size_t got = fread(resp, 1, (size_t)len, rf);
                    resp[got] = '\0';
                    if (extract_text(resp, out_buffer, out_size) == 0) {
                        result = 0;
                    }
                    free(resp);
                }
            }
            fclose(rf);
        }
    }
    remove(resp_path);
    return result;
}

int llm_pick_move_index(const char *prompt, int move_count, int *out_index)
{
    if (out_index == NULL || move_count <= 0 || move_count > 9) return -1;

    char response[256];
    if (llm_generate(prompt, response, sizeof(response)) != 0) return -1;

    /* 응답에서 첫 자릿수 한 글자만 본다. */
    for (size_t i = 0; response[i] != '\0'; i++) {
        char c = response[i];
        if (c >= '1' && c <= '9') {
            int n = c - '0';
            if (n >= 1 && n <= move_count) {
                *out_index = n - 1;
                return 0;
            }
            return -1;
        }
    }
    return -1;
}
