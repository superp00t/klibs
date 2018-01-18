#ifndef _KHTTP_H
#define _KHTTP_H

#include "ksock.h"
#include "byte_buffer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "kurl.h"

#define GET  0
#define POST 1
#define PUT  2
#define HEAD 4

#define FLAG_CHUNKED 1 << 1

#define XFER_ENCODING  1
#define CONTENT_LENGTH 2

typedef struct {
  int      verb;
  kurl_t  *parsedurl;

  ksock_t       *conn;
  int           status;
  byte_buffer_t *body;
} khttp_req_t;

khttp_req_t *khttp_req_alloc(int verb, char *url);

void khttp_do_req(khttp_req_t *khr);

#endif