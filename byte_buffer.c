#include "byte_buffer.h"
#include "debug.h"

/*  Allocate new byte buffer */
byte_buffer_t *byte_buffer_alloc()
{
  byte_buffer_t *bb = (byte_buffer_t *)malloc(sizeof(byte_buffer_t));
  bb->buf = (uint8_t *)malloc(FRAG);
  memset(bb->buf, 0, FRAG);
  bb->bufsize = FRAG;
  bb->wpos = 0;
  bb->rpos = 0;

  return bb;
}

/* Appends an 8 bit unsigned integer (byte) to the byte buffer,
   expanding it by FRAG-long fragments as needed
*/
void byte_buffer_put_byte(byte_buffer_t *bb, uint8_t ch)
{
  bb->buf[bb->wpos] = ch;
  bb->wpos++;

  // Grow buffer
  if((bb->wpos+1) > bb->bufsize)
  {
    uint8_t  *tmp = bb->buf;
    uint32_t bz = bb->bufsize + FRAG;
    bb->buf = (uint8_t *)malloc(bz);
    memset(bb->buf, 0, bz);
    memcpy(bb->buf, tmp, bb->bufsize);
    free(tmp);
    bb->bufsize = bz;
  }
}

/* Appends random data to the byte buffer */
void byte_buffer_put_rand(byte_buffer_t *bb, int ln)
{
  uint8_t *bff = (uint8_t*)malloc(ln);
  randombytes(bff, ln);
  byte_buffer_put_bytes(bb, bff, ln);
  free(bff);
}

/* Appends a byte array to the byte buffer */
void byte_buffer_put_bytes(byte_buffer_t *bb, uint8_t *bs, int ln)
{
  for(int i = 0; i < ln; i++)
  {
    byte_buffer_put_byte(bb, bs[i]);
  }
}

/* Appends a string to the byte buffer */
void byte_buffer_put_string(byte_buffer_t *bb, char *string)
{
  byte_buffer_put_bytes(bb, (uint8_t*)string, strlen(string));
}

/* Destroys byte buffer memory allocations, making it unusable */
void byte_buffer_destroy(byte_buffer_t *bb)
{
  memset(bb->buf, 0, bb->bufsize);
  free(bb->buf);
  memset(bb, 0, sizeof(byte_buffer_t));
  free(bb);
}

/* INTEGER FUNCTIONS
   ===================================================

   Appends a little-endian 64 bit unsigned integer to the byte buffer */
void byte_buffer_put_uint64(byte_buffer_t *bb, uint64_t v)
{
  byte_buffer_put_byte(bb, (uint8_t)(v));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 8));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 16));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 24));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 32));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 40));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 48));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 56));
}

// Appends a little-endian 32 bit unsigned integer to the byte buffer
void byte_buffer_put_uint32(byte_buffer_t *bb, uint32_t v)
{
  byte_buffer_put_byte(bb, (uint8_t)(v));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 8));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 16));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 24));
}

// Appends a little-endian 16-bit unsigned integer to the byte buffer
void byte_buffer_put_uint16(byte_buffer_t *bb, uint16_t v)
{
  byte_buffer_put_byte(bb, (uint8_t)(v));
  byte_buffer_put_byte(bb, (uint8_t)(v >> 8));
}

// Returns a little-endian 8-bit unsigned integer
uint8_t byte_buffer_get_byte(byte_buffer_t *bb)
{
  if((bb->rpos+1) > bb->bufsize)
  {
    return 0;
  }

  uint8_t by = bb->buf[bb->rpos];
  bb->rpos++;
  return by;
}

// Returns a 64-bit unsigned integer
uint64_t byte_buffer_get_uint64(byte_buffer_t *bb)
{
  return 
  (uint64_t)(byte_buffer_get_byte(bb)) |
  (uint64_t)(byte_buffer_get_byte(bb)) << 8 |
  (uint64_t)(byte_buffer_get_byte(bb)) << 16 |
  (uint64_t)(byte_buffer_get_byte(bb)) << 24 |
  (uint64_t)(byte_buffer_get_byte(bb)) << 32 |
  (uint64_t)(byte_buffer_get_byte(bb)) << 40 |
  (uint64_t)(byte_buffer_get_byte(bb)) << 48 |
  (uint64_t)(byte_buffer_get_byte(bb)) << 56;
}

// Returns a 32-bit unsigned integer
uint32_t byte_buffer_get_uint32(byte_buffer_t *bb)
{
  return
  (uint32_t)(byte_buffer_get_byte(bb)) |
  (uint32_t)(byte_buffer_get_byte(bb)) << 8 |
  (uint32_t)(byte_buffer_get_byte(bb)) << 16 |
  (uint32_t)(byte_buffer_get_byte(bb)) << 24;
}

// Returns a 16-bit unsigned integer
uint16_t byte_buffer_get_uint16(byte_buffer_t *bb)
{
  return
  (uint16_t)(byte_buffer_get_byte(bb)) |
  (uint16_t)(byte_buffer_get_byte(bb)) << 8;
}

uint8_t *byte_buffer_get_bytes(byte_buffer_t *bb, int ln)
{
  uint8_t *b = (uint8_t*)malloc(ln);

  for(int i = 0; i < ln; i++)
  {
    b[i] = byte_buffer_get_byte(bb);
  }

  return b;
}

char *byte_buffer_get_string(byte_buffer_t *bb, char delimiter)
{
  // Find location of delimiter, and calculate string size
  int d_off = -1;
  for(int i = bb->rpos; ;i++)
  {
    if(i == bb->wpos)
    {
      return NULL;
    }

    if(bb->buf[i] == (uint8_t)delimiter)
    {
      d_off = i + 1;
      break;
    }
  }

  if(d_off == -1)
  {
    return NULL;
  }

  // Create string buffer
  int o_len = d_off - bb->rpos;
  char *out = (char *)malloc(o_len + 1);
  uint8_t *obuf = byte_buffer_get_bytes(bb, o_len);
  memcpy(out, obuf, o_len);
  free(obuf);
  out[o_len] = '\0';

  return out;
}

char *byte_buffer_get_dstring(byte_buffer_t *bb, char delimiter)
{
  char *in = byte_buffer_get_string(bb, delimiter);
  int sz = strlen(in);
  char *out = malloc(sz);
  memcpy(out, in, sz-1);
  out[sz-1] = '\0';

  free(in);
  return out;
}

uint32_t secure_rand(uint32_t min, uint32_t max)
{
  uint32_t range = max - min;
  byte_buffer_t *bb = byte_buffer_alloc(); 
  byte_buffer_put_rand(bb, 4);
  uint32_t out = byte_buffer_get_uint32(bb);
  out = out % range;
  byte_buffer_destroy(bb);
  return min + out;
}