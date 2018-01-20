#include "ksock.h"
#include "debug.h"

ksock_udp_t *ksock_udp_bind(int port)
{
  ksock_handle_t sock;
  struct sockaddr_in si_me;

  memset(&si_me, 0, sizeof(struct sockaddr_in));
#ifdef _WIN32
  WSADATA wsa;

  if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
  {
    return NULL;
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == INVALID_SOCKET)
  {
    return NULL;
  }
#else
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock <= 0)
  {
    return NULL;
  }
#endif

  si_me.sin_family = AF_INET;
  si_me.sin_addr.s_addr = htonl(INADDR_ANY); // listen from 0.0.0.0
  si_me.sin_port = htons(port);

  if ( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
  {
    return NULL;
  }

  ksock_udp_t *c = (ksock_udp_t*)malloc(sizeof(ksock_udp_t));
  memset(c, 0, sizeof(ksock_udp_t));
  c->sock = sock;

  return c;
}

struct sockaddr_in *ksock_query_hostname(char *hostname, int port)
{
  struct hostent *host;
  int sl = sizeof(struct sockaddr_in);
  struct sockaddr_in *addr = (struct sockaddr_in*)malloc(sl);

  if ( (host = gethostbyname(hostname)) == NULL )
  {
      return NULL;
  }

  memset(addr, 0, sl);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = *(long*)(host->h_addr);

  return addr;
}

void ksock_destroy_hostname(struct sockaddr_in *s)
{
  free(s);
}

byte_buffer_t *ksock_udp_recvfrom_buf(
  ksock_udp_t *c,
  struct sockaddr_in *peer_address, // Sets this pointer to the address of the peer.
  int expected)
{
  unsigned char *buf = (unsigned char *)malloc(expected);
  int limited = 0;
  int s = sizeof(struct sockaddr_in);
  
  limited = recvfrom(c->sock, buf, expected, 0, (struct sockaddr *)peer_address, &s);

  if (limited < 0)
  {
    free(buf);
    return NULL;
  }

  byte_buffer_t *n = byte_buffer_alloc();
  byte_buffer_put_bytes(n, buf, limited);
  free(buf);
  return n;
}

int ksock_udp_sendto_buf(
  ksock_udp_t *c,
  struct sockaddr_in *peer_address,
  byte_buffer_t *bb)
{
  int s = sizeof(struct sockaddr_in);
  return sendto(c->sock, bb->buf, bb->wpos, 0, (struct sockaddr*)peer_address, s);
}

void ksock_udp_close(ksock_udp_t *c)
{
#ifdef _WIN32
  closesocket(c->sock);
  WSACleanup();
#else
  close(c->sock);
#endif
  free(c);
}