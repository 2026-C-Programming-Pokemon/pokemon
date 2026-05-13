#include "llm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define LLM_API_URL          "https://api.anthropic.com/v1/messages"
#define LLM_API_VERSION      "2023-06-01"
#define LLM_DEFAULT_MODEL    "claude-haiku-4-5"
#define LLM_MAX_TOKENS       256
#define LLM_RESPONSE_BUFFER  (16 * 1024)

/* 호출 측이 미리 llm_init 을 호출했는지 추적합니다. */
static int g_initialized = 0;

/* libcurl 가 응답을 흘려 넣을 동적 버퍼. */
typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} ResponseBuffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t add = size * nmemb;
    ResponseBuffer *buf = (ResponseBuffer *)userp;

    if (buf->len + add + 1 > buf->cap) {
        size_t new_cap = buf->cap ? buf->cap * 2 : 4096;
        while (new_cap < buf->len + add + 1) {
            new_cap *= 2;
        }
        char *grown = (char *)realloc(buf->data, new_cap);
        if (grown == NULL) {
            return 0;
        }
        buf->data = grown;
        buf->cap  = new_cap;
    }

    memcpy(buf->data + buf->len, contents, add);
    buf->len += add;
    buf->data[buf->len] = '\0';
    return add;
}

/*
 * JSON 문자열에 들어갈 수 있도록 사용자 입력을 이스케이프합니다.
 * 매우 단순한 처리만 합니다: " \ \n \r \t 와 제어문자.
 * out 에 충분한 공간이 있다고 가정합니다 (대략 입력 길이의 6배).
 */
static void json_escape(const char *in, char *out, size_t out_size)
{
    size_t o = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        unsigned char c = (unsigned char)in[i];
        const char *seq = NULL;
        char  unicode_buf[8];
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

/*
 * 응답 JSON 에서 첫 번째 content[0].text 값을 뽑아냅니다.
 * 정식 파서가 아니라, Anthropic Messages 응답 포맷에 한정한 단순 추출입니다.
 * 응답 예시:
 *   { ..., "content":[{"type":"text","text":"...실제 텍스트..."}], ... }
 * 성공 시 0, 실패 시 -1.
 */
static int extract_text(const char *json, char *out, size_t out_size)
{
    const char *p = strstr(json, "\"text\"");
    if (p == NULL) return -1;

    p = strchr(p + 6, ':');
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
                    /* 단순화를 위해 \uXXXX 는 그대로 4글자를 건너뛰고 ? 로 대체합니다. */
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

int llm_init(void)
{
    if (g_initialized) return 0;
    CURLcode rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (rc != CURLE_OK) return -1;
    g_initialized = 1;
    return 0;
}

void llm_cleanup(void)
{
    if (!g_initialized) return;
    curl_global_cleanup();
    g_initialized = 0;
}

int llm_is_available(void)
{
    const char *key = getenv("ANTHROPIC_API_KEY");
    return (key != NULL && key[0] != '\0') ? 1 : 0;
}

int llm_generate(const char *prompt, char *out_buffer, size_t out_size)
{
    if (out_buffer == NULL || out_size == 0) return -1;
    out_buffer[0] = '\0';

    if (!g_initialized) {
        if (llm_init() != 0) return -1;
    }

    const char *api_key = getenv("ANTHROPIC_API_KEY");
    if (api_key == NULL || api_key[0] == '\0') return -1;

    const char *model = getenv("LLM_MODEL");
    if (model == NULL || model[0] == '\0') model = LLM_DEFAULT_MODEL;

    /* prompt 를 JSON 으로 직렬화. 입력 길이의 약 6배까지 안전. */
    size_t prompt_len = strlen(prompt);
    size_t esc_size = prompt_len * 6 + 16;
    char *escaped = (char *)malloc(esc_size);
    if (escaped == NULL) return -1;
    json_escape(prompt, escaped, esc_size);

    size_t body_cap = esc_size + 512;
    char *body = (char *)malloc(body_cap);
    if (body == NULL) {
        free(escaped);
        return -1;
    }
    snprintf(body, body_cap,
        "{\"model\":\"%s\",\"max_tokens\":%d,"
        "\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
        model, LLM_MAX_TOKENS, escaped);
    free(escaped);

    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        free(body);
        return -1;
    }

    ResponseBuffer resp = { NULL, 0, 0 };

    struct curl_slist *headers = NULL;
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "x-api-key: %s", api_key);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "anthropic-version: " LLM_API_VERSION);
    headers = curl_slist_append(headers, "content-type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, LLM_API_URL);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    CURLcode rc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(body);

    int result = -1;
    if (rc == CURLE_OK && http_code >= 200 && http_code < 300 && resp.data != NULL) {
        if (extract_text(resp.data, out_buffer, out_size) == 0) {
            result = 0;
        }
    }

    free(resp.data);
    return result;
}
