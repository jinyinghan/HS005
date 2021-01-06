#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "curl/curl.h"

#define HTTP_DEBUG_MSG

#define HTTP_TMP_SIZE 512

#define CERT_PATH    "/etc/SNIP39/cert"
#define KEY_PATH     "/etc/SNIP39/key"

#define DF_CACERT_PATH  "/etc/SNIP39/default/cacert"
#define DF_CERT_PATH    "/etc/SNIP39/default/cert"
#define DF_KEY_PATH     "/etc/SNIP39/default/key"

//#define CURL_DEBUG
#ifdef CURL_DEBUG
#define curl_dbg(x)	\
	if(x != CURLE_OK) {\
		printf( "[%s][%d] failed: %d, %s\n",__FUNCTION__,__LINE__,\
		 x, curl_easy_strerror(x));\
	}
#define multi_dbg(x)	\
	if(x != CURLM_OK) {\
		printf( "[%s][%d] failed: %d, %s\n",__FUNCTION__,__LINE__,\
		 x, curl_multi_strerror(x));\
	}
#else
#define curl_dbg(x)
#define mutli_dbg(x)
#endif

typedef struct {
  char* out;
  size_t size;
  size_t cur;
}OutMemoryStruct;

int http_request (char *url_URL, char *url_Post, uint8_t certFlag,void *out,int verbose);
int https_post_request (char *url_URL, char *url_Post, uint8_t certFlag,void *out,int verbose);
int https_post_request2(char * url_URL, char *url_Post, void *out, int verbose);

int https_get_request (char *url_URL, uint8_t certFlag,void *out,int verbose);
int http_upload(char *url_URL,char *buff,int size,int res_type,long timestamp,int8_t certFlag,char *out,int verbose);
int http_download(char *url,int *process_data1 , int verbose);
void print_link(struct curl_slist *head);
struct curl_slist *http_head_append(struct curl_slist *head, char *event_level, int res_type, long timestamp);
int free_head(struct curl_slist *head);
int auth_file_path_update(char *cacert, char *cert, char *key);
int get_verbose(void);
int http_mutli_upload(char *url_URL, int res_type, long timestamp, char *out);


#endif
