/*
 * LLM 호출 모듈 — 시스템에 깔린 curl 바이너리를 통해 OpenAI 호환
 * Chat Completions API 를 호출한다. 빌드 의존성은 없다.
 *
 * 큰 흐름:
 *   llm_generate(prompt)
 *     1. JSON 본문을 만들어 임시 파일에 쓴다
 *     2. system() 으로 curl 을 실행, 응답을 다른 임시 파일에 받는다
 *     3. 응답에서 "content" 텍스트만 뽑아 호출자에게 돌려준다
 *
 * 본문/응답을 파일로 주고받기 때문에 셸 이스케이프 걱정이 없다.
 */

#include "llm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <process.h>
#  define getpid           _getpid
#  define DEV_NULL         "NUL"
#  define TMPDIR_ENV       "TEMP"
#  define TMPDIR_FALLBACK  "."
#  define PATH_SEPARATOR   "\\"
#else
#  include <unistd.h>
#  define DEV_NULL         "/dev/null"
#  define TMPDIR_ENV       "TMPDIR"
#  define TMPDIR_FALLBACK  "/tmp"
#  define PATH_SEPARATOR   "/"
#endif

#define DEFAULT_BASE_URL      "https://api.openai.com/v1/chat/completions"
#define DEFAULT_MODEL         "gpt-4o-mini"
#define DEFAULT_TIMEOUT_SEC   30
#define MAX_TOKENS            256
#define RESPONSE_READ_BUFFER  8192

/* ============================================================
 * 공개 헬퍼
 * ============================================================ */

int llm_is_available(void)
{
    const char *api_key  = getenv("OPENAI_API_KEY");
    const char *base_url = getenv("LLM_BASE_URL");

    int has_api_key  = (api_key  != NULL && api_key[0]  != '\0');
    int has_base_url = (base_url != NULL && base_url[0] != '\0');

    return has_api_key || has_base_url;
}

/* ============================================================
 * 작은 유틸 함수들
 * ============================================================ */

/* 환경변수를 읽고 비어 있으면 기본값으로 대체. */
static const char *env_or_default(const char *name, const char *fallback)
{
    const char *value = getenv(name);
    return (value != NULL && value[0] != '\0') ? value : fallback;
}

/* 임시 디렉터리에 "llm_<pid>_<tag>" 형식 파일 경로를 만든다. */
static void build_temp_path(char *out, size_t out_size, const char *tag)
{
    const char *tmpdir = env_or_default(TMPDIR_ENV, TMPDIR_FALLBACK);
    snprintf(out, out_size, "%s%sllm_%d_%s",
             tmpdir, PATH_SEPARATOR, (int)getpid(), tag);
}

/* LLM_TIMEOUT_MS 를 읽어 초 단위 정수로 돌려준다. (curl --max-time 이 초 단위) */
static long read_timeout_seconds(void)
{
    const char *ms_str = getenv("LLM_TIMEOUT_MS");
    if (ms_str == NULL || ms_str[0] == '\0') return DEFAULT_TIMEOUT_SEC;

    long ms = strtol(ms_str, NULL, 10);
    if (ms <= 0) return DEFAULT_TIMEOUT_SEC;

    /* 올림 — 1500ms 면 2초로 둔다. */
    return (ms + 999) / 1000;
}

/* ============================================================
 * JSON 직렬화 / 파싱
 * ============================================================ */

/* prompt 의 ", \, 개행을 JSON 문자열 안에 들어갈 수 있게 이스케이프.
 * 우리 프롬프트엔 그 외 제어문자가 들어올 일이 없어서 처리 안 한다. */
static void escape_for_json(const char *src, char *dst, size_t dst_size)
{
    size_t out = 0;

    for (size_t i = 0; src[i] != '\0' && out + 2 < dst_size; i++) {
        char c = src[i];

        if (c == '"' || c == '\\') {
            dst[out++] = '\\';
            dst[out++] = c;
        } else if (c == '\n') {
            dst[out++] = '\\';
            dst[out++] = 'n';
        } else if (c == '\r') {
            dst[out++] = '\\';
            dst[out++] = 'r';
        } else {
            dst[out++] = c;
        }
    }
    dst[out] = '\0';
}

/* OpenAI 응답 JSON 에서 choices[0].message.content 값을 뽑는다.
 * 응답 예: { "choices": [ { "message": { "content": "..." } } ] }
 *
 * 정식 JSON 파서가 아니라 우리가 쓰는 포맷만 처리하는 단순 추출.
 * \uXXXX 같은 유니코드 이스케이프는 '?' 로 치환한다.
 */
static int extract_content_field(const char *json, char *out, size_t out_size)
{
    /* "content" 키 위치를 찾는다. */
    const char *cursor = strstr(json, "\"content\"");
    if (cursor == NULL) return -1;
    cursor += strlen("\"content\"");

    /* 콜론 + 공백을 건너뛰고 여는 큰따옴표까지 간다. */
    while (*cursor == ' ' || *cursor == '\t' || *cursor == ':' ||
           *cursor == '\n' || *cursor == '\r') {
        cursor++;
    }
    if (*cursor != '"') return -1;
    cursor++;

    /* 닫는 큰따옴표까지 내용을 복사하면서 이스케이프 디코딩. */
    size_t written = 0;
    while (*cursor != '\0' && *cursor != '"' && written + 1 < out_size) {
        if (*cursor == '\\' && cursor[1] != '\0') {
            switch (cursor[1]) {
                case 'n':  out[written++] = '\n'; break;
                case 't':  out[written++] = '\t'; break;
                case 'r':  out[written++] = '\r'; break;
                case 'u':  out[written++] = '?'; cursor += 4; break; /* \uXXXX 건너뜀 */
                default:   out[written++] = cursor[1]; break;       /* ", \, /, b, f */
            }
            cursor += 2;
        } else {
            out[written++] = *cursor++;
        }
    }
    out[written] = '\0';

    /* 닫는 큰따옴표를 못 만났으면 응답이 잘린 것 */
    return (*cursor == '"') ? 0 : -1;
}

/* ============================================================
 * 요청 본문 / curl 명령어 생성
 * ============================================================ */

/* JSON 본문을 만들어 임시 파일에 쓴다. 성공 시 0. */
static int write_request_body(const char *prompt, const char *model,
                              const char *request_path)
{
    /* 이스케이프 후 최대 2배 + 약간 여유 */
    size_t escaped_capacity = strlen(prompt) * 2 + 16;
    char *escaped_prompt = malloc(escaped_capacity);
    if (escaped_prompt == NULL) return -1;
    escape_for_json(prompt, escaped_prompt, escaped_capacity);

    size_t body_capacity = escaped_capacity + 256;
    char *body = malloc(body_capacity);
    if (body == NULL) {
        free(escaped_prompt);
        return -1;
    }
    snprintf(body, body_capacity,
        "{\"model\":\"%s\",\"max_tokens\":%d,"
        "\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
        model, MAX_TOKENS, escaped_prompt);
    free(escaped_prompt);

    FILE *file = fopen(request_path, "wb");
    if (file == NULL) {
        free(body);
        return -1;
    }
    fwrite(body, 1, strlen(body), file);
    fclose(file);
    free(body);
    return 0;
}

/* curl 명령어 문자열을 조립한다. api_key 가 있을 때만 Authorization 헤더 추가. */
static void build_curl_command(char *out, size_t out_size,
                               const char *api_key, const char *base_url,
                               const char *request_path, const char *response_path,
                               long timeout_sec)
{
    char auth_header[512] = "";
    if (api_key != NULL && api_key[0] != '\0') {
        snprintf(auth_header, sizeof(auth_header),
                 "-H \"Authorization: Bearer %s\" ", api_key);
    }

    snprintf(out, out_size,
        "curl -sS -X POST "
        "-H \"content-type: application/json\" "
        "%s"
        "--data-binary @\"%s\" "
        "--max-time %ld "
        "-o \"%s\" "
        "\"%s\" "
        "2>%s",
        auth_header, request_path, timeout_sec, response_path, base_url, DEV_NULL);
}

/* 응답 파일을 읽어 그 안에서 content 텍스트만 out 에 채운다. */
static int read_and_parse_response(const char *response_path,
                                   char *out, size_t out_size)
{
    FILE *file = fopen(response_path, "rb");
    if (file == NULL) return -1;

    char buffer[RESPONSE_READ_BUFFER];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytes_read] = '\0';
    fclose(file);

    return extract_content_field(buffer, out, out_size);
}

/* ============================================================
 * 공개 함수: llm_generate / llm_pick_move_index
 * ============================================================ */

int llm_generate(const char *prompt, char *out, size_t out_size)
{
    if (out == NULL || out_size == 0) return -1;
    out[0] = '\0';

    const char *api_key  = getenv("OPENAI_API_KEY");
    const char *base_url = env_or_default("LLM_BASE_URL", DEFAULT_BASE_URL);
    const char *model    = env_or_default("LLM_MODEL",    DEFAULT_MODEL);

    /* OpenAI 직결인데 키가 없으면 호출할 의미가 없다. */
    int is_openai_direct = (strstr(base_url, "api.openai.com") != NULL);
    int has_api_key      = (api_key != NULL && api_key[0] != '\0');
    if (is_openai_direct && !has_api_key) return -1;

    char request_path[512];
    char response_path[512];
    build_temp_path(request_path,  sizeof(request_path),  "req.json");
    build_temp_path(response_path, sizeof(response_path), "resp.json");

    if (write_request_body(prompt, model, request_path) != 0) return -1;

    char command[4096];
    build_curl_command(command, sizeof(command),
                       api_key, base_url, request_path, response_path,
                       read_timeout_seconds());

    int curl_rc = system(command);
    remove(request_path);

    int result = -1;
    if (curl_rc == 0) {
        result = read_and_parse_response(response_path, out, out_size);
    }
    remove(response_path);
    return result;
}

int llm_pick_move_index(const char *prompt, int move_count, int *out_index)
{
    if (out_index == NULL || move_count <= 0 || move_count > 9) return -1;

    char response[256];
    if (llm_generate(prompt, response, sizeof(response)) != 0) return -1;

    /* 응답 어딘가에 있는 첫 자릿수만 받는다. 'LLM 이 "정답은 2번" 처럼
     * 떠들어도 '2' 만 잡아낸다. */
    for (size_t i = 0; response[i] != '\0'; i++) {
        char c = response[i];
        if (c >= '1' && c <= '9') {
            int picked = c - '0';
            if (picked <= move_count) {
                *out_index = picked - 1;
                return 0;
            }
            return -1;
        }
    }
    return -1;
}
