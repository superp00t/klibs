#include "crypt_easy.h"
#include "../debug.h"

crypt_easy_kp *crypt_easy_kp_alloc()
{
  crypt_easy_kp *kp = (crypt_easy_kp*)malloc(sizeof(crypt_easy_kp));
  memset(kp, 0, sizeof(crypt_easy_kp));

  randombytes(kp->secret, 32);
  crypto_scalarmult_base(kp->public, kp->secret);

  return kp;
}

void crypt_easy_kp_destroy(crypt_easy_kp *kp)
{
  free(kp);
}

byte_buffer_t *crypt_easy_seal(crypt_easy_kp* kp, uint8_t *peerkey, byte_buffer_t *msg)
{
  byte_buffer_t *bb = byte_buffer_alloc();
  // Put nonce data
  byte_buffer_put_rand(bb, crypto_box_NONCEBYTES);

  // Setup weird padding required by TweetNaCl
  uint32_t psize = crypto_box_BOXZEROBYTES + msg->wpos;
  uint8_t *padded = (uint8_t*)malloc(psize);
  uint8_t *encout = (uint8_t*)malloc(psize);
  
  memset(padded, 0, crypto_box_BOXZEROBYTES);
  memcpy(padded  +  crypto_box_BOXZEROBYTES, msg->buf, msg->wpos);

  /*                Read directly from buffer, where we put
                    random nonce data already

                    |
                    v                */
  crypto_box(encout, padded, psize, bb->buf, peerkey, kp->secret);
  free(padded);

  byte_buffer_put_bytes(bb, encout+crypto_box_BOXZEROBYTES, msg->wpos);
  free(encout);

  return bb;
}

// Returns status. sets out to new buffer of decrypted data, if successful.
byte_buffer_t *crypt_easy_open(byte_buffer_t *encin, crypt_easy_kp* kp, uint8_t *peerkey)
{
  byte_buffer_t *out = byte_buffer_alloc();
  int psize = (encin->wpos - crypto_box_NONCEBYTES) + crypto_box_BOXZEROBYTES;
  uint8_t *in = (uint8_t *)malloc(psize);
  uint8_t *outb = (uint8_t *)malloc(psize);
  
  memset(in, 0, psize);
  memset(outb, 0, psize);

  /* Copy into decryption buffer  the sector of message following the nonce
       |              |
       v              v                      */
  memcpy(in + crypto_box_BOXZEROBYTES, encin->buf + crypto_box_NONCEBYTES, encin->wpos - crypto_box_NONCEBYTES);

  KDEBUG_BYTES("Nonce", encin->buf, 24);
  KDEBUG_BYTES("CK", kp->public, 32);
  
  int status = crypto_box_open(outb, in, psize, encin->buf, peerkey, kp->secret);
  if(status < 0)
  {
    KDEBUGF("Bad status.\n");
    free(in);
    free(outb);
    byte_buffer_destroy(out);
    return NULL;
  }

  KDEBUG_BYTES("In", in, psize);
  byte_buffer_put_bytes(out, outb+crypto_box_BOXZEROBYTES, psize-crypto_box_BOXZEROBYTES);

  free(in);
  free(outb);

  return out;
}

// Returns SHA-512 hash of byte buffer
uint8_t *digest_buf(byte_buffer_t *b)
{
  uint8_t *out = (uint8_t*)malloc(64);
  crypto_hash(out, b->buf, b->wpos);
  return out;
}

