#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

typedef struct  {
	char *protocol;
	char *host;
	int port;
	char *path;
	char *query_string;
	int host_exists;
	char *host_ip;
} kurl_t;

int kurl_parse(char *url, bool verify_host, kurl_t *parsed_url);
void kurl_destroy(kurl_t *k);