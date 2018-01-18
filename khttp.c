#include "khttp.h"
#include "debug.h"

khttp_req_t *khttp_req_alloc(int verb, char *url)
{
  khttp_req_t *khr = (khttp_req_t*)malloc(sizeof(khttp_req_t));
  memset(khr, 0, sizeof(khttp_req_t));
  
  khr->verb = verb;
  khr->parsedurl = (kurl_t *)malloc(sizeof(kurl_t));
  memset(khr->parsedurl, 0, sizeof(kurl_t));
  int resp = kurl_parse(url, false, khr->parsedurl);

  return khr;
}

char *khttp_verb_tostring(int verb)
{
  switch (verb)
  {
    case GET:
    return "GET";
    break;

    case POST:
    return "POST";
    break;

    case PUT:
    return "PUT";
    break;

    case HEAD:
    return "HEAD";
    break;

    default:
    return "GET";
    break;
  }
}

void khttp_destroy(khttp_req_t *khr)
{
  kurl_destroy(khr->parsedurl);
  ksock_close(khr->conn);
  if(khr->body != NULL)
  {
    byte_buffer_destroy(khr->body);
  }

  free(khr);
}

void khttp_do_req(khttp_req_t *khr)
{
  int port = 80;
  int ssl  = 0;
  if (khr->parsedurl->port == 0)
  {
    if (strcmp(khr->parsedurl->protocol, "https") == 0)
    {
      port = 443;
      ssl++;
    }
  }
  else
  {
    port = khr->parsedurl->port;
    if (strcmp(khr->parsedurl->protocol, "https") == 0)
    {
      ssl++;
    }
  }

  khr->conn = ksock_dial(khr->parsedurl->host, port, ssl);
  if(khr->conn == NULL)
  {
    khr->status = -1;
    return;
  }

  byte_buffer_t *request_buf = byte_buffer_alloc();
  // Append verb buffer: GET, PUT, POST?
  byte_buffer_put_string(request_buf, khttp_verb_tostring(khr->verb));
  byte_buffer_put_string(request_buf, " "); // "GET "
  byte_buffer_put_string(request_buf, khr->parsedurl->path); // "GET /path/"
  byte_buffer_put_string(request_buf, " HTTP/1.1\r\n"); // "GET /path/ HTTP/1.1\r\n"
  
  // Add user agent
  byte_buffer_put_string(request_buf, "User-Agent: ");
  byte_buffer_put_string(request_buf, USER_AGENT);
  byte_buffer_put_string(request_buf, "\r\n");

  // Add host
  byte_buffer_put_string(request_buf, "Host: ");
  byte_buffer_put_string(request_buf, khr->parsedurl->host);
  byte_buffer_put_string(request_buf, "\r\n\r\n");

  KDEBUGF("%s\n", request_buf->buf);
  
  ksock_send_buf(khr->conn, request_buf);

  byte_buffer_destroy(request_buf);

  byte_buffer_t *response_buf = ksock_recv_buf(khr->conn, 8192);

  // KDEBUGF("%s\n", response_buf->buf);

  char *data = byte_buffer_get_dstring(response_buf, ' ');
  if(data == NULL)
  {
    KDEBUGF("Data is null\n");
    khr->status = -1;
    return;
  }

  if(strcmp(data, "HTTP/1.1") != 0)
  {
    KDEBUGF("Protocol is not HTTP\n");
    khr->status = -1;
    return;
  }

  free(data);

  int flags = 0;

  char *resp_code = byte_buffer_get_dstring(response_buf, ' ');
  if(resp_code == NULL)
  {
    KDEBUGF("Resp code is null\n");
    khr->status = -1;
    return;
  }

  int resp = strtol(resp_code, NULL, 10);
  KDEBUGF("HTTP status code: %d\n", resp);
  free(resp_code);

  khr->status = resp;

  char *dump = byte_buffer_get_dstring(response_buf, '\n');
  free(dump);

  int  content_length = 0;

  char *header = NULL;
  char *value = NULL;

  int  scanning = 1;
  // Scan response headers
  while(scanning)
  {
    header = byte_buffer_get_dstring(response_buf, ':');
    if(header == NULL)
    {
      khr->status = -1;
      break;
    }

    if(strcmp(header, "") == 0)
    {
      khr->status = -1;
      break;
    }

    // Detect header type
    int hcode = 0;
    if (strcmp(header, "Transfer-Encoding") == 0)
    {
      hcode = XFER_ENCODING;
    } 
    else
    if (strcmp(header, "Content-Length") == 0)
    {
      hcode = CONTENT_LENGTH;
    }

    // ignore whitespace
    byte_buffer_get_byte(response_buf);

    value = byte_buffer_get_dstring(response_buf, '\r');
    if(value == NULL)
    {
      khr->status = -1;
      break;
    }

    if(hcode == CONTENT_LENGTH)
    {
      content_length = strtol(value, NULL, 10);
    }
    else
    if(hcode == XFER_ENCODING)
    {
      if (strcmp(value, "chunked") == 0)
      {
        flags |= FLAG_CHUNKED;
      }
    }

    // ignore newline
    byte_buffer_get_byte(response_buf);

    if(header != NULL)
    {
      free(header);
      header = NULL;
    }
  
    if(value != NULL)
    {
      free(value);
      value = NULL;
    }

    if((response_buf->wpos - response_buf->rpos) > 2)
    {
      if (byte_buffer_get_byte(response_buf) == '\r' &&
          byte_buffer_get_byte(response_buf) == '\n')
      {
        // We've reached the end of the header field
        scanning = 0;
      }
      else
      {
        response_buf->rpos -= 1;
      }
    }
    else
    {
      khr->status = -1;
      return;
    }
  }

  byte_buffer_t *out = byte_buffer_alloc();
  
  // Now we read the response.
  if (flags & FLAG_CHUNKED)
  {
    // Use HTTP/1.1 chunking.
    int scan_resp = 1;
    while(scan_resp)
    {
      char *chunk_str = byte_buffer_get_dstring(response_buf, '\r');
      byte_buffer_get_byte(response_buf);
      
      int chunk_length = strtol(chunk_str, NULL, 16);
      if(chunk_length == 0)
      {
        scan_resp = 0;
        free(chunk_str);
        break;
      }

      if((response_buf->wpos - response_buf->rpos) < chunk_length)
      {
        free(chunk_str);
        byte_buffer_destroy(out);
        byte_buffer_destroy(response_buf);
        khr->status = -1;
        return;
      }

      int prev = response_buf->rpos;
      
      uint8_t *tmpdata = byte_buffer_get_bytes(response_buf, chunk_length);
      byte_buffer_put_bytes(out, tmpdata, chunk_length);
      free(tmpdata);

      // \r\n
      byte_buffer_get_byte(response_buf);
      byte_buffer_get_byte(response_buf);
      
      free(chunk_str);
    }
  }
  else
  {
    // Use standard HTTP/1.0 response format
    int bytes_read = 0;

    while(bytes_read < content_length)
    {
      int remaining = response_buf->wpos - response_buf->rpos;
      if((content_length - bytes_read) > remaining)
      {
        // Read new packet from network connection
        byte_buffer_t *additional_data = ksock_recv_buf(khr->conn, 4096);
        if(additional_data == NULL)
        {
          byte_buffer_destroy(out);
          byte_buffer_destroy(response_buf);
          khr->status = -1;
          return;
        }

        // Append new packet to response buffer
        byte_buffer_put_bytes(response_buf, additional_data->buf, additional_data->wpos);
        byte_buffer_destroy(additional_data);

        continue;
      }

      uint8_t *tmpdata = byte_buffer_get_bytes(response_buf, remaining);
      bytes_read += remaining;
      byte_buffer_put_bytes(out, tmpdata, remaining);
      free(tmpdata);
    }
  }

  byte_buffer_destroy(response_buf);
  khr->body = out;
}
