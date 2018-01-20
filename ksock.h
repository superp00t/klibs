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
#include <fcntl.h> // for open
#include <unistd.h> // for close

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

typedef struct {
  ksock_handle_t sock;
} ksock_udp_t;

#include "byte_buffer.h"

// TCP
void ksock_close(ksock_t *sock);
ksock_t *ksock_dial(const char *hostname, int port, int ssl);
int ksock_send_buf(ksock_t *c, byte_buffer_t *b);
byte_buffer_t *ksock_recv_buf(ksock_t *c, int expected);

// UDP
void ksock_destroy_hostname(struct sockaddr_in *s);
struct sockaddr_in *ksock_query_hostname(char *hostname, int port);
void ksock_udp_close(ksock_udp_t *c);
ksock_udp_t *ksock_udp_bind(int port);
int ksock_udp_sendto_buf(
  ksock_udp_t *c,
  struct sockaddr_in *peer_address,
  byte_buffer_t *bb);

byte_buffer_t *ksock_udp_recvfrom_buf(
ksock_udp_t *c,
struct sockaddr_in *peer_address, // Sets this pointer to the address of the peer.
int expected);
#endif