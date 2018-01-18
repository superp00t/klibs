#ifndef KSOCK_H__
#define KSOCK_H__
#ifdef _WIN32
// WinSock systems
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

typedef SOCKET ksock_handle_t;

#else
// UNIX-based systems
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>

typedef int ksock_handle_t;

#endif

#ifdef KTLS
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

typedef struct {
  ksock_handle_t sock;
  int            using_ssl;
#ifdef KTLS
  SSL           *ssl;
  SSL_CTX       *ctx;
#endif
} ksock_t;

#include "byte_buffer.h"
void ksock_close(ksock_t *sock);
ksock_t *ksock_dial(const char *hostname, int port, int ssl);
int ksock_send_buf(ksock_t *c, byte_buffer_t *b);
byte_buffer_t *ksock_recv_buf(ksock_t *c, int expected);
#endif