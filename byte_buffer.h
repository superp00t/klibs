#ifndef BYTE_BUFFER_H__
#define BYTE_BUFFER_H__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "crypto/rand.h"
/* Fragment size.
   
   Buffer uint8 array size grows by FRAG whenever it overflows.
*/
#define FRAG 512

typedef struct {
  uint8_t  *buf;

  // Size of the buffer.
  uint32_t bufsize;

  // Write position.
  uint32_t wpos;

  // Read position.
  uint32_t rpos;
} byte_buffer_t;

byte_buffer_t *byte_buffer_alloc();
void byte_buffer_put_byte(byte_buffer_t *bb, uint8_t ch);
void byte_buffer_put_bytes(byte_buffer_t *bb, uint8_t *bs, int ln);
void byte_buffer_destroy(byte_buffer_t *bb);
void byte_buffer_put_uint64(byte_buffer_t *bb, uint64_t v);
void byte_buffer_put_uint32(byte_buffer_t *bb, uint32_t v);
void byte_buffer_put_uint16(byte_buffer_t *bb, uint16_t v);
void byte_buffer_put_rand(byte_buffer_t *bb, int ln);
void byte_buffer_put_string(byte_buffer_t *bb, char *str);
uint8_t byte_buffer_get_byte(byte_buffer_t *bb);
uint8_t *byte_buffer_get_bytes(byte_buffer_t *bb, int ln);
uint64_t byte_buffer_get_uint64(byte_buffer_t *bb);
uint32_t byte_buffer_get_uint32(byte_buffer_t *bb);
uint16_t byte_buffer_get_uint16(byte_buffer_t *bb);
char *byte_buffer_get_string(byte_buffer_t *bb, char delimiter);
char *byte_buffer_get_dstring(byte_buffer_t *bb, char delimiter);
uint32_t secure_rand(uint32_t min, uint32_t max);
#endif