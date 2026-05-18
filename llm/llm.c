/* getaddrinfo / struct addrinfo 등 POSIX 소켓 API 를 -std=c99 에서 노출시키기 위함. */
#define _POSIX_C_SOURCE 200112L

#include "llm.h"

/*
 * 모든 LLM 호출이 거치는 배틀 중계 system 프롬프트.
 * 직결/프록시 모드와 무관하게 llm_battle_line 이 이 지시문을 붙입니다.
 */
#define LLM_BATTLE_SYSTEM \
    "너는 1세대 포켓몬 배틀의 중계 아나운서다. " \
    "주어진 배틀 상황을 한국어 한 문장으로 생동감 있고 간결하게 중계하라. " \
    "포켓몬 이름, 기술 이름, 효과는 사실 그대로 두고 없는 사실을 지어내지 마라. " \
    "출력은 따옴표 없이 한 문장, 60자 이내."

#ifdef LLM_DISABLED

/* 네트워크 코드 없이도 빌드되도록 한 스텁 구현. 게임은 폴백 메시지로 동작합니다. */

int  llm_init(void)         { return 0; }
void llm_cleanup(void)      { }
int  llm_is_available(void) { return 0; }

int llm_generate(const char *prompt, char *out_buffer, size_t out_size)
{
    (void)prompt;
    if (out_buffer != NULL && out_size > 0) out_buffer[0] = '\0';
    return -1;
}

int llm_battle_line(const char *situation, char *out_buffer, size_t out_size)
{
    (void)situation;
    if (out_buffer != NULL && out_size > 0) out_buffer[0] = '\0';
    return -1;
}

#else

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <netdb.h>          /* getaddrinfo */
#include <sys/socket.h>     /* socket, connect, setsockopt */
#include <sys/time.h>       /* struct timeval */
#include <unistd.h>         /* close */

/* 외부 공개 도메인 암호 코드 (llm/sha256.*, llm/hmac_sha256.*). */
#include "sha256.h"
#include "hmac_sha256.h"

/* 기본값: 같은 머신에서 도는 app/ 프록시. LLM_BASE_URL 로 바꿀 수 있습니다.
 * 이 클라이언트는 평문 HTTP 만 지원하므로 URL 은 http:// 여야 합니다.
 * (외부 인터넷 노출이 필요하면 nginx/Caddy 가 TLS 를 종단하도록 앞단에 둡니다.) */
#define LLM_DEFAULT_URL       "http://127.0.0.1:8080/v1/chat/completions"
#define LLM_DEFAULT_MODEL     "gpt-4o-mini"
#define LLM_MAX_TOKENS        256
#define LLM_PROTOCOL_VERSION  "1"
#define LLM_DEFAULT_CLIENT_ID "pokemon-c-client/1.0"
#define LLM_SOCKET_TIMEOUT_S  30

/* ===========================================================================
 *  동적 바이트 버퍼 — HTTP 응답을 누적하는 데 씁니다.
 * ========================================================================= */
typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} ByteBuffer;

/* buf 에 add 바이트를 이어붙입니다. 성공 0, 메모리 실패 -1. 항상 NUL 종료. */
static int buf_append(ByteBuffer *buf, const char *src, size_t add)
{
    if (buf->len + add + 1 > buf->cap) {
        size_t new_cap = buf->cap ? buf->cap * 2 : 4096;
        while (new_cap < buf->len + add + 1) {
            new_cap *= 2;
        }
        char *grown = (char *)realloc(buf->data, new_cap);
        if (grown == NULL) return -1;
        buf->data = grown;
        buf->cap  = new_cap;
    }
    memcpy(buf->data + buf->len, src, add);
    buf->len += add;
    buf->data[buf->len] = '\0';
    return 0;
}

/* ===========================================================================
 *  암호 헬퍼 — 위에서 include 한 공개 도메인 코드를 hex 문자열로 감쌉니다.
 * ========================================================================= */

/* raw 바이트열을 소문자 hex 문자열로. out 은 len*2+1 바이트 이상. */
static void to_hex(const uint8_t *raw, size_t len, char *out)
{
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[i * 2]     = digits[(raw[i] >> 4) & 0x0f];
        out[i * 2 + 1] = digits[raw[i] & 0x0f];
    }
    out[len * 2] = '\0';
}

/* data 의 SHA-256 을 소문자 hex 64자로. out 은 65바이트. */
static void hex_sha256(const void *data, size_t len, char out[65])
{
    SHA256_HASH digest;
    Sha256Calculate(data, (uint32_t)len, &digest);
    to_hex(digest.bytes, SHA256_HASH_SIZE, out);
}

/* HMAC-SHA256(key, msg) 를 소문자 hex 64자로. out 은 65바이트. */
static void hex_hmac_sha256(const void *key, size_t key_len,
                            const void *msg, size_t msg_len, char out[65])
{
    uint8_t mac[SHA256_HASH_SIZE];
    hmac_sha256(key, key_len, msg, msg_len, mac, sizeof(mac));
    to_hex(mac, SHA256_HASH_SIZE, out);
}

/* ===========================================================================
 *  JSON 직렬화 / 응답 파싱
 * ========================================================================= */

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
 * 응답 JSON 에서 choices[0].message.content 값을 뽑아냅니다.
 * 정식 파서가 아니라, OpenAI Chat Completions 응답 포맷에 한정한 단순 추출입니다.
 * 응답 예시:
 *   { ..., "choices":[{"index":0,"message":{"role":"assistant","content":"...실제 텍스트..."}, ...}], ... }
 * 성공 시 0, 실패 시 -1.
 */
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

/*
 * model / (옵션) system / user 로 OpenAI Chat Completions 요청 본문 JSON 을
 * 만듭니다. 통신 규격(docs/proxy-protocol.md §3) 의 허용 필드만 사용합니다.
 * 성공 시 malloc 된 버퍼, 실패 시 NULL. 호출 측이 free 합니다.
 */
static char *build_chat_body(const char *model, const char *system_prompt,
                             const char *user_prompt)
{
    size_t user_len = strlen(user_prompt);
    size_t sys_len  = system_prompt ? strlen(system_prompt) : 0;
    /* 이스케이프 후 최대 길이는 입력의 약 6배. */
    size_t esc_cap  = (user_len > sys_len ? user_len : sys_len) * 6 + 16;

    char *user_esc = (char *)malloc(esc_cap);
    char *sys_esc  = (char *)malloc(esc_cap);
    if (user_esc == NULL || sys_esc == NULL) {
        free(user_esc);
        free(sys_esc);
        return NULL;
    }
    json_escape(user_prompt, user_esc, esc_cap);
    if (system_prompt != NULL) {
        json_escape(system_prompt, sys_esc, esc_cap);
    }

    size_t body_cap = esc_cap * 2 + 256;
    char *body = (char *)malloc(body_cap);
    if (body == NULL) {
        free(user_esc);
        free(sys_esc);
        return NULL;
    }

    if (system_prompt != NULL) {
        snprintf(body, body_cap,
            "{\"model\":\"%s\",\"max_tokens\":%d,\"messages\":["
            "{\"role\":\"system\",\"content\":\"%s\"},"
            "{\"role\":\"user\",\"content\":\"%s\"}]}",
            model, LLM_MAX_TOKENS, sys_esc, user_esc);
    } else {
        snprintf(body, body_cap,
            "{\"model\":\"%s\",\"max_tokens\":%d,\"messages\":["
            "{\"role\":\"user\",\"content\":\"%s\"}]}",
            model, LLM_MAX_TOKENS, user_esc);
    }

    free(user_esc);
    free(sys_esc);
    return body;
}

/*
 * 16바이트 무작위 nonce 를 32자 hex 문자열로 채웁니다 (out 은 33바이트).
 * /dev/urandom 을 우선 쓰고, 없으면 시각/주소 기반 폴백을 씁니다. nonce 는
 * 리플레이 방지를 위한 유일값이면 충분하므로 폴백도 실용상 안전합니다.
 */
static void make_nonce(char out[33])
{
    uint8_t raw[16];
    int got = 0;
    size_t i;

    FILE *f = fopen("/dev/urandom", "rb");
    if (f != NULL) {
        if (fread(raw, 1, sizeof(raw), f) == sizeof(raw)) {
            got = 1;
        }
        fclose(f);
    }
    if (!got) {
        static unsigned long counter = 0;
        unsigned long seed = (unsigned long)time(NULL)
                           ^ ((unsigned long)clock() << 8)
                           ^ ((unsigned long)(uintptr_t)out << 4)
                           ^ (++counter);
        for (i = 0; i < sizeof(raw); i++) {
            seed = seed * 6364136223846793005UL + 1442695040888963407UL;
            raw[i] = (uint8_t)(seed >> 33);
        }
    }
    to_hex(raw, sizeof(raw), out);
}

/* ===========================================================================
 *  평문 HTTP 클라이언트 (POSIX 소켓)
 *
 *  libcurl 을 쓰지 않고 직접 구현합니다. 평문 HTTP 만 지원하며 HTTPS(TLS)는
 *  지원하지 않습니다 — 같은 머신/LAN 의 app/ 프록시를 호출하는 용도입니다.
 * ========================================================================= */

/*
 * "http://host[:port]/path" 형태의 URL 을 host / port / path 로 쪼갭니다.
 * port 가 없으면 "80", path 가 없으면 "/" 가 들어갑니다.
 * 성공 0, http:// 가 아니거나 형식이 틀리면 -1.
 */
static int parse_http_url(const char *url,
                          char *host, size_t host_size,
                          char *port, size_t port_size,
                          char *path, size_t path_size)
{
    const char *prefix = "http://";
    size_t plen = strlen(prefix);
    if (strncmp(url, prefix, plen) != 0) {
        return -1;  /* https:// 등은 이 클라이언트가 지원하지 않음. */
    }

    const char *p = url + plen;        /* host[:port]/path 의 시작 */
    const char *slash = strchr(p, '/');
    const char *colon = strchr(p, ':');
    const char *host_end;

    /* ':' 가 path 시작('/') 보다 앞에 있을 때만 포트 구분자로 본다. */
    if (colon != NULL && (slash == NULL || colon < slash)) {
        host_end = colon;
        const char *port_start = colon + 1;
        const char *port_end = slash ? slash : (port_start + strlen(port_start));
        size_t plen2 = (size_t)(port_end - port_start);
        if (plen2 == 0 || plen2 >= port_size) return -1;
        memcpy(port, port_start, plen2);
        port[plen2] = '\0';
    } else {
        host_end = slash ? slash : (p + strlen(p));
        snprintf(port, port_size, "80");
    }

    size_t hlen = (size_t)(host_end - p);
    if (hlen == 0 || hlen >= host_size) return -1;
    memcpy(host, p, hlen);
    host[hlen] = '\0';

    if (slash == NULL) {
        snprintf(path, path_size, "/");
    } else {
        if (strlen(slash) >= path_size) return -1;
        snprintf(path, path_size, "%s", slash);
    }
    return 0;
}

/* fd 로 len 바이트를 끝까지 보냅니다. 성공 0, 실패 -1. */
static int send_all(int fd, const char *data, size_t len)
{
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

/*
 * url 로 HTTP POST 를 보내고 응답 본문을 받아옵니다.
 *  - headers: "Name: Value" 문자열 배열 (Host/Content-Length/Connection 은 자동 부착).
 *  - 성공 시 HTTP 상태코드(>=100), 전송 단계 실패 시 -1 을 반환.
 *  - 성공 시 *out_body 에 응답 본문을 malloc 해서 채웁니다. 호출 측이 free.
 */
static int http_post(const char *url,
                     const char *const *headers, int nheaders,
                     const char *body,
                     char **out_body)
{
    char host[256], port[16], path[1024];
    if (parse_http_url(url, host, sizeof(host), port, sizeof(port),
                       path, sizeof(path)) != 0) {
        return -1;
    }

    /* 1) 호스트 이름 해석. */
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;     /* IPv4/IPv6 무관 */
    hints.ai_socktype = SOCK_STREAM;   /* TCP */
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return -1;
    }

    /* 2) 소켓 생성 + 연결. */
    int fd = -1;
    for (struct addrinfo *ai = res; ai != NULL; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) return -1;

    /* 송수신 타임아웃 — 서버가 멈춰도 게임이 영원히 대기하지 않도록. */
    struct timeval tv;
    tv.tv_sec  = LLM_SOCKET_TIMEOUT_S;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    /* 3) 요청 메시지 조립. */
    ByteBuffer req = { NULL, 0, 0 };
    char line[1280];
    int ok = 0;

    snprintf(line, sizeof(line), "POST %s HTTP/1.1\r\n", path);
    ok |= buf_append(&req, line, strlen(line));
    snprintf(line, sizeof(line), "Host: %s\r\n", host);
    ok |= buf_append(&req, line, strlen(line));
    for (int i = 0; i < nheaders; i++) {
        ok |= buf_append(&req, headers[i], strlen(headers[i]));
        ok |= buf_append(&req, "\r\n", 2);
    }
    snprintf(line, sizeof(line), "Content-Length: %zu\r\n", strlen(body));
    ok |= buf_append(&req, line, strlen(line));
    /* Connection: close — 서버가 응답 후 연결을 닫으므로 EOF 까지 읽으면 끝. */
    ok |= buf_append(&req, "Connection: close\r\n\r\n", 21);
    ok |= buf_append(&req, body, strlen(body));

    if (ok != 0 || send_all(fd, req.data, req.len) != 0) {
        free(req.data);
        close(fd);
        return -1;
    }
    free(req.data);

    /* 4) 응답 수신 — 연결이 닫힐 때까지. */
    ByteBuffer resp = { NULL, 0, 0 };
    char chunk[4096];
    for (;;) {
        ssize_t n = recv(fd, chunk, sizeof(chunk), 0);
        if (n > 0) {
            if (buf_append(&resp, chunk, (size_t)n) != 0) {
                free(resp.data);
                close(fd);
                return -1;
            }
        } else {
            break;  /* 0 = 정상 종료, <0 = 오류/타임아웃 */
        }
    }
    close(fd);

    if (resp.data == NULL) {
        return -1;
    }

    /* 5) 상태줄 파싱 + 헤더/본문 분리. */
    int status = -1;
    const char *sp = strchr(resp.data, ' ');
    if (sp != NULL) {
        status = atoi(sp + 1);
    }
    const char *body_start = strstr(resp.data, "\r\n\r\n");
    if (body_start != NULL) {
        body_start += 4;
        size_t body_len = resp.len - (size_t)(body_start - resp.data);
        char *copy = (char *)malloc(body_len + 1);
        if (copy != NULL) {
            memcpy(copy, body_start, body_len);
            copy[body_len] = '\0';
            *out_body = copy;
        }
    }

    free(resp.data);
    return status;
}

/* ===========================================================================
 *  공개 API
 * ========================================================================= */

int llm_init(void)
{
    /* POSIX 소켓은 전역 초기화가 필요 없습니다. API 호환을 위해 남겨둡니다. */
    return 0;
}

void llm_cleanup(void)
{
    /* 정리할 전역 상태가 없습니다. */
}

int llm_is_available(void)
{
    /* 프록시 비밀, LLM_BASE_URL, 업스트림 키 중 하나라도 있으면 시도 가능으로 본다. */
    const char *secret = getenv("POKEMON_CLIENT_SECRET");
    const char *url    = getenv("LLM_BASE_URL");
    const char *key    = getenv("OPENAI_API_KEY");
    if (secret != NULL && secret[0] != '\0') return 1;
    if (url    != NULL && url[0]    != '\0') return 1;
    if (key    != NULL && key[0]    != '\0') return 1;
    return 0;
}

/*
 * 공통 호출 경로. system_prompt 가 NULL 이면 user 메시지만 보냅니다.
 * POKEMON_CLIENT_SECRET 이 설정돼 있으면 프록시 모드로 HMAC 서명 헤더를 붙입니다.
 */
static int llm_chat(const char *system_prompt, const char *user_prompt,
                    char *out_buffer, size_t out_size)
{
    if (out_buffer == NULL || out_size == 0 || user_prompt == NULL) return -1;
    out_buffer[0] = '\0';

    const char *api_key = getenv("OPENAI_API_KEY");
    const char *url     = getenv("LLM_BASE_URL");
    if (url == NULL || url[0] == '\0') url = LLM_DEFAULT_URL;

    const char *secret = getenv("POKEMON_CLIENT_SECRET");
    int proxy_mode = (secret != NULL && secret[0] != '\0');

    const char *model = getenv("LLM_MODEL");
    if (model == NULL || model[0] == '\0') model = LLM_DEFAULT_MODEL;

    char *body = build_chat_body(model, system_prompt, user_prompt);
    if (body == NULL) return -1;

    /* 요청 헤더 목록 구성. 동적 버퍼는 http_post 호출이 끝날 때까지 살아 있어야
     * 하므로 이 함수 스택에 둔다. */
    const char *hdrs[8];
    int nh = 0;
    hdrs[nh++] = "Content-Type: application/json";

    char h_proto[64], h_cid[128], h_ts[64], h_nonce[96], h_sig[160], h_ua[128];
    char h_auth[600];

    if (proxy_mode) {
        /* 통신 규격(docs/proxy-protocol.md §4) 의 HMAC 서명 헤더를 붙인다. */
        const char *client_id = getenv("POKEMON_CLIENT_ID");
        if (client_id == NULL || client_id[0] == '\0') client_id = LLM_DEFAULT_CLIENT_ID;

        char ts[32], nonce[33], body_hash[65], canonical[160], sig[65];
        snprintf(ts, sizeof(ts), "%ld", (long)time(NULL));
        make_nonce(nonce);
        hex_sha256(body, strlen(body), body_hash);
        snprintf(canonical, sizeof(canonical), "%s\n%s\n%s", ts, nonce, body_hash);
        hex_hmac_sha256(secret, strlen(secret),
                        canonical, strlen(canonical), sig);

        snprintf(h_proto, sizeof(h_proto), "X-Pokemon-Protocol: %s", LLM_PROTOCOL_VERSION);
        snprintf(h_cid,   sizeof(h_cid),   "X-Pokemon-Client-Id: %s", client_id);
        snprintf(h_ts,    sizeof(h_ts),    "X-Pokemon-Timestamp: %s", ts);
        snprintf(h_nonce, sizeof(h_nonce), "X-Pokemon-Nonce: %s", nonce);
        snprintf(h_sig,   sizeof(h_sig),   "X-Pokemon-Signature: %s", sig);
        snprintf(h_ua,    sizeof(h_ua),    "User-Agent: %s", client_id);

        hdrs[nh++] = h_proto;
        hdrs[nh++] = h_cid;
        hdrs[nh++] = h_ts;
        hdrs[nh++] = h_nonce;
        hdrs[nh++] = h_sig;
        hdrs[nh++] = h_ua;
    } else if (api_key != NULL && api_key[0] != '\0') {
        /* 직결 모드: 업스트림 키를 직접 Authorization 으로 보낸다. */
        snprintf(h_auth, sizeof(h_auth), "Authorization: Bearer %s", api_key);
        hdrs[nh++] = h_auth;
    }

    char *resp_body = NULL;
    int status = http_post(url, hdrs, nh, body, &resp_body);
    free(body);

    int result = -1;
    if (status >= 200 && status < 300 && resp_body != NULL) {
        if (extract_text(resp_body, out_buffer, out_size) == 0) {
            result = 0;
        }
    }
    free(resp_body);
    return result;
}

int llm_generate(const char *prompt, char *out_buffer, size_t out_size)
{
    if (prompt == NULL) {
        if (out_buffer != NULL && out_size > 0) out_buffer[0] = '\0';
        return -1;
    }
    return llm_chat(NULL, prompt, out_buffer, out_size);
}

int llm_battle_line(const char *situation, char *out_buffer, size_t out_size)
{
    if (situation == NULL) {
        if (out_buffer != NULL && out_size > 0) out_buffer[0] = '\0';
        return -1;
    }
    return llm_chat(LLM_BATTLE_SYSTEM, situation, out_buffer, out_size);
}

#endif /* LLM_DISABLED */
