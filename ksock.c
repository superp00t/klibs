
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "ksock.h"
#include "byte_buffer.h"
#include "debug.h"

void ksock_close(ksock_t *sock)
{
#ifdef _WIN32
    closesocket(sock->sock);
    WSACleanup();
#else
    close(sock->sock);
#endif

#ifdef KTLS
    if(sock->using_ssl)
    {
        SSL_free(sock->ssl);
        SSL_CTX_free(sock->ctx);
    }
#endif
    free(sock);
}

static int init_ssl_data = -1;

#ifdef KTLS
SSL_CTX* init_ctx(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;
    if(init_ssl_data == -1)
    {
        SSL_library_init();
        init_ssl_data = 1;
    }
    OpenSSL_add_all_algorithms();		/* Load cryptos, et.al. */
    SSL_load_error_strings();			/* Bring in and register error messages */
    method = SSLv23_client_method();		/* Create new client-method instance */
    ctx = SSL_CTX_new(method);			/* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    return ctx;
}
#endif

ksock_t *ksock_dial(const char *hostname, int port, int ssl)
{
    ksock_t *ksocket = (ksock_t*)malloc(sizeof(ksock_t));
    memset(ksocket, 0, sizeof(ksock_t));

#ifdef _WIN32
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        free(ksocket);
        return NULL;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(hostname, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        free(ksocket);
        return NULL;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family,
                               ptr->ai_socktype, 
                               ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            free(ksocket);
            return NULL;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        free(ksocket);
        return NULL;
    }

    ksocket->sock = ConnectSocket;
#else
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;

    if ( (host = gethostbyname(hostname)) == NULL )
    {
        return NULL;
    }

    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, &addr, sizeof(addr)) != 0 )
    {
        close(sd);
        return NULL;
    }

    ksocket->sock = sd;
#endif
    if(ssl)
    {
#ifdef KTLS
        ksocket->using_ssl = 1;
        ksocket->ctx = init_ctx();
        ksocket->ssl  = SSL_new(ksocket->ctx);
        SSL_set_fd(ksocket->ssl, ksocket->sock);
        if (SSL_connect(ksocket->ssl) == -1)
        {
            // Todo: cleanup ssl
            close(ksocket->sock);
            free(ksocket);
            return NULL;
        }
    
#else
        KDEBUGF("Cannot connect with SSL without OpenSSL.");
        close(ksocket->sock);
        free(ksocket);
        return NULL;
#endif
    }
    
    return ksocket;
}

int ksock_send_buf(ksock_t *c, byte_buffer_t *b)
{
    if(c->using_ssl)
    {
#ifdef KTLS
        return SSL_write(c->ssl, b->buf, b->wpos);
#endif
    }

    return send(c->sock, b->buf, b->wpos, 0);
}

byte_buffer_t *ksock_recv_buf(ksock_t *c, int expected)
{
    unsigned char *buf = (unsigned char *)malloc(expected);
    int limited = 0;

    if(c->using_ssl)
    {
#ifdef KTLS
        limited = SSL_read(c->ssl, buf, expected);
#endif
    } else
    {
        limited = recv(c->sock, buf, expected, 0);
    }

    if(limited < 0)
    {
        free(buf);
        return NULL;
    }

    byte_buffer_t *out = byte_buffer_alloc();
    byte_buffer_put_bytes(out, buf, limited);
    free(buf);

    return out;
}
