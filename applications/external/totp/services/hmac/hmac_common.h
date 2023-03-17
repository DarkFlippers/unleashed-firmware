#include <string.h>
#include "sha256.h"
#include "memxor.h"

#define IPAD 0x36
#define OPAD 0x5c

/* Concatenate two preprocessor tokens.  */
#define _GLHMAC_CONCAT_(prefix, suffix) prefix##suffix
#define _GLHMAC_CONCAT(prefix, suffix) _GLHMAC_CONCAT_(prefix, suffix)

#if GL_HMAC_NAME == 5
#define HMAC_ALG md5
#else
#define HMAC_ALG _GLHMAC_CONCAT(sha, GL_HMAC_NAME)
#endif

#define GL_HMAC_CTX _GLHMAC_CONCAT(HMAC_ALG, _ctx)
#define GL_HMAC_FN _GLHMAC_CONCAT(hmac_, HMAC_ALG)
#define GL_HMAC_FN_INIT _GLHMAC_CONCAT(HMAC_ALG, _init_ctx)
#define GL_HMAC_FN_BLOC _GLHMAC_CONCAT(HMAC_ALG, _process_block)
#define GL_HMAC_FN_PROC _GLHMAC_CONCAT(HMAC_ALG, _process_bytes)
#define GL_HMAC_FN_FINI _GLHMAC_CONCAT(HMAC_ALG, _finish_ctx)

static void
    hmac_hash(const void* key, size_t keylen, const void* in, size_t inlen, int pad, void* resbuf) {
    struct GL_HMAC_CTX hmac_ctx;
    char block[GL_HMAC_BLOCKSIZE];

    memset(block, pad, sizeof block);
    memxor(block, key, keylen);

    GL_HMAC_FN_INIT(&hmac_ctx);
    GL_HMAC_FN_BLOC(block, sizeof block, &hmac_ctx);
    GL_HMAC_FN_PROC(in, inlen, &hmac_ctx);
    GL_HMAC_FN_FINI(&hmac_ctx, resbuf);
}

int GL_HMAC_FN(const void* key, size_t keylen, const void* in, size_t inlen, void* resbuf) {
    char optkeybuf[GL_HMAC_HASHSIZE];
    char innerhash[GL_HMAC_HASHSIZE];

    /* Ensure key size is <= block size.  */
    if(keylen > GL_HMAC_BLOCKSIZE) {
        struct GL_HMAC_CTX keyhash;

        GL_HMAC_FN_INIT(&keyhash);
        GL_HMAC_FN_PROC(key, keylen, &keyhash);
        GL_HMAC_FN_FINI(&keyhash, optkeybuf);

        key = optkeybuf;
        /* zero padding of the key to the block size
         is implicit in the memxor.  */
        keylen = sizeof optkeybuf;
    }

    /* Compute INNERHASH from KEY and IN.  */
    hmac_hash(key, keylen, in, inlen, IPAD, innerhash);

    /* Compute result from KEY and INNERHASH.  */
    hmac_hash(key, keylen, innerhash, sizeof innerhash, OPAD, resbuf);

    return 0;
}
