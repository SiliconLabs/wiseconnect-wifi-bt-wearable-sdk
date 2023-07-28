/*
* http-client.c
*/
#if (defined(FRDM_K28F) || defined(MXRT_595s))
#ifdef RSI_ENABLE_DEMOS
#include <rsi_common_app.h>
#endif
#if (BT_A2DP_SOURCE_WIFI_HTTP_S_RX || WIFI_HTTP_S_5MB_RX_WITH_STANDBY || BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX \
     || BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX || BLE_MULTI_SLAVE_MASTER_BT_A2DP_WIFI_HTTP_S_RX                      \
     || WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY || UNIFIED_PROTOCOL || WIFI_HTTP_SOCKET_SELECT || COEX_MAX_APP \
     || COEX_TEST_FRAMEWORK || COEX_MAX_APP_BLE_2MAS_8SLAV)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "http_client.h"

#include <errno.h>
#include <stringx.h>
#include <urlparser.h>

uint16_t http_body_len;

struct parsed_url *purl = NULL;
extern uint8_t RSI_HTTP_STATE;

#if !SD_DEMO
extern sd_data_buff_t sd_buff;
#else
extern uint8_t *sdram;
extern volatile uint32_t sdram_idx;
extern volatile uint32_t data_processed;
#endif

/*
 Handles redirect if needed for get requests
 */
struct http_response *handle_redirect_get(struct http_response *hresp, char *custom_headers)
{
  if (hresp->status_code_int > 300 && hresp->status_code_int < 399) {
    char *token = strtok(hresp->response_headers, "\r\n");
    while (token != NULL) {
      if (str_contains(token, "Location:")) {
        /* Extract url */
        char *location = str_replace("Location: ", "", token);
        return http_get(location, custom_headers);
      }
      token = strtok(NULL, "\r\n");
    }
  } else {
    /* We're not dealing with a redirect, just return the same structure */
    return hresp;
  }

  return hresp;
}

/*
 Parse HTTP Response
 */
struct http_response *parse_http_resp(char *response)
{
  int offset        = 0;
  uint32_t resp_len = 0;
#if !SD_DEMO
  resp_len = sd_buff.len;
#else
  if (data_processed > 1024)
    resp_len = 1024;
  else
    resp_len = data_processed; //! SDRAM index
#endif

  /* Allocate memory for http response */
  struct http_response *hresp = (struct http_response *)malloc(sizeof(struct http_response));
  if (hresp == NULL) {
    LOG_PRINT("Unable to allocate memory for http content.");
    return NULL;
  }
  memset(hresp, '\0', sizeof(struct http_response));
  http_body_len           = 0;
  hresp->request_headers  = NULL;
  hresp->response_headers = NULL;
  hresp->status_code      = NULL;
  hresp->status_code_int  = 0;
  hresp->status_text      = NULL;

  /* Parse status code and text */
  if (strstr(response, "HTTP/1")) {
    char *status_line = get_until(response, "\r\n");

    //! first argument should be "HTTP/1.0 " or "HTTP/1.1 "
    if (strstr(status_line, "HTTP/1.1 "))
      status_line = str_replace("HTTP/1.1 ", "", status_line);
    else if (strstr(status_line, "HTTP/1.0 "))
      status_line = str_replace("HTTP/1.0 ", "", status_line);
    else
      return NULL;
    char *status_code      = str_ndup(status_line, 4);
    status_code            = str_replace(" ", "", status_code);
    char *status_text      = str_replace(status_code, "", status_line);
    status_text            = str_replace(" ", "", status_text);
    hresp->status_code     = status_code;
    hresp->status_code_int = rsi_atoi((const int8_t *)status_code);
    hresp->status_text     = status_text;
  }

  //! Check for response headers
  if (strstr(response, "\r\n\r\n")) {
    /* Parse response headers */
    char *headers           = get_until(response, "\r\n\r\n");
    hresp->response_headers = headers;

    if (RSI_HTTP_STATE != RSI_HTTP_RECV_BODY) {
#if !SD_DEMO
      offset = str_index_of((char *)sd_buff.buff, "\r\n\r\n");
#else
      offset = str_index_of((char *)(sdram + sdram_idx), "\r\n\r\n");
#endif
      http_body_len = resp_len - offset - strlen("\r\n\r\n");
#if SD_DEMO
      sdram_idx += offset + strlen("\r\n\r\n");
#endif
    }
  }

  /* Return response */
  return hresp;
}

/*
 Makes a HTTP GET request to the given url
 */
char *build_http_get_req(char *url, char *custom_headers)
{
  /* Parse url */
  purl = parse_url(url);
  if (purl == NULL) {
    LOG_PRINT("Unable to parse http url");
    return NULL;
  }

  /* Declare variable */
  char *upwd        = NULL;
  char *base64      = NULL;
  char *auth_header = NULL;
  char *http_req    = (char *)malloc(1024);

  /* Build query/headers */
  if (purl->path != NULL) {
    if (purl->query != NULL) {
      sprintf(http_req, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
    } else {
      sprintf(http_req, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
    }
  } else {
    if (purl->query != NULL) {
      sprintf(http_req, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
    } else {
      sprintf(http_req, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
    }
  }

  /* Handle authorization if needed */
  if (purl->username != NULL) {
    /* Format username:password pair */
    upwd = (char *)malloc(1024);
    sprintf(upwd, "%s:%s", purl->username, purl->password);
    upwd = (char *)realloc(upwd, strlen(upwd) + 1);

    /* Base64 encode */
    base64 = base64_encode(upwd);

    /* Form header */
    auth_header = (char *)malloc(1024);
    sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
    auth_header = (char *)realloc(auth_header, strlen(auth_header) + 1);

    /* Add to header */
    http_req = (char *)realloc(http_req, strlen(http_req) + strlen(auth_header) + 2);
    sprintf(http_req, "%s%s", http_req, auth_header);
  }

  /* Add user-agent header */
  if (HTTP_USER_AGENT_HEADER != NULL) {
    http_req = (char *)realloc(http_req, strlen(http_req) + strlen(HTTP_USER_AGENT_HEADER) + 2);
    sprintf(http_req, "%s%s\r\n", http_req, HTTP_USER_AGENT_HEADER);
  } else {
    sprintf(http_req, "%s\r\n", http_req);
  }
  http_req = (char *)realloc(http_req, strlen(http_req) + 1);

  /* Add accept header */
  if (HTTP_ACCEPT_HEADER != NULL) {
    http_req = (char *)realloc(http_req, strlen(http_req) + strlen(HTTP_ACCEPT_HEADER) + 2);
    sprintf(http_req, "%s%s\r\n", http_req, HTTP_ACCEPT_HEADER);
  } else {
    sprintf(http_req, "%s\r\n", http_req);
  }
  http_req = (char *)realloc(http_req, strlen(http_req) + 1);

  /* Add custom headers, and close */
  if (custom_headers != NULL) {
    http_req = (char *)realloc(http_req, strlen(http_req) + strlen(custom_headers) + 2);
    sprintf(http_req, "%s%s\r\n", http_req, custom_headers);
  } else {
    sprintf(http_req, "%s\r\n", http_req);
  }
  http_req = (char *)realloc(http_req, strlen(http_req) + 1);

  if (upwd != NULL) {
    free(upwd);
    upwd = NULL;
  }
  if (base64 != NULL) {
    free(base64);
    base64 = NULL;
  }
  if (auth_header != NULL) {
    free(auth_header);
    auth_header = NULL;
  }

  return http_req;
}

/*
 Free memory of http_response
 */
void http_response_free(struct http_response *hresp)
{
  if (hresp != NULL) {
    //if (hresp->body != NULL)
    //free(hresp->body);
    if (hresp->status_code != NULL)
      free(hresp->status_code);
    if (hresp->status_text != NULL)
      free(hresp->status_text);
    if (hresp->response_headers != NULL)
      free(hresp->response_headers);
    free(hresp);
  }
}
#endif
#endif
