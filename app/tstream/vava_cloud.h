/*******************************************************************************
* write at SZ, 2018-10-11 11:08:16, by echo
*******************************************************************************/

#if !defined(_VAVA_CLOUD_)
#define _VAVA_CLOUD_

#include "http_curl.h"
#include "common_func.h"

#define VAVA_SERVER_URL  "https://iot-api-test.sunvalleycloud.com"


#define CURL_MAX_OUT_LENGTH_512  512
#define CURL_MAX_OUT_LENGTH_4096  4096


typedef enum {
	vava_login_msg_rsp,  //登录命令
	vava_device_bind_rsp,  //绑定激活命令
	vava_get_did_rsp,      //获取p2p通道did
	vava_get_session_check_rsp, //p2p通道 回话确认	
	vava_get_ota_fw_rsp,	//静默升级获取升级包信息
}e_vava_msg_type;

int vava_login(void);
void init_vava_cloud(void);
void vava_set_ticket(char *t);
int vava_get_ota_fwbin(void);

/*
typedef struct vava_cloud_info
{
	char access_token[64];
	char token_type[32];
	char refresh_token[64];
	
}vava_cloud_info_t;
*/

#endif
