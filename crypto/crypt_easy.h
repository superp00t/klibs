#include "../byte_buffer.h"
#include "rand.h"
#include "tweetnacl.h"

#define EASY_CRYPT_OK   0
#define EASY_CRYPT_FAIL -1

typedef struct {
  uint8_t public[32];
  uint8_t secret[32];
} crypt_easy_kp;

byte_buffer_t *crypt_easy_seal(crypt_easy_kp* kp, uint8_t *peerkey, byte_buffer_t *msg);
byte_buffer_t *crypt_easy_open(byte_buffer_t *encin, crypt_easy_kp* kp, uint8_t *peerkey);
crypt_easy_kp *crypt_easy_kp_alloc();
void crypt_easy_kp_destroy(crypt_easy_kp *kp);
uint8_t *digest_buf(byte_buffer_t *b);