/*
 * http_client.h
 */
#if (defined(FRDM_K28F) || defined(MXRT_595s))
#ifndef RS9116_NBZ_WC_GENR_OSI_1_0_0_BETA_HOST_SAPIS_NWK_APPLICATIONS_HTTP_CLIENT_HTTP_CLIENT_H_
#define RS9116_NBZ_WC_GENR_OSI_1_0_0_BETA_HOST_SAPIS_NWK_APPLICATIONS_HTTP_CLIENT_HTTP_CLIENT_H_
#ifdef RSI_ENABLE_DEMOS
#include <rsi_common_app.h>
#endif
#if (BT_A2DP_SOURCE_WIFI_HTTP_S_RX || WIFI_HTTP_S_5MB_RX_WITH_STANDBY || BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX \
     || BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX || BLE_MULTI_SLAVE_MASTER_BT_A2DP_WIFI_HTTP_S_RX                      \
     || WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY || UNIFIED_PROTOCOL || WIFI_HTTP_SOCKET_SELECT || COEX_MAX_APP \
     || COEX_TEST_FRAMEWORK || COEX_MAX_APP_BLE_2MAS_8SLAV)
#include "stdint.h"

struct parsed_url {
  char *uri;      /* mandatory */
  char *scheme;   /* mandatory */
  char *host;     /* mandatory */
  uint32_t ip;    /* mandatory */
  char *port;     /* optional */
  char *path;     /* optional */
  char *query;    /* optional */
  char *fragment; /* optional */
  char *username; /* optional */
  char *password; /* optional */
};

/*
	Represents an HTTP html response
*/
struct http_response {
  struct parsed_url *request_uri;
  char *status_code;
  int status_code_int;
  char *status_text;
  char *request_headers;
  char *response_headers;
};

//! Enumeration for states in HTTP RESPONSE
typedef enum rsi_http_client_recv_state_e {
  RSI_HTTP_RECV_STATUS  = 0,
  RSI_HTTP_RECV_HEADERS = 1,
  RSI_HTTP_RECV_BODY    = 2,
  RSI_HTTP_RECV_NONE    = 3
} rsi_http_client_recv_state_t;
/*
	Prototype functions
*/
struct http_response *http_req(char *http_headers, struct parsed_url *purl);
struct http_response *http_get(char *url, char *custom_headers);
struct http_response *http_head(char *url, char *custom_headers);
struct http_response *http_post(char *url, char *custom_headers, char *post_data);

char *build_http_get_req(char *url, char *custom_headers);
void http_response_free(struct http_response *hresp);
struct http_response *parse_http_resp(char *response);
struct http_response *parse_http_parse();
void parsed_url_free(struct parsed_url *purl);
#endif
#endif /* RS9116_NBZ_WC_GENR_OSI_1_0_0_BETA_HOST_SAPIS_NWK_APPLICATIONS_HTTP_CLIENT_HTTP_CLIENT_H_ */
#endif