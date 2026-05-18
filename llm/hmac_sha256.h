/*
 * ===========================================================================
 *  외부 공개 도메인 코드 — 직접 작성하지 않고 외부 저장소에서 그대로 가져왔습니다.
 *
 *  출처 저장소 : h5p9sl/hmac_sha256
 *  파일 URL    : https://github.com/h5p9sl/hmac_sha256/blob/9445307/hmac_sha256.h
 *  고정 커밋   : 9445307885b86fb997b10f49ada5bee47496950a
 *  라이선스    : The Unlicense (퍼블릭 도메인) — https://unlicense.org
 *
 *  아래 본문은 위 저장소의 원본 그대로이며, 이 머리말 주석 블록만 추가했습니다.
 *  (퍼블릭 도메인이므로 그대로 복제·사용이 허용됩니다.)
 * ===========================================================================
 */

/*
    hmac_sha256.h
    Originally written by https://github.com/h5p9sl
*/

#ifndef _HMAC_SHA256_H_
#define _HMAC_SHA256_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stddef.h>

size_t  // Returns the number of bytes written to `out`
hmac_sha256(
    // [in]: The key and its length.
    //      Should be at least 32 bytes long for optimal security.
    const void* key,
    const size_t keylen,

    // [in]: The data to hash alongside the key.
    const void* data,
    const size_t datalen,

    // [out]: The output hash.
    //      Should be 32 bytes long. If it's less than 32 bytes,
    //      the resulting hash will be truncated to the specified length.
    void* out,
    const size_t outlen);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _HMAC_SHA256_H_
