/*
*	urlparser.h
*/

#if (defined(FRDM_K28F) || defined(MXRT_595s))
#ifdef RSI_ENABLE_DEMOS
#include "rsi_common_app.h"
#endif
#if (BT_A2DP_SOURCE_WIFI_HTTP_S_RX || WIFI_HTTP_S_5MB_RX_WITH_STANDBY || BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX \
     || BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX || BLE_MULTI_SLAVE_MASTER_BT_A2DP_WIFI_HTTP_S_RX                      \
     || WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY || UNIFIED_PROTOCOL || WIFI_HTTP_SOCKET_SELECT || COEX_MAX_APP \
     || COEX_TEST_FRAMEWORK || COEX_MAX_APP_BLE_2MAS_8SLAV)
#include "rsi_wlan_config.h"
#include "rsi_driver.h"
#include "rsi_nwk.h"
#include "rsi_socket.h"
#include "stdio.h"
#include "stdint.h"
#include "http_client.h"
#include "rsi_utils.h"

/*
	Free memory of parsed url
*/
void parsed_url_free(struct parsed_url *purl)
{
  if (NULL != purl) {
    if (NULL != purl->scheme)
      free(purl->scheme);
    if (NULL != purl->host)
      free(purl->host);
#if !RSI_DNS_CLIENT
    if (NULL != purl->port)
      free(purl->port);
#endif
    if (NULL != purl->path)
      free(purl->path);
    if (NULL != purl->query)
      free(purl->query);
    if (NULL != purl->fragment)
      free(purl->fragment);
    if (NULL != purl->username)
      free(purl->username);
    if (NULL != purl->password)
      free(purl->password);
    free(purl);
  }
}

/*
	Retrieves the IP adress of a hostname
*/
uint32_t hostname_to_ip(char *hostname)
{
  int32_t status = 0;
  rsi_rsp_dns_query_t dns_resp;

  //! Support for only IPv4
  status = rsi_dns_req(RSI_IP_VERSION_4, (uint8_t *)hostname, NULL, NULL, &dns_resp, sizeof(dns_resp));
  if (status != 0) {
    return 0;
  }

  return rsi_bytes4R_to_uint32(dns_resp.ip_address[0].ipv4_address);
}

/*
	Check whether the character is permitted in scheme string
*/
int is_scheme_char(int c)
{
  return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

/*
	Parses a specified URL and returns the structure named 'parsed_url'
	Implented according to:
	RFC 1738 - http://www.ietf.org/rfc/rfc1738.txt
	RFC 3986 -  http://www.ietf.org/rfc/rfc3986.txt
*/
struct parsed_url *parse_url(const char *url)
{

  /* Define variable */
  struct parsed_url *purl;
  const char *tmpstr;
  const char *curstr;
  int len;
  int i;
  int userpass_flag;
  int bracket_flag;
  uint32_t ip_addr;

  /* Allocate the parsed url storage */
  purl = (struct parsed_url *)malloc(sizeof(struct parsed_url));
  if (NULL == purl) {
    LOG_PRINT("malloc failed for url parse\n");
    return NULL;
  }
  memset(purl, '\0', sizeof(struct parsed_url));
  purl->scheme   = NULL;
  purl->host     = NULL;
  purl->port     = NULL;
  purl->path     = NULL;
  purl->query    = NULL;
  purl->fragment = NULL;
  purl->username = NULL;
  purl->password = NULL;
  curstr         = url;

  /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */
  /* Read scheme */
  tmpstr = strchr(curstr, ':');
  if (NULL == tmpstr) {
    parsed_url_free(purl);
    fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);

    return NULL;
  }

  /* Get the scheme length */
  len = tmpstr - curstr;

  /* Check restrictions */
  for (i = 0; i < len; i++) {
    if (is_scheme_char(curstr[i]) == 0) {
      /* Invalid format */
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
  }
  /* Copy the scheme to the storage */
  purl->scheme = (char *)malloc(sizeof(char) * (len + 1));
  if (NULL == purl->scheme) {
    parsed_url_free(purl);
    fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);

    return NULL;
  }

  (void)strncpy(purl->scheme, curstr, len);
  purl->scheme[len] = '\0';

  /* Make the character to lower if it is upper case. */
  for (i = 0; i < len; i++) {
    purl->scheme[i] = tolower(purl->scheme[i]);
  }

  /* Skip ':' */
  tmpstr++;
  curstr = tmpstr;

  /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
  /* Eat "//" */
  for (i = 0; i < 2; i++) {
    if ('/' != *curstr) {
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
    curstr++;
  }

  /* Check if the user (and password) are specified. */
  userpass_flag = 0;
  tmpstr        = curstr;
  while ('\0' != *tmpstr) {
    if ('@' == *tmpstr) {
      /* Username and password are specified */
      userpass_flag = 1;
      break;
    } else if ('/' == *tmpstr) {
      /* End of <host>:<port> specification */
      userpass_flag = 0;
      break;
    }
    tmpstr++;
  }

  /* User and password specification */
  tmpstr = curstr;
  if (userpass_flag) {
    /* Read username */
    while ('\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr) {
      tmpstr++;
    }
    len            = tmpstr - curstr;
    purl->username = (char *)malloc(sizeof(char) * (len + 1));
    if (NULL == purl->username) {
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
    (void)strncpy(purl->username, curstr, len);
    purl->username[len] = '\0';

    /* Proceed current pointer */
    curstr = tmpstr;
    if (':' == *curstr) {
      /* Skip ':' */
      curstr++;

      /* Read password */
      tmpstr = curstr;
      while ('\0' != *tmpstr && '@' != *tmpstr) {
        tmpstr++;
      }
      len            = tmpstr - curstr;
      purl->password = (char *)malloc(sizeof(char) * (len + 1));
      if (NULL == purl->password) {
        parsed_url_free(purl);
        fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
      }
      (void)strncpy(purl->password, curstr, len);
      purl->password[len] = '\0';
      curstr              = tmpstr;
    }
    /* Skip '@' */
    if ('@' != *curstr) {
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
    curstr++;
  }

  if ('[' == *curstr) {
    bracket_flag = 1;
  } else {
    bracket_flag = 0;
  }
  /* Proceed on by delimiters with reading host */
  tmpstr = curstr;
  while ('\0' != *tmpstr) {
    if (bracket_flag && ']' == *tmpstr) {
      /* End of IPv6 address. */
      tmpstr++;
      break;
    } else if (!bracket_flag && (':' == *tmpstr || '/' == *tmpstr)) {
      /* Port number is specified. */
      break;
    }
    tmpstr++;
  }
  len        = tmpstr - curstr;
  purl->host = (char *)malloc(sizeof(char) * (len + 1));
  if (NULL == purl->host || len <= 0) {
    parsed_url_free(purl);
    fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
    return NULL;
  }
  (void)strncpy(purl->host, curstr, len);
  purl->host[len] = '\0';
  curstr          = tmpstr;

  /* Is port number specified? */
  if (':' == *curstr) {
    curstr++;
    /* Read port number */
    tmpstr = curstr;
    while ('\0' != *tmpstr && '/' != *tmpstr) {
      tmpstr++;
    }
    len        = tmpstr - curstr;
    purl->port = (char *)malloc(sizeof(char) * (len + 1));
    if (NULL == purl->port) {
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
    (void)strncpy(purl->port, curstr, len);
    purl->port[len] = '\0';
    curstr          = tmpstr;
  } else {
    if (strstr(purl->scheme, "https"))
      purl->port = "443";

    else if (strstr(purl->scheme, "http"))
      purl->port = "80";
    else //! Nothing!!!
      ;
  }

  /* Get IP */
#if RSI_DNS_CLIENT
  ip_addr = hostname_to_ip(purl->host);
  if (ip_addr == 0) {
    purl = NULL;
    return purl;
  }
#else
  ip_addr = ip_to_reverse_hex((char *)SERVER_IP_ADDRESS);
#endif

  //char *ip = (char*)ip_addr;
  purl->ip = ip_addr;

  /* Set uri */
  purl->uri = (char *)url;

  /* End of the string */
  if ('\0' == *curstr) {
    return purl;
  }

  /* Skip '/' */
  if ('/' != *curstr) {
    parsed_url_free(purl);
    fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
    return NULL;
  }
  curstr++;

  /* Parse path */
  tmpstr = curstr;
  while ('\0' != *tmpstr && '#' != *tmpstr && '?' != *tmpstr) {
    tmpstr++;
  }
  len        = tmpstr - curstr;
  purl->path = (char *)malloc(sizeof(char) * (len + 1));
  if (NULL == purl->path) {
    parsed_url_free(purl);
    fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
    return NULL;
  }
  (void)strncpy(purl->path, curstr, len);
  purl->path[len] = '\0';
  curstr          = tmpstr;

  /* Is query specified? */
  if ('?' == *curstr) {
    /* Skip '?' */
    curstr++;
    /* Read query */
    tmpstr = curstr;
    while ('\0' != *tmpstr && '#' != *tmpstr) {
      tmpstr++;
    }
    len         = tmpstr - curstr;
    purl->query = (char *)malloc(sizeof(char) * (len + 1));
    if (NULL == purl->query) {
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
    (void)strncpy(purl->query, curstr, len);
    purl->query[len] = '\0';
    curstr           = tmpstr;
  }

  /* Is fragment specified? */
  if ('#' == *curstr) {
    /* Skip '#' */
    curstr++;
    /* Read fragment */
    tmpstr = curstr;
    while ('\0' != *tmpstr) {
      tmpstr++;
    }
    len            = tmpstr - curstr;
    purl->fragment = (char *)malloc(sizeof(char) * (len + 1));
    if (NULL == purl->fragment) {
      parsed_url_free(purl);
      fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
      return NULL;
    }
    (void)strncpy(purl->fragment, curstr, len);
    purl->fragment[len] = '\0';
    curstr              = tmpstr;
  }
  return purl;
}
#endif
#endif
