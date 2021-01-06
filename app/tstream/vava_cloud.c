

//此文件 实现了 《IPC_HS005固件接口文档.docx》 的内容
#include <sys/types.h>
#include <json/json.h>

#include "common_env.h"
#include "vava_cloud.h"
#include "log.h"

char *vava_client_id="daf3e403efd446668a8168e6781fe426";
char *vava_client_secret="7a0de155a9434c00aea70829f45fe90c";
char vava_sn[MAX_UID_SIZE+1]={"P020201000301201117500001"};

char vava_login_tocken[64]={0};
char vava_ticket[64]={0};
char vava_session_key[64]={0};
extern char vava_otn_bsup_url[128];
extern char vava_otn_bsup_upver[7];
extern char vava_otn_bsup_timestart[7];
extern char vava_otn_bsup_timeend[7];


char vava_login_status = 0;// 1 is login in ok


long long vava_get_time_stamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}


void vava_set_ticket(char *t)
{
	snprintf(vava_ticket, sizeof(vava_ticket), "%s",t);
	return ;
}

void vava_set_session_key(char *k)
{
	strncpy(vava_session_key,k,sizeof(vava_session_key));
	return ;
}

//信息解析
int vava_msg_parser(char *out,e_vava_msg_type m_type)
{
	int ret = -1;
	char * ret_string;
	struct json_object *js_obj=NULL, *members_obj=NULL, *otainfo_obj = NULL;
	struct json_object *data_obj=NULL;
	
	if(!out)
		return -1;

	js_obj = json_tokener_parse(out);
    if(!js_obj)
    {
        LOG("vava_msg_parser parse failed:%s\n",out);
        goto end;
    }
	members_obj = json_object_object_get(js_obj,"stateCode");
    if(!members_obj)
    {
        goto end;
    }
	ret = json_object_get_int(members_obj);
	if(ret != 200)
	{
		LOG("stateCode %d\n",ret);
		return ret;
	}
	
	switch(m_type)
	{
		case vava_login_msg_rsp:
		{
			LOG("vava_login_msg_rsp start parse data\n");
			members_obj = json_object_object_get(js_obj,"data");
		    if(!members_obj)
		    {
		        goto end;
		    }
			data_obj = json_tokener_parse(json_object_get_string(members_obj));
			if(!data_obj)
			{
				goto end;
			}  

			members_obj = json_object_object_get(data_obj,"access_token");
			if(!members_obj)
			{
				goto end;
			}
			ret_string = (char *)json_object_get_string(members_obj);
			snprintf(vava_login_tocken,64-1,"%s",ret_string);
			LOG("vava_login_tocken %s \n",vava_login_tocken);
			json_object_put(data_obj);
			data_obj=NULL;
		}
		break;
		case vava_device_bind_rsp:
		{
			LOG("vava_device_bind OK\n");
		}
		break;		
		case vava_get_did_rsp:
		{
			char *didCode;
			char *initCode;
			char *crcKey;
			
			LOG("vava_get_did_rsp\n");
			members_obj = json_object_object_get(js_obj,"data");
		    if(!members_obj)
		    {
		        goto end;
		    }
			data_obj = json_tokener_parse(json_object_get_string(members_obj));
			if(!data_obj)
			{
				goto end;
			}  

			members_obj = json_object_object_get(data_obj,"didCode");
			if(!members_obj)
			{
				goto end;
			}
			didCode = (char *)json_object_get_string(members_obj);

			members_obj = json_object_object_get(data_obj,"initCode");
			if(!members_obj)
			{
				goto end;
			}
			initCode = (char *)json_object_get_string(members_obj);			

			members_obj = json_object_object_get(data_obj,"crcKey");
			if(!members_obj)
			{
				goto end;
			}
			crcKey = (char *)json_object_get_string(members_obj);		
			
			LOG("didCode %s \ninitCode %s \ncrcKey %s\n",didCode,initCode,crcKey);
			ppcs_service_set_param(didCode,initCode,crcKey);
				
			json_object_put(data_obj);
			data_obj=NULL;		
		}
		break;
		case vava_get_session_check_rsp:
		{
			LOG("vava_get_session_check_rsp OK\n");
		}
		break;
		case vava_get_ota_fw_rsp:
		{			
			LOG("vava_get_ota_fw_rsp start parse data\n");
			members_obj = json_object_object_get(js_obj,"data");
		    if(!members_obj)
		    {
		        goto end;
		    }
			data_obj = json_tokener_parse(json_object_get_string(members_obj));
			if(!data_obj)
			{
				goto end;
			}		

			members_obj = json_object_object_get(data_obj,"otaUrl");
			if(!members_obj)
			{
				goto end;
			}
			data_obj = json_tokener_parse(json_object_get_string(members_obj));
			if(!data_obj)
			{
				goto end;
			}
			members_obj = json_object_object_get(data_obj,"bsup");
			if(!members_obj)
			{
				goto end;
			}
			data_obj = json_tokener_parse(json_object_get_string(members_obj));
			if(!data_obj)
			{
				goto end;
			}
			
			otainfo_obj = json_object_object_get(data_obj,"url");
			if(!otainfo_obj)
			{
				goto end;
			}
			
			ret_string = (char *)json_object_get_string(otainfo_obj);
			snprintf(vava_otn_bsup_url,128-1,"%s",ret_string);
			LOG("vava_otn_bsup_url %s \n",vava_otn_bsup_url);
			
			otainfo_obj = json_object_object_get(data_obj,"upver");
			if(!otainfo_obj)
			{
				goto end;
			}
			ret_string = (char *)json_object_get_string(otainfo_obj);
			snprintf(vava_otn_bsup_upver,12-1,"%s",ret_string);
			LOG("vava_otn_bsup_upver %s \n",vava_otn_bsup_upver);
			
			otainfo_obj = json_object_object_get(data_obj,"timestart");
			if(!otainfo_obj)
			{
				goto end;
			}
			ret_string = (char *)json_object_get_string(otainfo_obj);
			snprintf(vava_otn_bsup_timestart,12-1,"%s",ret_string);
			LOG("vava_otn_bsup_timestart %s \n",vava_otn_bsup_timestart);

			otainfo_obj = json_object_object_get(data_obj,"timeend");
			if(!otainfo_obj)
			{
				goto end;
			}
			ret_string = (char *)json_object_get_string(otainfo_obj);
			snprintf(vava_otn_bsup_timeend,12-1,"%s",ret_string);
			LOG("vava_otn_bsup_timeend %s \n",vava_otn_bsup_timeend);
			
			json_object_put(data_obj);
			json_object_put(otainfo_obj);
			data_obj=NULL;
			otainfo_obj=NULL;

		}
		break;
			
		default:
		break;
	}
	
end:	
	if(data_obj)
		json_object_put(data_obj);
	if(js_obj)
		json_object_put(js_obj);
    return ret;
}

//设备登录
int vava_login(void)
{
	OutMemoryStruct oms;
    char sendbuf[512] = {0};
    char *msg_url = NULL;
    char *out = NULL;
    int ret = -1;

    out = (char *)malloc(CURL_MAX_OUT_LENGTH_512);
    if(!out){
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    msg_url = (char *)malloc(1024);
    if(!msg_url){
		free(out);
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    memset( out, 0, CURL_MAX_OUT_LENGTH_512);
    memset( msg_url, 0, 1024);
	
    snprintf( msg_url,1024-1,"%s/oauth/login",VAVA_SERVER_URL );

	get_uid_from_flash((char *)vava_sn);

    snprintf( sendbuf,512-1,"{\"client_id\":\"%s\",\"client_secret\":\"%s\",\"scope\":\"all\",\"grant_type\":\"password\","
			"\"auth_type\":\"sn_password\",\"sn\":\"%s\"}",vava_client_id,vava_client_secret,vava_sn );
    int cloud_dbg = 0;

	//if(access("/tmp/cloud_dbg",F_OK)==0)
        cloud_dbg = 1;
	oms.cur=0;
	oms.size=512;
	oms.out=out;
	
    ret = https_post_request2(msg_url, sendbuf, &oms, cloud_dbg);

	if(ret == 0)
	{
		vava_msg_parser(out, vava_login_msg_rsp);
		vava_login_status = 1;

		g_enviro_struct.t_online = 1;
	}
	else
	{
		
	}
	

	printf("####\n %s\n",out);

    free(msg_url);
    free(out);
    return ret;
}


//激活绑定
int vava_device_bind(void)
{
	OutMemoryStruct oms;
    char sendbuf[512] = {0};
    char *msg_url = NULL;
    char *out = NULL;
    int ret = -1;
    int cloud_dbg = 0;

    out = (char *)malloc(CURL_MAX_OUT_LENGTH_512);
    if(!out){
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    msg_url = (char *)malloc(1024);
    if(!msg_url){
		free(out);
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    memset( out, 0, CURL_MAX_OUT_LENGTH_512);
    memset( msg_url, 0, 1024);
	
    snprintf( msg_url,1024-1,"%s/ipc/device/main/add?access_token=%s&lang=zh_CN&timestamp=%lld",
			VAVA_SERVER_URL,vava_login_tocken,vava_get_time_stamp());

    snprintf( sendbuf,512-1,"{\"deviceSn\":\"%s\",\"deviceName\":\"test\",\"ticket\":\"%s\"}",
		vava_sn,vava_ticket);

	if(access("/tmp/cloud_dbg",F_OK)==0)
        cloud_dbg = 1;
	oms.cur=0;
	oms.size=512;
	oms.out=out;
	
    ret = https_post_request2(msg_url, sendbuf, &oms, cloud_dbg);

	if(ret == 0)
	{
		vava_msg_parser(out, vava_device_bind_rsp);
	}
	else
	{
		
	}
	
	printf("####\n %s\n",out);

    free(msg_url);
    free(out);
    return ret;
}


//获取 DID
int vava_get_did(void)
{
	OutMemoryStruct oms;
    char sendbuf[512] = {0};
    char *msg_url = NULL;
    char *out = NULL;
    int ret = -1;
    int cloud_dbg = 0;

    out = (char *)malloc(CURL_MAX_OUT_LENGTH_512);
    if(!out){
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    msg_url = (char *)malloc(1024);
    if(!msg_url){
		free(out);
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    memset( out, 0, CURL_MAX_OUT_LENGTH_512);
    memset( msg_url, 0, 1024);
	
    snprintf( msg_url,1024-1,"%s/ipc/p2p/get-did?access_token=%s&lang=zh_CN&timestamp=%lld",
			VAVA_SERVER_URL,vava_login_tocken,vava_get_time_stamp() );

	if(access("/tmp/cloud_dbg",F_OK)==0)
        cloud_dbg = 1;
	oms.cur=0;
	oms.size=512;
	oms.out=out;
	
    ret = https_post_request2(msg_url, sendbuf, &oms, cloud_dbg);

	if(ret == 0)
	{
		vava_msg_parser(out, vava_get_did_rsp);
	}
	else
	{
		
	}
	
	printf("####\n %s\n",out);

    free(msg_url);
    free(out);
    return ret;
}

//获取 firmware升级包地址
int vava_get_ota_fwbin(void)
{
	OutMemoryStruct oms;
    char sendbuf[512] = {0};
    char *msg_url = NULL;
    char *out = NULL;
    int ret = -1;
    int otafw_dbg = 0;

    out = (char *)malloc(CURL_MAX_OUT_LENGTH_512);
    if(!out){
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    msg_url = (char *)malloc(1024);
    if(!msg_url){
		free(out);
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    memset( out, 0, CURL_MAX_OUT_LENGTH_512);
    memset( msg_url, 0, 1024);

  	snprintf( msg_url,1024-1,"%s/ipc/device/station/report-status?access_token=%s&timeStamp=%lld&lang=en",
	VAVA_SERVER_URL,vava_login_tocken,vava_get_time_stamp() );

    snprintf( sendbuf,512-1,"{\"cameraStatusObjectList\":[],\"stationStatusObject\":{}}");

    otafw_dbg = 1;
	oms.cur=0;
	oms.size=512;
	oms.out=out;
	
    ret = https_post_request2(msg_url, sendbuf, &oms, otafw_dbg);

	if(ret == 0)
	{
		vava_msg_parser(out, vava_get_ota_fw_rsp);
	}
	else
	{
		//请求失败
	}
	
	printf("####\n %s\n",out);

    free(msg_url);
    free(out);
    return ret;
}


int vava_session_check(void)
{
	OutMemoryStruct oms;
    char sendbuf[512] = {0};
    char *msg_url = NULL;
    char *out = NULL;
    int ret = -1;
    int cloud_dbg = 0;

    out = (char *)malloc(CURL_MAX_OUT_LENGTH_512);
    if(!out){
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    msg_url = (char *)malloc(1024);
    if(!msg_url){
		free(out);
        printf("malloc Err with %s\n",strerror(errno));
        return -1;
    }
    memset( out, 0, CURL_MAX_OUT_LENGTH_512);
    memset( msg_url, 0, 1024);
	
    snprintf( msg_url,1024-1,"%s/ipc/p2p/check-session-key?access_token=%s&lang=zh_CN&timestamp=%lld",
			VAVA_SERVER_URL,vava_login_tocken,vava_get_time_stamp() );


	if(access("/tmp/cloud_dbg",F_OK)==0)
        cloud_dbg = 1;
	oms.cur=0;
	oms.size=512;
	oms.out=out;

    snprintf( sendbuf,512-1,"{\"sessionKey\":\"%s\"}", vava_session_key );
	
    ret = https_post_request2(msg_url, sendbuf, &oms, cloud_dbg);

	if(ret == 0)
	{
		if(200 == vava_msg_parser(out, vava_get_session_check_rsp))
		{
			LOG("session check success\n");
		}
	}
	else
	{
	
	}
	
	printf("####\n %s\n",out);

    free(msg_url);
    free(out);
    return ret;//ok 0 fail -1
}

void init_vava_cloud(void)
{

	do{
		vava_login();
		if(vava_login_status) break;
		sleep(5);
	}while(!vava_login_status);

	//if(!GetDeviceMode())
	{
		vava_device_bind();
		vava_get_did();	
		vava_get_ota_fwbin();
	}
		
	return ;
}

