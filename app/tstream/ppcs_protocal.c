
/*
	文件名  ppcs_protocal.c
	描述 此文件 实现了 泽宝p2p传输协议 《IPC与APP交互协议V1.0.3.pdf》 的内容
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>  	// linux gethostbyname
#include <net/if.h>
#include <dirent.h>
#include <arpa/inet.h>	// inet_ntoa
#include <sys/reboot.h>  //just for reboot(RB_AUTOBOOT);
#include <fcntl.h>



#include <json/json.h>
#include "PPCS_API.h"
//#include "vava_header.h"

#include "media.h"
#include "ppcs_protocal.h"
#include "log.h"
#include "common_env.h"
#include "sdrecord.h"
#include "play.h"
#include "ota.h"


void Handle_IOCTRL_Cmd(int SID, int channel, char *buf, int len,int iGNo);
void system_cmd(int SID, int channel, char *buf, int iGNo);
void media_stream_cmd(int SID, int channel, char *buf, int iGNo);
void storage_cmd(int SID, int channel, char *buf,  int iGNo);
void network_cmd(int SID, int channel, char *buf, int iGNo);

static int ppcs_live_write_video_status=0;//控制KEYFAME
static sd_info_t sdinfo;
ota_info_t *ota_cmd = NULL;
extern bool g_appOtaFlag;
extern int g_appOtaSession ;

ppcs_cmd_t ppcs_cmdfunc[] = {
    // 6.1 系统消息 命令字1~199
    //下行消息
    {VAVA_CMD_SESSION_AUTH,         system_cmd}, //会话认证
    {VAVA_CMD_BUZZER_OPEN,          system_cmd}, //开启报警语音
    {VAVA_CMD_BUZZER_CLOSE,         system_cmd}, //关闭报警语音
    {VAVA_CMD_GET_IRMODE,           system_cmd}, //获取夜视模式
    {VAVA_CMD_SET_IRMODE,           system_cmd}, //设置夜视模式
    {VAVA_CMD_GET_PIR_SENSITIVITY,  system_cmd}, //获取 PIR 灵敏度
    {VAVA_CMD_SET_SENSITIVITY,      system_cmd}, //设置 PIR 灵敏度
    {VAVA_CMD_GET_ARMINGINFO_V1,    system_cmd}, //获取布防配置信息 
    {VAVA_CMD_SET_ARMINGINFO_V1,    system_cmd}, //设置布防配置信息
    {VAVA_CMD_GET_PDPARAM,          system_cmd}, //获取人形检测参数
    {VAVA_CMD_SET_PDPARAM,          system_cmd}, //设置人形检测参数
    {VAVA_CMD_GET_FDPARAM,          system_cmd}, //获取人脸检测参数
    {VAVA_CMD_SET_FDPARAM,          system_cmd}, //设置人脸检测参数
    {VAVA_CMD_STATUSLED_GET,        system_cmd}, //获取 LED 补光灯状态
    {VAVA_CMD_STATUSLED_SET,        system_cmd}, //设置 LED 补光灯开关
    {VAVA_CMD_GET_AI_REGION,        system_cmd}, //获取自定义 AI 检测区域
    {VAVA_CMD_SET_AI_REGION,        system_cmd}, //设置自定义 AI 检测区域
    {VAVA_CMD_GET_IPC_SPEAKERVOL,   system_cmd}, //获取喇叭音量
    {VAVA_CMD_SET_IPC_SPEAKERVOL,   system_cmd}, //设置喇叭音量
    {VAVA_CMD_GET_OSD_STATUS,       system_cmd}, //获取 LOGO 水印状态
    {VAVA_CMD_SET_OSD_STATUS,       system_cmd}, //设置 LOGO 水印状态
    {VAVA_CMD_GET_ALARM_MODE,       system_cmd}, //获取报警模式
    {VAVA_CMD_SET_ALARM_MODE,       system_cmd}, //设置报警模式
    {VAVA_CMD_GET_NTPINFO,          system_cmd}, //获取时区信息
    {VAVA_CMD_SET_NTPINFO,          system_cmd}, //设置时区信息
    {VAVA_CMD_SYSTEM_NEWVESION,     system_cmd}, //升级
    {VAVA_CMD_GET_UPDATE_STATUS,    system_cmd}, //获取升级状态
    {VAVA_CMD_CAMERA_RESET,         system_cmd}, //设备端恢复默认参数
    {VAVA_CMD_PAIR_MODE,            system_cmd}, //设备端进入待绑定状态
    {VAVA_CMD_GET_LANGUAGE,         system_cmd}, //获取语言
    {VAVA_CMD_SET_LANGUAGE,         system_cmd}, //设置语言
	{VAVA_CMD_SET_CAMERA_ENABLE,    system_cmd},  //设置摄像头状态 开关
	{VAVA_CMD_GET_MD_SENSITIVITY,   system_cmd},  //获取移动侦测灵敏度
	{VAVA_CMD_SET_MD_SENSITIVITY,   system_cmd},  //设置移动侦测灵敏度
	{VAVA_CMD_GET_RECORD_MODE,      system_cmd},   //获取录像形式
	{VAVA_CMD_SET_RECORD_MODE,      system_cmd},   //设置录像形式 
	{VAVA_CMD_GET_DETECT_MODE,      system_cmd},   //获取侦测类型
	{VAVA_CMD_SET_DETECT_MODE,      system_cmd},   //设置侦测类型
	{VAVA_CMD_SET_SOUND_DETECTION,  system_cmd},  // 设置声音侦测状态
	{VAVA_CMD_SET_DETECTION_TRACK,  system_cmd},  // 设置人形/移动追踪
	{VAVA_CMD_CAMERA_RESTART,       system_cmd},    //获取 SD 卡信息
	{VAVA_CMD_CAMERAINFO,           system_cmd},    //获取摄像机信息
	
    // 6.2 流媒体消息 命令字200~299
	//下行消息
	{VAVA_CMD_OPENVIDEO ,        media_stream_cmd}, //开启视频流
	{VAVA_CMD_CLOSEVIDEO ,       media_stream_cmd}, //关闭视频流
	{VAVA_CMD_AUDIO_OPEN ,       media_stream_cmd}, //开启音频流
	{VAVA_CMD_AUDIO_CLOSE,       media_stream_cmd}, //关闭音频流
	{VAVA_CMD_TALK_OPEN ,        media_stream_cmd}, //开启对讲
	{VAVA_CMD_TALK_CLOSE ,       media_stream_cmd}, //关闭对讲
	{VAVA_CMD_MIC_OPEN ,         media_stream_cmd}, //设备端麦克风开启
	{VAVA_CMD_MIC_CLOSE ,        media_stream_cmd}, //设备端麦克风关闭
	{VAVA_CMD_SPEAKER_OPEN ,     media_stream_cmd}, //设备端喇叭开启
	{VAVA_CMD_SPEAKER_CLOSE ,    media_stream_cmd}, //设备端喇叭关闭
	{VAVA_CMD_GET_VIDEO_QUALITY, media_stream_cmd}, //获取视频质量
	{VAVA_CMD_SET_VIDEO_QUALITY, media_stream_cmd}, //设置视频质量
	{VAVA_CMD_GET_MIRRORMODE ,   media_stream_cmd}, //获取视频镜像
	{VAVA_CMD_SET_MIRRORMODE ,   media_stream_cmd}, //设置视频镜像
	{VAVA_CMD_GET_VIDIOCODE ,    media_stream_cmd}, //获取视频编码格式
	{VAVA_CMD_SET_VIDIOCODE ,    media_stream_cmd}, //设置视频编码格式
	{VAVA_CMD_SNAPSHOT ,         media_stream_cmd}, //抓图

    // 6.3 录像、存储消息 命令字300~399	
    //下行消息
    {VAVA_CMD_GET_RECMODE,          storage_cmd}, //获取录像模式
    {VAVA_CMD_SET_RECMODE,          storage_cmd}, //设置录像模式
    {VAVA_CMD_GET_REC_QUALITY,      storage_cmd}, //获取录像质量
    {VAVA_CMD_SET_REC_QUALITY,      storage_cmd}, //设置录像质量
    {VAVA_CMD_GET_RECTIMER,         storage_cmd}, //获取定时录像的时间段
    {VAVA_CMD_SET_RECTIMER,         storage_cmd}, //设置定时录像的时间段
    {VAVA_CMD_GET_RECTIME,          storage_cmd}, //获取报警录像时长
    {VAVA_CMD_SET_RECTIME,          storage_cmd}, //设置报警录像时长
    {VAVA_CMD_RECORDDATE_SEARCH,    storage_cmd}, //搜索存在录像的日期
    {VAVA_CMD_RECORDLIST_SEARCH,    storage_cmd}, //搜索指定日期的录像
    {VAVA_CMD_RECORD_PLAY_BYTIME,   storage_cmd}, //按时间段进行录像回放
    {VAVA_CMD_RECORD_PLAY_CTRL,     storage_cmd}, //回放控制
    {VAVA_CMD_RECORD_IMG,           storage_cmd}, //获取录像 I 帧数据或抓图图片
    {VAVA_CMD_RECORD_IMG_STOP,      storage_cmd}, //停止处理录像 I 帧或抓图图片的传输
    {VAVA_CMD_RECORD_DEL,           storage_cmd}, //删除录像文件
    {VAVA_CMD_EXPORT_TIME_LAPSE,    storage_cmd}, //导出缩时录影
    {VAVA_CMD_SDINFO,               storage_cmd}, //获取 SD 卡信息
    {VAVA_CMD_FORMAT_SDCARD,        storage_cmd}, //格式化 SD 卡
    {VAVA_CMD_POP_SDCARD,           storage_cmd}, //弹出 SD 卡
    {VAVA_CMD_PLAY_SINGLE_FILE,     storage_cmd}, //单个录像回放基站TF卡种存储的录像，支持回放模式和文件传输模式
    // 6.4 网络消息 命令字400~499	
	//下行消息
    {VAVA_CMD_WIFI_SCAN_AP,         network_cmd}, //获取 wifi 搜索热点的结果
    {VAVA_CMD_WIFI_CONNECT_AP,      network_cmd}, //设置 wifi 连接热点的信息
    {VAVA_CMD_GET_WIFI_STATUS,      network_cmd}, //获取 wifi 连接状态
    {VAVA_CMD_GET_NASSERVER,        network_cmd}, //获取 NAS 服务器信息
    {VAVA_CMD_SET_NASSERVER,        network_cmd}  //设置 NAS 服务器信息  
};

void dump_VAVA_MSG_HEADER_AV_STREAM(VAVA_MSG_HEADER_AV_STREAM *header)
{
	if(!header)
		return ;
    UINT32 tag;         // 同步头 实时视频流同步头 0x1D000002;实时音频流同步头 0x1D000003;对讲音频流同步头 0x1D000004;回放视频同步头 0x1D000005;回放音频同频头 0x1D000006;
    UINT8  encodeType;  // 消息内容长度 视频编码类型 0:H264 1:H265;音频编码类型 0:PCM 1:G711A 2:G711U 3:AAC 4:OPUS
    UINT8  frameType;   // 帧类型 0:P帧 1:I帧 8:音频帧
    UINT8  frameRate;   // 帧率 1~120
    UINT8  res;         // 分辨率 仅视频有效 VAVA_VIDEO_RESOLUTION
    UINT32 size;        // 帧数据长度 仅音视频帧数据的长度，不包含消息头
    UINT32 ntsampSec;   // 时间戳 秒
    UINT32 ntsampUsec;  // 时间戳 毫秒，注意:秒+毫秒才是一个完整的毫 秒级时间戳
    UINT32 frameNum;    // 帧序号 从0开始累加	
	printf("======= dump dump_VAVA_MSG_HEADER_AV_STREAM\n");
	printf("tag: 0x%X\n",header->tag);
	printf("encodeType: %d\n",header->encodeType);
	printf("frameType: %d\n",header->frameType);
	printf("frameRate: %d\n",header->frameRate);
	printf("res: %d\n",header->res);
	printf("size: %d\n",header->size);
	printf("ntsampSec: %d\n",header->ntsampSec);	
	printf("ntsampUsec: %d\n",header->ntsampUsec);		
	printf("frameNum: %d\n",header->frameNum);		
	return ;
}

int ysx_json_get_str(json_object *obj, const char *key, char *val, int len)
{
    json_object *param = NULL;

    param = json_object_object_get(obj, key);
    if(param) {
        strncpy(val, (char *)json_object_get_string(param), len-1);
        return 0;
    }
    return -1;
}

int ysx_json_get_int(json_object *obj, const char *key, int *val)
{
    json_object *param = NULL;

    param = json_object_object_get(obj, key);
    if(param) {
        *val = json_object_get_int(param);
        return 0;
    }
    return -1;
}


int parse_system_cmd_json(char *in_buf, VAVA_SYSTEM_CMD type, void *system_data)
{
    int ret = 0;
    json_object *obj = NULL;

    if(!in_buf)
        return -1;

    if(!system_data)
        return -1;

    obj = json_tokener_parse(in_buf);
    if(!obj){
        LOG("system_cmd_str parse failed\n");
        ret = -1;
        goto exit;        
    }  

    switch(type)
    {
        case VAVA_CMD_SESSION_AUTH:
        {
            session_auth_t *auth_data = (session_auth_t *)system_data;  

            if(ysx_json_get_int(obj, "random", &auth_data->random)){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "auth", auth_data->auth, sizeof(auth_data->auth))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "key", auth_data->key, sizeof(auth_data->key))){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_BUZZER_OPEN:
        {
            int *buzzer_type = (int *)system_data;

            if(ysx_json_get_int(obj, "type", buzzer_type)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_IRMODE:
        {
            int *ir_mode = (int *)system_data; 

            if(ysx_json_get_int(obj, "irmode", ir_mode)){
                ret = -1;
                break;
            }  
        }
        break;
        case VAVA_CMD_SET_SENSITIVITY:
        {
            int *pir_sensitivity = (int *)system_data;

            if(ysx_json_get_int(obj, "sensitivity", pir_sensitivity)){
                ret = -1;
                break;
            } 
        }
        break;
        case VAVA_CMD_SET_ARMINGINFO_V1:
        {
            int i = 0;
            json_object *obj_get = NULL;
            json_object *obj_array = NULL;
        
            arminginfo_t *arming_info = (arminginfo_t *)system_data;   

            memset(arming_info, 0, sizeof(arminginfo_t));

            if(ysx_json_get_int(obj, "type", &arming_info->type)){ //type值不同，布防列表参数是否会有不同？
                ret = -1;
                break;
            }  

            obj_get = json_object_object_get(obj, "arminglist");
            if(!obj_get){
                ret = -1;
                break;
            }  

            //解析 arminglist
            while(1){
                if(i >= 10) //最大支持10条布防配置列表
                    break;
                    
                obj_array = json_object_array_get_idx(obj_get, i);
                if(!obj_array) //列表数据没有了，就跳出循环
                    break;

                if(ysx_json_get_int(obj_array, "enable", &arming_info->arming_list[i].enable)){ 
                    ret = -1;
                    break;
                }

                if(ysx_json_get_str(obj_array, "starttime", arming_info->arming_list[i].start_time, sizeof(arming_info->arming_list[i].start_time))){
                    ret = -1;
                    break;
                }

                
                if(ysx_json_get_str(obj_array, "endtime", arming_info->arming_list[i].end_time, sizeof(arming_info->arming_list[i].end_time))){
                    ret = -1;
                    break;
                }

                if(ysx_json_get_str(obj_array, "days", arming_info->arming_list[i].days, sizeof(arming_info->arming_list[i].days))){
                    ret = -1;
                    break;
                }

                i++;
            }
        }
        break;
        case VAVA_CMD_SET_PDPARAM:
        {
            pdparam_t *pd_param = (pdparam_t *)system_data;

            if(ysx_json_get_int(obj, "enable", &pd_param->enable)){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "rect", &pd_param->rect)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_FDPARAM:
        {
            fdparam_t *fd_param = (fdparam_t *)system_data;

            if(ysx_json_get_int(obj, "enable", &fd_param->enable)){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "rect", &fd_param->rect)){
                ret = -1;
                break;
            }          
        }
        break;
        case VAVA_CMD_STATUSLED_SET:
        {
            int *led_status = (int *)system_data;

            if(ysx_json_get_int(obj, "ctrl", led_status)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_AI_REGION:
        {
            ai_region_t *region = (ai_region_t *)system_data;

            if(ysx_json_get_int(obj, "start_x", &region->start_x)){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "end_x", &region->end_x)){
                ret = -1;
                break;
            }
            
            if(ysx_json_get_int(obj, "start_y", &region->start_y)){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "end_y", &region->end_y)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_IPC_SPEAKERVOL:
        {
            int *volume = (int *)system_data;

            if(ysx_json_get_int(obj, "vol", volume)){
                ret = -1;
                break;
            }            
        }
        break;
        case VAVA_CMD_SET_OSD_STATUS:
        {
            int *osd_status = (int *)system_data;

            if(ysx_json_get_int(obj, "status", osd_status)){
                ret = -1;
                break;
            } 
        }
        break;
        case VAVA_CMD_SET_ALARM_MODE:
        {
            alarm_mode_t *alarm = (alarm_mode_t *)system_data; 

            memset(alarm, 0, sizeof(alarm_mode_t));

            if(ysx_json_get_int(obj, "alarmmode", &alarm->mode)){
                ret = -1;
                break;
            } 

            if(alarm->mode == 2){
                if(ysx_json_get_int(obj, "pirsens", &alarm->pirsens)){
                    ret = -1;
                    break;
                } 

                if(ysx_json_get_int(obj, "pirwait", &alarm->pirwait)){
                    ret = -1;
                    break;
                } 

                if(ysx_json_get_int(obj, "rectime", &alarm->rectime)){
                    ret = -1;
                    break;
                } 
            } 
        }
        break;
        case VAVA_CMD_SET_NTPINFO:
        {
            ntpinfo_t *ntp = (ntpinfo_t *)system_data;   

            memset(ntp, 0, sizeof(ntpinfo_t));

            if(ysx_json_get_str(obj, "region", ntp->region, sizeof(ntp->region))){
                ret = -1;
                break;
            }
            
            if(ysx_json_get_str(obj, "data", ntp->data, sizeof(ntp->data))){
                ret = -1;
                break;
            }             
        }
        break;
        case VAVA_CMD_SYSTEM_NEWVESION:
        {
            ota_cmd = (ota_info_t *)system_data;

            memset(ota_cmd, 0, sizeof(ota_info_t));

            if(ysx_json_get_str(obj, "url", ota_cmd->url, sizeof(ota_cmd->url))){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "type", &ota_cmd->type)){
                ret = -1;
                break;
            } 

            if(ysx_json_get_str(obj, "otaver", ota_cmd->otaver, sizeof(ota_cmd->otaver))){
                ret = -1;
                break;
            } 
			
			if(ota_cmd->type == VAVA_UPDATE_TYPE_CAMERA)
			{
				ret = 0;
				LOG("ret = %d, url :%s ,type : %d, ver : %s \n", ret, ota_cmd->url, ota_cmd->type , ota_cmd->otaver);				
			}
			else
			{
				ret = -1;
			}
        }
        break;
		case VAVA_CMD_GET_UPDATE_STATUS:
		{
			
	
		}
		break;
        case VAVA_CMD_SET_LANGUAGE:
        {
            int *language = (int *)system_data;

            if(ysx_json_get_int(obj, "language", language)){
                ret = -1;
                break;
            }          
        }
        break;
		case VAVA_CMD_SET_CAMERA_ENABLE://设置摄像头状态 开关
		{
            int *enable = (int *)system_data;

            if(ysx_json_get_int(obj, "enable", enable)){
                ret = -1;
                break;
            }          
		}
		break;
		case VAVA_CMD_SET_MD_SENSITIVITY: //设置移动侦测灵敏度
		{
		    int *md_sensitivity = (int *)system_data;

            if(ysx_json_get_int(obj, "mdsensitivity", md_sensitivity)){
                ret = -1;
                break;
            }
        }
		break;
		case VAVA_CMD_SET_DETECT_MODE: //设置侦测类型
		{
			int *detect_mode = (int *)system_data;

            if(ysx_json_get_int(obj, "mode", detect_mode)){
                ret = -1;
                break;
            }
		}
		break;
		case VAVA_CMD_SET_SOUND_DETECTION:// 设置声音侦测状态
		{
			int *status = (int *)system_data;

            if(ysx_json_get_int(obj, "enable", status)){
                ret = -1;
                break;
            }
		}
		break;
		case VAVA_CMD_SET_DETECTION_TRACK:// 设置人形/移动追踪
		{
			int *status = (int *)system_data;

            if(ysx_json_get_int(obj, "enable", status)){
                ret = -1;
                break;
            }
		}
		break;	
		        
        default:
            LOG("type [%d] not support\n",type);
        break;
    }

exit:
    if(obj)
        json_object_put(obj);	
        
    return ret;    
}

int parse_media_stream_cmd_json(char *in_buf, VAVA_MEDIA_CMD type, void *media_data)
{
    int ret = 0;
    json_object *obj = NULL;

    if(!in_buf)
        return -1;

    if(!media_data)
        return -1;

    obj = json_tokener_parse(in_buf);
    if(!obj){
        LOG("media_stream_str parse failed\n");
        ret = -1;
        goto exit;        
    }

    switch(type)
    {
        case VAVA_CMD_SET_VIDEO_QUALITY:
        {
            int *quality_tmp = (int *)media_data;

            if(ysx_json_get_int(obj, "quality", quality_tmp)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_MIRRORMODE:
        {
            int *mirrormode_tmp = (int *)media_data;

            if(ysx_json_get_int(obj, "mirrormode", mirrormode_tmp)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_VIDIOCODE:
        {
            int *videocode_tmp = (int *)media_data;

            if(ysx_json_get_int(obj, "videocode", videocode_tmp)){
                ret = -1;
                break;
            }
        }
        break;
        default:
            LOG("type [%d] not support\n",type);
        break;
    }

exit:
    if(obj)
        json_object_put(obj);	
        
    return ret;
}

int parse_storage_cmd_json(char *in_buf, VAVA_STORAGE_CMD type, void *storage_data)
{
    int ret = 0;
    json_object *obj = NULL;

	
    if(!in_buf)
        return -1;

    if(!storage_data)
        return -1;

    obj = json_tokener_parse(in_buf);
    if(!obj){
        LOG("storage_cmd_str parse failed\n");
        ret = -1;
        goto exit;        
    }

	
    switch(type)
    {
        case VAVA_CMD_SET_RECMODE:
        {
            record_mode_t *rec_mode = (record_mode_t *)storage_data;

            if(ysx_json_get_int(obj, "recmode", (int *)&rec_mode->r_mod)){
                ret = -1;
                break;
            }         
			memset(rec_mode->starttime, 0, sizeof(rec_mode->starttime));
            if(  ysx_json_get_str(obj, "starttime", rec_mode->starttime,sizeof(rec_mode->starttime))  ){
                ret = -1;
                break;
            }         
			memset(rec_mode->endtime, 0, sizeof(rec_mode->endtime));
            if( ysx_json_get_str(obj, "endtime", rec_mode->endtime,sizeof(rec_mode->endtime)) ){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_REC_QUALITY:
        {
            int *rec_quality = (int *)storage_data;

            if(ysx_json_get_int(obj, "quality", rec_quality)){
                ret = -1;
                break;
            }  
        }
        break;
        case VAVA_CMD_SET_RECTIMER:
        {
            rectimer_t *rec_time = (rectimer_t *)storage_data;

            if(ysx_json_get_int(obj, "week", &rec_time->week)){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "time", &rec_time->time)){
                ret = -1;
                break;
            } 
        }
        break;
        case VAVA_CMD_SET_RECTIME:
        {
            int *alarm_rec_time = (int *)storage_data;

            if(ysx_json_get_int(obj, "alarmrecord", alarm_rec_time)){
                ret = -1;
                break;
            } 
        }
        break;
        case VAVA_CMD_RECORDDATE_SEARCH:
        {
            int *rec_type = (int *)storage_data;

            if(ysx_json_get_int(obj, "type", rec_type)){
                ret = -1;
                break;
            } 
        }
        break;
        case VAVA_CMD_RECORDLIST_SEARCH:
        {
            recdate_t *rec_date = (recdate_t *)storage_data;

            memset(rec_date, 0, sizeof(recdate_t));

            if(ysx_json_get_str(obj, "date", rec_date->date, sizeof(rec_date->date))){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "type", &rec_date->type)){
                ret = -1;
                break;
            } 
        }
        break;
        case VAVA_CMD_RECORD_PLAY_BYTIME:
        {
            recode_play_bytime_t *rec_bytime = (recode_play_bytime_t *)storage_data;

            memset(rec_bytime, 0, sizeof(recode_play_bytime_t));

            if(ysx_json_get_str(obj, "dirname", rec_bytime->dirname, sizeof(rec_bytime->dirname))){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "type", &rec_bytime->type)){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "rectype", &rec_bytime->rectype)){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "starttime", rec_bytime->starttime, sizeof(rec_bytime->starttime))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "endtime", rec_bytime->endtime, sizeof(rec_bytime->endtime))){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_RECORD_PLAY_CTRL:
        {
            record_play_ctrl_t *rec_ctrl = (record_play_ctrl_t *)storage_data;

            if(ysx_json_get_int(obj, "token", &rec_ctrl->token)){
                ret = -1;
                break;
            }
            
            if(ysx_json_get_int(obj, "ctrl", &rec_ctrl->ctrl)){
                ret = -1;
                break;
            }
        }
        break;
        case  VAVA_CMD_RECORD_IMG:
        {
            record_img_t *rec_img = (record_img_t *)storage_data;

            memset(rec_img, 0, sizeof(record_img_t));

            if(ysx_json_get_str(obj, "dirname", rec_img->dirname, sizeof(rec_img->dirname))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "filename", rec_img->filename, sizeof(rec_img->filename))){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_RECORD_DEL:
        {

        }
        break;
        case VAVA_CMD_EXPORT_TIME_LAPSE:
        {
            time_lapse_t *lapse = (time_lapse_t *)storage_data;

            memset(lapse, 0, sizeof(time_lapse_t));

            if(ysx_json_get_str(obj, "dirname", lapse->dirname, sizeof(lapse->dirname))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "starttime", lapse->starttime, sizeof(lapse->starttime))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "endtime", lapse->endtime, sizeof(lapse->endtime))){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "lastsec", &lapse->lastsec)){
                ret = -1;
                break;
            }
        }
        break;
		case VAVA_CMD_PLAY_SINGLE_FILE:
		{
			play_single_file_t *psf = (play_single_file_t *)storage_data;
            memset(psf, 0, sizeof(play_single_file_t));

            if(ysx_json_get_str(obj, "dirname", psf->dirname, sizeof(psf->dirname))){
                ret = -1;
                break;
            }
            if(ysx_json_get_str(obj, "filename", psf->filename, sizeof(psf->filename))){
                ret = -1;
                break;
            }
            if(ysx_json_get_int(obj, "type", (int *)&psf->type)){
                ret = -1;
                break;
            }
		}
		break;

        default:
            LOG("type [%d] not support\n",type);
        break;
    }
    
exit:
    if(obj)
        json_object_put(obj);   
        
    return ret;
}

int parse_network_cmd_json(char *in_buf, VAVA_NETWORK_CMD type, void *network_data)
{
    int ret = 0;
    json_object *obj = NULL;

    if(!in_buf)
        return -1;

    if(!network_data)
        return -1;

    obj = json_tokener_parse(in_buf);
    if(!obj){
        LOG("network_cmd_str parse failed\n");
        ret = -1;
        goto exit;        
    }

    switch(type)
    {
        case VAVA_CMD_WIFI_CONNECT_AP:
        {
            connect_ap_t *connect = (connect_ap_t *)network_data;

            memset(connect, 0, sizeof(connect_ap_t));

            if(ysx_json_get_str(obj, "ssid", connect->ssid, sizeof(connect->ssid))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "password", connect->password, sizeof(connect->password))){
                ret = -1;
                break;
            }

            if(ysx_json_get_int(obj, "encrypt", &connect->encrypt)){
                ret = -1;
                break;
            }
        }
        break;
        case VAVA_CMD_SET_NASSERVER:
        {
            nas_server_t *nas_s = (nas_server_t *)network_data;
            
            memset(nas_s, 0, sizeof(nas_server_t));

            if(ysx_json_get_int(obj, "ctrl", &nas_s->ctrl)){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "ip", nas_s->ip, sizeof(nas_s->ip))){
                ret = -1;
                break;
            }

            if(ysx_json_get_str(obj, "path", nas_s->path, sizeof(nas_s->path))){
                ret = -1;
                break;
            }
        }
        break;
        
        default:
            LOG("type [%d] not support\n",type);
        break;
    }

exit:
    if(obj)
        json_object_put(obj);   
        
    return ret;

}

/*json打包函数*/
int package_network_cmd_json(void *in_buf, VAVA_SYSTEM_CMD type, void *system_data)
{
    int i = 0;

    if(!in_buf)
        return -1;

    if(!system_data)
        return -1;


    switch(type)
    {
        case VAVA_CMD_WIFI_SCAN_AP:
        {
            /*
            {"result":"ok","apnum":24,"endflag":0,"aplist":
            [{"ssid":"YSX-2.4G-008","signal":0,"encrypt":1}]}
            */
            char *obj = (char *)system_data;
            scan_ap_t *ap = (scan_ap_t*)in_buf;
			int left_num;
            struct json_object *obj_all = json_object_new_object();
            struct json_object *obj_aplist = json_object_new_array();

            json_object_object_add(obj_all, "result", json_object_new_string(ap->result));
			if( (ap->apnum - ap->curnum) > 50 )//剩余的大于50个
			{
	            json_object_object_add(obj_all, "apnum", json_object_new_int(50));
	            json_object_object_add(obj_all, "endflag", json_object_new_int(0));
				left_num = 50;//发送50个
			}
			else//最后一包
			{
	            json_object_object_add(obj_all, "apnum", json_object_new_int(ap->apnum-ap->curnum));
	            json_object_object_add(obj_all, "endflag", json_object_new_int(1));		
				left_num = ap->apnum - ap->curnum; //发送剩余的
			}
            for (i = ap->curnum; i < ap->curnum+left_num; i++)
            {
                struct json_object *obj_cell = json_object_new_object();
                json_object_object_add(obj_cell, "ssid", json_object_new_string(ap->aplist[i].ssid));
                json_object_object_add(obj_cell, "signal", json_object_new_int(ap->aplist[i].signal));
                json_object_object_add(obj_cell, "encrypt", json_object_new_int(ap->aplist[i].encrypt));
                json_object_array_add(obj_aplist, obj_cell);
            }
            json_object_object_add(obj_all, "aplist", obj_aplist);

            strcpy(obj,json_object_to_json_string(obj_all));
			
            #if 0
            for (i = 0; i < ap->apnum; i++)
            {
                printf("%16s\t",ap->aplist[i].ssid);
                printf("%10d",ap->aplist[i].encrypt);
                printf("%10d\n",ap->aplist[i].signal);
            }
            printf("\n-------------------------------------------\n\n");
            printf(" obj:\n%s\n",obj);
            #endif
            json_object_put(obj_all);
            
        }
    }	
        
    return 1;  
}


//删除录像文件操作
int record_del_handle(char *in_buf)
{
    int ret = 0;
    int i=0, j= 0;
    recode_del_t recfile_del;
    char file[16] = {0};
    json_object *obj = NULL;
    json_object *obj_get = NULL;
    json_object *obj_array = NULL;
    json_object *array_data = NULL;
    json_object *obj_sub_array = NULL;

    if(!in_buf)
        return -1;

    obj = json_tokener_parse(in_buf);
    if(!obj){
        LOG("parse record_del_str failed\n");
        ret = -1;
        goto exit;        
    }   

    memset(&recfile_del, 0, sizeof(recode_del_t));                    

    obj_get = json_object_object_get(obj, "dirlist");
    if(!obj_get){
        ret = -1;
        goto exit ;
    }  

    //解析 dirlist
    while(1){            
        obj_array = json_object_array_get_idx(obj_get, i);
        if(!obj_array) //列表数据没有了，就跳出循环
            break;

        if(ysx_json_get_str(obj_array, "dirname", recfile_del.dirname, sizeof(recfile_del.dirname))){
            ret = -1;
            break;
        }

        if(ysx_json_get_int(obj_array, "flag", &recfile_del.flag)){
            ret = -1;
            break;
        }
        
        if(recfile_del.flag){ 
            //删除文件夹
            
            //清空结构体 recfile_del
            memset(&recfile_del, 0, sizeof(recode_del_t));  
        }else{
            array_data = json_object_object_get(obj_array, "filelist");
            if(!array_data){
                ret = -1;
                break;
            }

            //解析 filelist
            while(1){
                obj_sub_array = json_object_array_get_idx(array_data, j);
                if(!obj_sub_array){ //列表数据没有了，就跳出循环
                    j = 0;
                    break;
                } 

                if(ysx_json_get_str(obj_sub_array, "filename", file, sizeof(file))){
                    ret = -1;
                    break;
                }

                //删除文件
                
                //清空字符数组 file
                memset(file, 0, sizeof(file));  
                                
                j++;
            }
        }

        i++;
    }

exit:
    if(obj)
        json_object_put(obj);   
        
    return ret;
}


void Handle_IOCTRL_Cmd(int SID, int channel, char *buf, int len, int iGNo)
{
	int type;
	int index=0;
	unsigned int i=0;
	VAVA_MSG_HEADER_COMMAND *header;
	char t_buf[4096];

	while( len - index > sizeof(VAVA_MSG_HEADER_COMMAND) )//命令存在粘连 可能一次readbuf会来几个命令，但是命令不会分成几个包
	{
		
		header = (VAVA_MSG_HEADER_COMMAND *)(buf+index);
		type = header->cmd;
	    LOG("Handle_IOCTRL_Cmd type %d iGNo %d\n", type, iGNo);
		
		memset(t_buf, 0, sizeof(t_buf));
		if(sizeof(VAVA_MSG_HEADER_COMMAND)+header->cmdLength > sizeof(t_buf))
		{
			LOG("t_buf is too short\n");
			return ;
		}
		memcpy(t_buf, buf+index, sizeof(VAVA_MSG_HEADER_COMMAND)+header->cmdLength);
	    for(i=0; i<sizeof(ppcs_cmdfunc)/sizeof(ppcs_cmd_t); i++) {
	        if(ppcs_cmdfunc[i].type == type) {
	            (*ppcs_cmdfunc[i].cfunc)(SID, channel, t_buf, iGNo);
	            break ;
	        }
	    }
	    index = index+sizeof(VAVA_MSG_HEADER_COMMAND)+header->cmdLength;
		if(i == sizeof(ppcs_cmdfunc)/sizeof(ppcs_cmd_t))
		{
			LOG("err cmd type %d\n",type);
			return ;
		}
	    
	}
    return;
}

VAVA_VIDEO_QUALITY ppcs_proto_get_current_res(void)
{
	//如果处于自动模式下 需要返回对应auto 模式的 分辨率
	if( GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_AUTO)
	{
		if(g_media_struct.resolution == YSX_RESOLUTION_HIGH)
			return (UINT8)VAVA_VIDEO_QUALITY_AUTO_2K;
		else if(g_media_struct.resolution == YSX_RESOLUTION_MID)
			return (UINT8)VAVA_VIDEO_QUALITY_AUTO_HIGH;
		else
			return (UINT8)VAVA_VIDEO_QUALITY_AUTO_RENEWAL;
	}

	return (UINT8)GetYSXCtl(YSX_VQUALITY);
}


//发送视频 空帧
void ppcs_proto_send_meta_video_frame(int tag,int ch,const int sid)
{
    struct timeval tv;
	char data[sizeof(VAVA_MSG_HEADER_AV_STREAM)+1] = {0};
	VAVA_MSG_HEADER_AV_STREAM *header;

    //gettimeofday(&tv, NULL);

	header = (VAVA_MSG_HEADER_AV_STREAM *)data;
	
	header->tag = tag;
	header->frameRate = 15;
	header->frameType = 1;
	header->encodeType = 0;
	header->frameNum = 0 ;// 每个用户 都要设置 从0开始

	header->ntsampSec =  0; //tv.tv_sec;
	header->ntsampUsec = 0; //tv.tv_usec/1000;
	header->size = 1;//需要给个非0的数据
	header->res = ppcs_proto_get_current_res();
						
	ppcs_service_write(sid, ch, data, sizeof(data));
	
	return ;
}


//重要：data 包含了header的内容 
int ppcs_proto_send_video(int tag, int ch,const int sid, const void *data, const int len, const struct timeval tv, const int keyframe, int frame_num)
{
	const char *video_buf = NULL;
	VAVA_MSG_HEADER_AV_STREAM *header = NULL;
	
	video_buf = data;
	if(video_buf == NULL)
	{
		printf("video_buf is null \n");
		return ;
	}
	
	header = (VAVA_MSG_HEADER_AV_STREAM *)video_buf;
	header->tag = tag;
	header->frameRate = 15;
	header->frameType = keyframe;
	header->encodeType = 0;
	header->frameNum = frame_num ;// 每个用户 都要设置 从1开始 ？

	header->ntsampSec =  tv.tv_sec;
	header->ntsampUsec = tv.tv_usec/1000;
	//printf("header->ntsampSec %d header->ntsampUsec %d\n",header->ntsampSec,header->ntsampUsec);
	header->size = len;
	header->res = ppcs_proto_get_current_res();
						
	return 	ppcs_service_write(sid, ch, data, len+sizeof(VAVA_MSG_HEADER_AV_STREAM));

}


//重要：data 包含了header的内容 
void ppcs_proto_send_audio(int tag,int ch,const int sid, const void *data, const int len, const struct timeval tv, int frame_num)
{
	const char *video_buf = NULL;
	VAVA_MSG_HEADER_AV_STREAM *header = NULL;
	int ret;
	
	video_buf = data;
	if(video_buf == NULL)
	{
		printf("video_buf is null \n");
		return ;
	}
	
	header = (VAVA_MSG_HEADER_AV_STREAM *)video_buf;
	header->tag = tag;
	header->frameRate = 15;
	header->frameType = 8;
	header->encodeType = VAVA_AUDIO_CODEC_AAC;
	header->frameNum = frame_num ;// 每个用户 都要设置 从0开始 ？

	header->ntsampSec =  tv.tv_sec;
	header->ntsampUsec = tv.tv_usec/1000;
	//printf("header->ntsampSec %d header->ntsampUsec %d\n",header->ntsampSec,header->ntsampUsec);
	header->size = len;
	header->res = 0;
						
	ret = ppcs_service_write(sid, ch, data, len+sizeof(VAVA_MSG_HEADER_AV_STREAM));
	//if(ret != 0)
	//	LOG("ppcs_proto_send_audio ERROR=%d\n",ret);
	
	return ;
}

//重要：data 包含了header的内容 
int ppcs_proto_recv_audio(const int SessionID,const int Channel,const void *data,const int len)
{
	int ret;
	VAVA_MSG_HEADER_AV_STREAM *header = NULL;
	header = (VAVA_MSG_HEADER_AV_STREAM *)data;

	ret = ppcs_service_read(SessionID, Channel, data,  sizeof(VAVA_MSG_HEADER_AV_STREAM));//先读取头部到data
	
	if(ret>0)
	{
		dump_VAVA_MSG_HEADER_AV_STREAM(header);
		ppcs_service_read(SessionID, Channel, data+sizeof(VAVA_MSG_HEADER_AV_STREAM),  header->size);// 一定能读到
	}
	if(header->tag != VAVA_TALK_AUDIO_TAG)
		return 0;
	
	return header->size; 
}


//命令 608    升级状态改变
void ppcs_proto_send_upgrate_status(const int SID,VAVA_UPDATE_STATUS status,VAVA_UPDATE_TYPE type,int loaddata)
{
	VAVA_MSG_HEADER_COMMAND *recv_header;

    VAVA_MSG_HEADER_COMMAND *vava_msg_header;
    char payload[PAYLOAD_SIZE] = {0};
	char *resp_data;

	memset(payload,0,PAYLOAD_SIZE);
	resp_data = &payload[sizeof(VAVA_MSG_HEADER_COMMAND)];

    printf("status %d type %d loaddata %d\n",status,type,loaddata);

    vava_msg_header = (VAVA_MSG_HEADER_COMMAND *)&payload ;
    vava_msg_header->cmd = VAVA_CMD_UPGRATE_STATUS;
    vava_msg_header->tag = VAVA_CMD_TAG;
    vava_msg_header->sn = 0;

	if(status == VAVA_UPDATE_LOADING)
		snprintf(resp_data,DATA_SIZE-1, "{\"status\":%d,\"type\":%d\"loaddata\":%d}",status,type,loaddata);
	else
		snprintf(resp_data,DATA_SIZE-1, "{\"status\":%d,\"type\":%d}",status,type);
    vava_msg_header->cmdLength = strlen(resp_data);

    PPCS_Write(SID, 0, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
	
	return ;
}

void system_cmd(int SID, int channel, char *buf, int iGNo)
{
	VAVA_MSG_HEADER_COMMAND *recv_header;

    VAVA_MSG_HEADER_COMMAND *vava_msg_header;
    char payload[PAYLOAD_SIZE] = {0};
	char *resp_data;

	memset(payload,0,PAYLOAD_SIZE);
	resp_data = &payload[sizeof(VAVA_MSG_HEADER_COMMAND)];
	recv_header=(VAVA_MSG_HEADER_COMMAND *)buf;

    LOG("system_cmd type %d sn %d\n",recv_header->cmd, iGNo);

    vava_msg_header = (VAVA_MSG_HEADER_COMMAND *)&payload ;
    vava_msg_header->cmd = recv_header->cmd;
    vava_msg_header->tag = VAVA_CMD_TAG;
    vava_msg_header->sn = recv_header->sn;
	
	char *data_buf = buf + sizeof(VAVA_MSG_HEADER_COMMAND);
	if(recv_header->cmdLength)
		LOG("%s\n",data_buf);	

    switch(recv_header->cmd)
    {
        case VAVA_CMD_SESSION_AUTH:
        {
            session_auth_t auth;
			printf("VAVA_CMD_SESSION_AUTH %s\n", data_buf);

			parse_system_cmd_json(data_buf, VAVA_CMD_SESSION_AUTH, &auth);
			
            printf("%d\n%s\n%s\n", auth.random, auth.auth, auth.key);

            //后续命令字确定后，再去掉

			vava_set_session_key(auth.key);
			if(0 == vava_session_check())
			{
				LOG("VAVA_CMD_SESSION_AUTH success\n");
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"hardver\":\"%s\",\"softver\":\"%s\",\"otaver\":\"%s\"}", HARDVER, SOFTVER, OTAVER);
			}
			else
			{
				LOG("VAVA_CMD_SESSION_AUTH failed\n");
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":1}");				
			}

        }
        break;
        case VAVA_CMD_BUZZER_OPEN:
        { 
            int buzzer;
            parse_system_cmd_json(data_buf, VAVA_CMD_BUZZER_OPEN, &buzzer);
            
            if(buzzer == VAVA_BUZZER_TYPE_OPEN){

            }else if(buzzer == VAVA_BUZZER_TYPE_INTERVAL){

            }

            //开启报警语音操作...
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);

        }
        break;
        case VAVA_CMD_BUZZER_CLOSE:
        {
            //关闭报警语音操作...
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_IRMODE:
        {
            int irmode = VAVA_IRLED_MODE_CLOSE;
            //获取夜视模式...（应该只支持关闭夜视模式和黑白夜视模式）
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"irmode\":%d}", GetYSXCtl(YSX_IR_MODE) ); 
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_IRMODE:
        {
            int irmode;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_IRMODE, &irmode);

            //设置夜视模式...
            SetYSXCtl(YSX_IR_MODE,(UINT8)irmode);
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_PIR_SENSITIVITY:// 暂时没有这个需求
        {
            int sensitivity = VAVA_PIR_SENSITIVITY_LOW;
            //获取PIR灵敏度...
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"sensitivity\":%d}", sensitivity); 
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_SENSITIVITY:// 暂时没有这个需求
        {
            int sensitivity;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_SENSITIVITY, &sensitivity);

            //设置PIR灵敏度...
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);         
        }
        break;
        case VAVA_CMD_GET_ARMINGINFO_V1:// 暂时没有这个需求
        {
            arminginfo_t info;
            char tmp_str[256] = {0};
            //获取布防配置信息...
            //成功返回ok，否则返回fail
            //resp_data和tmp_str的长度应该不够
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"type\":%d,\"arminglist\":[%s]}", info.type, tmp_str);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);                   
        }
        break;
        case VAVA_CMD_SET_ARMINGINFO_V1:// 暂时没有这个需求
        {
            arminginfo_t info;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_ARMINGINFO_V1, &info);

            //设置布防配置信息...
            //成功返回ok，否则返回fail
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_PDPARAM:
        {
            pdparam_t pd;
			int enable, rect;
            //获取人形检测参数...
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"enable\":%d,\"rect\":%d}", enable, rect);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_PDPARAM:
        {
            pdparam_t pd;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_PDPARAM, &pd);

            //设置人形检测参数...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_FDPARAM:// 暂时没有这个需求
        {
            fdparam_t fd;
			int enable, rect;
            //获取人脸检测参数...
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"enable\":%d,\"rect\":%d}", enable, rect);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_FDPARAM:// 暂时没有这个需求
        {
            int enable, rect;
            //sscanf(data_buf, "{\"enable\":%d,\"rect\":%d}", &enable, &rect);

            //设置人形检测参数...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_STATUSLED_GET:
        {
            //获取LED补光灯状态...
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"ctrl\":%d}", g_enviro_struct.light_en);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_STATUSLED_SET:
        {
            VAVA_CTRL_TYPE led;
            parse_system_cmd_json(data_buf, VAVA_CMD_STATUSLED_SET, &led);
            SetYSXCtl(YSX_LIGHT_EN, (int)led);

            //设置led补光灯开关...
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_AI_REGION:
        {
            ai_region_t *region = &g_enviro_struct.detect_region;
            //获取自定义ai检测区域...
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"start_x\":%d,\"end_x\":%d,\"start_y\":%d,\"end_y\":%d}", 
                     region->start_x, region->end_x, region->start_y, region->end_y);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_AI_REGION:
        {
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_AI_REGION, &g_enviro_struct.detect_region);

            //设置ai检测区域...
            SetYSXCtlString(YSX_DETECT_REGION, data_buf, strlen(data_buf)+1);

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_IPC_SPEAKERVOL:
        {
            int volume = 0;
            //获取喇叭音量...

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"vol\":%d}", volume);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_IPC_SPEAKERVOL:
        {
            int volume;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_IPC_SPEAKERVOL, &volume);

            //设置喇叭音量...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_OSD_STATUS:
        {
            int status = CLOSE;
            //获取LOGO水印状态...

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"status\":%d}", status);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_OSD_STATUS:
        {
            int status;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_OSD_STATUS, &status);

            //设置LOGO水印状态...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_ALARM_MODE:
        {
            alarm_mode_t alarm = {0};
            //获取报警模式...
            
            if(alarm.mode != VAVA_ALARMMODE_CUSTOM){
                snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"alarmmode\":%d}", alarm.mode);
                //文档写的省电和监控是mode，而自定义是alarmmode。不知道有没有写错。
                //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);
            }else{
                snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"alarmmode\":%d,\"pirsens\":%d,\"pirwait\":%d,\"rectime\":%d}", 
                     alarm.mode, alarm.pirsens, alarm.pirwait, alarm.rectime);
                //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
            }
        }
        break;
        case VAVA_CMD_SET_ALARM_MODE:
        {
            alarm_mode_t alarm;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_ALARM_MODE, &alarm);

            //设置报警模式...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);    
        }
        break;
        case VAVA_CMD_GET_NTPINFO:
        {
            ntpinfo_t info = {0};
            //获取时区信息

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"timezoom\":%d,\"region\":\"%s\",\"data\":\"%s\"}",
                info.timezoom, info.region, info.data);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_SET_NTPINFO:
        {
            ntpinfo_t info;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_NTPINFO, &info);

            //设置时区信息...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_SYSTEM_NEWVESION:
        {
            ota_info_t info;
            //升级...

            int tmp = parse_system_cmd_json(data_buf, VAVA_CMD_SYSTEM_NEWVESION, &info);
			if(tmp == 0)
			{				
				LOG("====+++++++++++++VAVA_CMD_SYSTEM_NEWVESION success======t_version = %s======\n",g_enviro_struct.t_version);		
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"hardver\":\"%s\",\"softver\":\"%s\",\"otaver\":\"%s\"}", HARDVER, SOFTVER, g_enviro_struct.t_version);
				g_appOtaFlag = true;
				g_appOtaSession = SID;
			}
			else
			{				
				LOG("VAVA_CMD_SYSTEM_NEWVESION failed\n");
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":1}");				
			}         

            //snprintf(resp_data, DATA_SIZE, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_GET_UPDATE_STATUS:
        {
        	//获取升级状态...
            ota_status_t info;
			info.type = VAVA_UPDATE_TYPE_CAMERA;
			get_fw_version(info.otaver);
			get_update_status(info.status,info.loaddata);
            
            snprintf(resp_data, DATA_SIZE-1, "{\"status\":%d,\"type\":%d,\"loaddata\":%d,\"otaver\":\"%s\",\"result\":\"ok\"}", 
                info.status, info.type, info.loaddata, info.otaver);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_CAMERA_RESET:
        {
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            vava_msg_header->cmdLength = strlen(resp_data);
            
            memcpy(&payload[sizeof(VAVA_MSG_HEADER_COMMAND)], resp_data, vava_msg_header->cmdLength);
            
            PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);

            //reset 恢复默认值
        }
        break;
        case VAVA_CMD_PAIR_MODE:
        {
            //设备端进入待绑定状态...
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_LANGUAGE:
        {
            int language = VAVA_LANGUAGE_ENGLISH;

            //获取设备端提示音语言...
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"language\":%d}", language);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_LANGUAGE:
        {
            int language;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_LANGUAGE, &language);
            //设置设备端提示音语言...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
		case VAVA_CMD_SET_CAMERA_ENABLE://设置摄像头状态 开关
		{
			VAVA_CTRL_TYPE enable;
			parse_system_cmd_json(data_buf, VAVA_CMD_SET_CAMERA_ENABLE, &enable);
			SetYSXCtl(YSX_C_EN,enable);
			snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
		}
		break;
		case VAVA_CMD_GET_MD_SENSITIVITY: //获取移动侦测灵敏度
		{      
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"sensitivity\":%d}", g_enviro_struct.t_sense);   
		}
		break;
		case VAVA_CMD_SET_MD_SENSITIVITY: //设置移动侦测灵敏度
		{
			VAVA_MD_SENSITIVITY sensitivity;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_MD_SENSITIVITY, &sensitivity);
            //设置移动侦测灵敏度
            SetYSXCtl(YSX_MD_SENSE, (int)sensitivity);

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);   
		}
		break;
		case VAVA_CMD_GET_DETECT_MODE://获取侦测类型
		{      
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"mode\":%d}", g_enviro_struct.motion_type);
		}
		break;
		case VAVA_CMD_SET_DETECT_MODE: //设置侦测类型
		{
			VAVA_DETECT_MODE dtc_mode;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_DETECT_MODE, &dtc_mode);
            //设置侦测类型
            SetYSXCtl(YSX_MD_TYPE, (int)dtc_mode);

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);  
		}
		break;
		case VAVA_CMD_SET_SOUND_DETECTION:// 设置声音侦测状态
		{
			VAVA_CTRL_TYPE status;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_SOUND_DETECTION, &status);
            //设置声音侦测状态
            SetYSXCtl(YSX_SOUND_DETECTION, (int)status);
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"enable\":%d}", status);
		}
		break;
		case VAVA_CMD_SET_DETECTION_TRACK:// 设置人形/移动追踪
		{
			VAVA_CTRL_TYPE status;
            parse_system_cmd_json(data_buf, VAVA_CMD_SET_DETECTION_TRACK, &status);
            //设置人形/移动追踪开启和关闭状态
            SetYSXCtl(YSX_ATRACK_EN, (int)status);

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"enable\":%d}", status);
		}
		break;
		case VAVA_CMD_CAMERA_RESTART://设备重启
		{
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK); //命令发送出去，设备重启
		}
		break;
		case VAVA_CMD_CAMERAINFO://获取摄像机信息
		{
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"{");
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"micstatus\":%d,",GetYSXCtl(YSX_MIC_EN));
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"h_devicenum\":\"%s\",","HS005");
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"h_appversionout\":\"%s\",",g_enviro_struct.t_version);
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"enable\":%d,",GetYSXCtl(YSX_C_EN));
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"mirrormode\":%d,",GetYSXCtl(YSX_INVERSION));
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"irmode\":%d,",GetYSXCtl(YSX_IR_MODE));
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"mdsensitivity\":%d,",g_enviro_struct.t_sense);  //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"sn\":\"%s\",",g_enviro_struct.t_uid);
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"softver\":\"%s\",","IPC_V_2.0.0");  //fix me
			{
				char t_mac[18]={0};
				memset(t_mac, 0, sizeof(t_mac));
				get_local_mac("wlan0",t_mac);
				snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data),"\"mac\":\"%s\",",t_mac);
			}
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"sdstatus\":%d,",sdinfo.sdstatus); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"totolsize\":%lld,",sdinfo.totolsize); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"usedsize\":%lld,",sdinfo.usedsize); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"freesize\":%lld,",sdinfo.freesize); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"nasstatus\":%d,",0); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"nas_totolsize\":%d,",0); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"nas_usedsize\":%d,",0); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"nas_freesize\":%d,",0); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"nas_ip\":\"%s\",","0.0.0.0"); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"nas_path\":\"%s\",","/mynas"); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"alarmrecord\":%d,",g_enviro_struct.detection_timer); //fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"recordmode\":%d,",g_enviro_struct.record_mode.r_mod);
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"recordstarttime\":\"%s\",",g_enviro_struct.record_mode.starttime);
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"recordendtime\":\"%s\",",g_enviro_struct.record_mode.endtime);
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"ledstatus\":%d,",g_enviro_struct.light_en);// fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"detectmode\":%d,",g_enviro_struct.motion_type);// fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"sounddetection\":%d,",g_enviro_struct.sound_detection);// fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"detectiontrack\":%d,",g_enviro_struct.atrack_en);// fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"osdstatus\":%d,",1);// fix me
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"\"result\":\"%s\"","ok");// fix me
			
			snprintf(resp_data+strlen(resp_data),DATA_SIZE-strlen(resp_data)-1,"}");
		}
		break;		
		

        default:
            printf("type [%d] not support\n",vava_msg_header->cmd);
        break;
    }
    
    vava_msg_header->cmdLength = strlen(resp_data);

    PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
}

void media_stream_cmd(int SID, int channel, char *buf, int iGNo)
{
	VAVA_MSG_HEADER_COMMAND *recv_header;

    VAVA_MSG_HEADER_COMMAND *vava_msg_header;
    char payload[PAYLOAD_SIZE] = {0};
	char *resp_data;

	memset(payload,0,PAYLOAD_SIZE);
	resp_data = &payload[sizeof(VAVA_MSG_HEADER_COMMAND)];
	recv_header=(VAVA_MSG_HEADER_COMMAND *)buf;

    LOG("media_stream_cmd type %d sn %d\n",recv_header->cmd, iGNo);

    vava_msg_header = (VAVA_MSG_HEADER_COMMAND *)&payload ;
    vava_msg_header->cmd = recv_header->cmd;
    vava_msg_header->tag = VAVA_CMD_TAG;
    vava_msg_header->sn = recv_header->sn;
	
	char *data_buf = buf + sizeof(VAVA_MSG_HEADER_COMMAND);
	if(recv_header->cmdLength)
		LOG("%s\n",data_buf);
	
    switch(recv_header->cmd)
    {
        case VAVA_CMD_OPENVIDEO:
        {
			if(GetYSXCtl(YSX_C_EN)==0)//如果摄像机 没有打开 需要返回对应错误码
			{
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":,%d}",VAVA_ERR_CODE_CAMERA_DISABLE);
				break;
			}
		}
        {
            video_info_t info = {0};   
            //开启视频流...
			media_cmd_open_video(iGNo,&info);

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"videocodec\":%d,\"videores\":%d,\"videoframerate\":%d,\"videobitrate\":%d,\"videonum\":%d}",
                info.videocodec, info.videores, info.videoframerate, info.videobitrate, info.videonum);
 
        }
        {
            audio_info_t info = {0};

            //开启音频流...
			media_cmd_open_audio(iGNo,&info);
			
        }		
        break;
        case VAVA_CMD_CLOSEVIDEO:
        {
            //关闭视频流...
			media_cmd_stop_video(iGNo);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        {
            //关闭音频流...
			media_cmd_stop_audio(iGNo);
        }		
        break;
        case VAVA_CMD_AUDIO_OPEN:
        {
			if(GetYSXCtl(YSX_C_EN)==0)//如果摄像机 没有打开 需要返回对应错误码
			{
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":,%d}",VAVA_ERR_CODE_CAMERA_DISABLE);
				break;
			}
		}
        {
            audio_info_t info = {0};

            //开启音频流...
			media_cmd_open_audio(iGNo, &info);
			
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"audiocodec\":%d,\"audiorate\":%d,\"audiobitper\":%d,\"audiochannel\":%d,\"audioframerate\":%d}",
                info.audiocodec, info.audiorate, info.audiobitper, info.audiochannel, info.audioframerate);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_AUDIO_CLOSE:
        {
            //关闭音频流...
			media_cmd_stop_audio(iGNo);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_TALK_OPEN:
        {
            //开启对讲...
            if(GetYSXCtl(YSX_SPK_EN))
            {
				media_cmd_start_speak_audio(iGNo);
				snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            }
			else
			{
				snprintf(resp_data, DATA_SIZE-1, RESULT_FAIL);
			}
				
        }
        break;
        case VAVA_CMD_TALK_CLOSE:
        {
            //关闭对讲...
			media_cmd_stop_speak_audio(iGNo);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_MIC_OPEN:
        {
            //开启麦克风，默认开启...
			SetYSXCtl(YSX_MIC_EN,1);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_MIC_CLOSE:
        {
            //关闭麦克风...
			SetYSXCtl(YSX_MIC_EN,0);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SPEAKER_OPEN:
        {
            //开启喇叭...
			SetYSXCtl(YSX_SPK_EN, 1);
			media_cmd_stop_speak_audio(iGNo);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SPEAKER_CLOSE:
        {
            //关闭喇叭...
			SetYSXCtl(YSX_SPK_EN, 0);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_VIDEO_QUALITY:
        {
            VAVA_VIDEO_QUALITY quality = VAVA_VIDEO_QUALITY_RENEWAL;
            //获取视频质量...
            quality = ppcs_proto_get_current_res();
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"quality\":%d}", quality);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_VIDEO_QUALITY:
        {
            VAVA_VIDEO_QUALITY quality;
            parse_media_stream_cmd_json(data_buf, VAVA_CMD_SET_VIDEO_QUALITY, &quality);
			LOG("switch to %d\n",quality);

            //设置视频质量...
            SetYSXCtl(YSX_VQUALITY, quality);
            
			if(quality == VAVA_VIDEO_QUALITY_2K
				|| quality == VAVA_VIDEO_QUALITY_HIGH
				|| quality == VAVA_VIDEO_QUALITY_RENEWAL
				|| quality == VAVA_VIDEO_QUALITY_AUTO )
			{
				media_cmd_switch_quality(quality);
				media_cmd_switch_record_quality(quality);
            	snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
			}
			else
            	snprintf(resp_data, DATA_SIZE-1, RESULT_FAIL); 
        }
        break;
        case VAVA_CMD_GET_MIRRORMODE:
        {
            int mirrormode = VAVA_MIRROR_TYPE_NORMAL;
            //获取视频镜像...
			mirrormode = GetYSXCtl(YSX_INVERSION);
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"mirrormode\":%d}", mirrormode);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_MIRRORMODE:
        {
            int mirrormode;
            parse_media_stream_cmd_json(data_buf, VAVA_CMD_SET_MIRRORMODE, &mirrormode);

            //设置视频镜像...
			SetYSXCtl(YSX_INVERSION,mirrormode);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_VIDIOCODE:
        {
            VAVA_VIDEO_CODEC videocode = VAVA_VIDEO_CODEC_H264;
            //获取视频编码格式...
			
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"videocode\":%d}", videocode);
        }
        break;
        case VAVA_CMD_SET_VIDIOCODE:
        {
            int videocode;
            parse_media_stream_cmd_json(data_buf, VAVA_CMD_SET_VIDIOCODE, &videocode);

            //设置视频编码格式...

            snprintf(resp_data, DATA_SIZE-1, RESULT_FAIL);//暂时不支持 修改视频编码格式 返回错误
        }
        break;
        case VAVA_CMD_SNAPSHOT:
        {
            //开启抓图...
			Md2Snapshot(FILE_SNAP_JPEG);
			//fix me , send jpeg to vava cloud here
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
        }
        break;
        default:
            LOG("type [%d] not support\n",vava_msg_header->cmd);
        break;        
    }

    vava_msg_header->cmdLength = strlen(resp_data);

    PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
}


void storage_cmd(int SID, int channel, char *buf,  int iGNo)
{
	VAVA_MSG_HEADER_COMMAND *recv_header;

    VAVA_MSG_HEADER_COMMAND *vava_msg_header;
    char payload[PAYLOAD_SIZE] = {0};
	char *resp_data;

	memset(payload,0,PAYLOAD_SIZE);
	resp_data = &payload[sizeof(VAVA_MSG_HEADER_COMMAND)];
	recv_header=(VAVA_MSG_HEADER_COMMAND *)buf;

    LOG("storage_cmd type %d iGNo %d\n",recv_header->cmd, iGNo);

    vava_msg_header = (VAVA_MSG_HEADER_COMMAND *)&payload ;
    vava_msg_header->cmd = recv_header->cmd;
    vava_msg_header->tag = VAVA_CMD_TAG;
    vava_msg_header->sn = recv_header->sn;

	char *data_buf = buf + sizeof(VAVA_MSG_HEADER_COMMAND);
	if(recv_header->cmdLength)
		LOG("%s\n",data_buf);

    switch(recv_header->cmd)
    {
        case VAVA_CMD_GET_RECMODE:// 300
        {
            record_mode_t *recmode = &g_enviro_struct.record_mode;
			
            //获取录像模式...
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"recmode\":%d,\"starttime\":\"%s\",\"endtime\":\"%s\"}", 
					recmode->r_mod,recmode->starttime,recmode->endtime);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_RECMODE:// 301
        {
            //设置录像模式...
            parse_storage_cmd_json(data_buf, VAVA_CMD_SET_RECMODE, (void *)&g_enviro_struct.record_mode);
			
			SetYSXCtlString( YSX_RECORD_MODE, data_buf, strlen(data_buf)+1);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_GET_REC_QUALITY:
        {
            //获取录像质量...
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"quality\":%d}", GetYSXCtl(YSX_RECORD_QUALITY));
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_REC_QUALITY:
        {
            VAVA_VIDEO_QUALITY quality;
            parse_storage_cmd_json(data_buf, VAVA_CMD_SET_REC_QUALITY, (void *)&quality);

			//media_cmd_switch_record_quality(quality);
			
            //设置录像质量...
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_RECTIMER://暂时用不到
        {
            rectimer_t rectime = {0};
            
            //获取定时录像时间段...
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"week\":%d,\"time\":%d}", rectime.week, rectime.time);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_RECTIMER://暂时用不到
        {
            rectimer_t rectime;
            sscanf(buf, "{\"week\":%d,\"time\":%d}", &rectime.week, &rectime.time);

            //设置定时录像时间段...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_RECTIME:
        {
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"alarmrecord\":%d}", g_enviro_struct.alarm_record_time);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_RECTIME:
        {
            int alarmrecord;
			parse_storage_cmd_json(data_buf, VAVA_CMD_SET_RECTIME, &alarmrecord);
            //设置报警录像时长...
            SetYSXCtl(YSX_DETECT_ALARM_TIMER, alarmrecord);
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);
        }
        break;
        case VAVA_CMD_RECORDDATE_SEARCH:// 308
        {
        	int n,i,cur_n=0,datelist_len;
			struct dirent **namelist,*ptr;
			
            VAVA_REC_TYPE rec_type; //录像类型
            recorddate_search_t recdate;
            parse_storage_cmd_json(data_buf, VAVA_CMD_RECORDDATE_SEARCH, &rec_type);

			if(access("/tmp/sd_ok",F_OK) != 0)//磁盘不存在
			{
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", VAVA_ERR_CODE_SD_NOFOUND);
				break;
			}
			
            //搜索录像，组装字符串...
            n = scandir("/tmp/mmcblk0p1/VAVACAM/", &namelist, NULL, alphasort);
			if (n < 0)
			{
				LOG("scan dir error:%s",strerror(errno));
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", VAVA_ERR_CODE_RECFILE_NOTFOUND);
				break;
			}

			memset(recdate.datelist, 0, sizeof(recdate.datelist));
			memset(resp_data, 0, DATA_SIZE);
			cur_n = 0;
			for(i=0; i<n; i++)//需要添加没有文件夹的场景
			{
				ptr = namelist[i];
				if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)	  ///current dir OR parrent dir
						continue;
				cur_n++;	//记录当前文件夹数量 这里最好判断下是不是 文件夹还是文件fix me
				
				snprintf(&recdate.datelist[datelist_len],DATA_SIZE-datelist_len-1,"{\"date\":\"%s\"},",ptr->d_name);
				datelist_len = strlen(recdate.datelist);
				
				if(cur_n == 200 || i == (n-1))//每满200个目录 或者最后一个文件夹
				{
					recdate.datenum = cur_n;
					recdate.endflag = (i==(n-1))?1:0;
					memset(resp_data, 0, DATA_SIZE);					
					recdate.datelist[datelist_len-1] = '\0';//去掉最后一个逗号 filelist_len 理论上不会为0
					snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"datenum\":%d,\"endflag\":%d,\"datelist\":[%s]}",
						recdate.datenum, recdate.endflag, recdate.datelist);

						
					printf("%s\n",resp_data);
					vava_msg_header->cmdLength = strlen(resp_data);
					PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
					memset(resp_data, 0, DATA_SIZE);
					cur_n = 0;
				}
			}

			if( vava_msg_header->cmdLength == 0)//有VAVA目录，但是没有具体日期的文件夹
			{
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", VAVA_ERR_CODE_RECFILE_NOTFOUND);
				break;
			}
			
			
			return ;

        }
        break;
        case VAVA_CMD_RECORDLIST_SEARCH: //309
        {
        	int n,i,cur_n=0,filelist_len;
			struct dirent **namelist,*ptr;
			char index_file[128];
			char line[32]={0};
			char list_buf[80];
			FILE *fp;
			
            recdate_t rec;
            recordlist_search_t reclist;
			
            parse_storage_cmd_json(data_buf, VAVA_CMD_RECORDLIST_SEARCH, &rec);

            //搜索录像...
			if(access("/tmp/sd_ok",F_OK) != 0)//磁盘不存在
			{
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", VAVA_ERR_CODE_SD_NOFOUND);
				break;
			}

			memset(index_file,0,sizeof(index_file));
			snprintf(index_file,sizeof(index_file)-1,"/tmp/mmcblk0p1/VAVACAM/%s/index.txt",rec.date);
			record_indexfile_Mutex_lock();//互斥 update_index_file
			fp=fopen(index_file,"r");
			if(fp==NULL)//如果索引文件不存在
			{
				record_indexfile_Mutex_unlock();
				return ;
			}
			else//索引文件存在
			{
				cur_n = 0;
				filelist_len = 0;
				reclist.endflag = 0;
				while (fgets(line, sizeof(line) , fp) != NULL)
				{
					int hhmmss,type, du_time;  //每行内容 {"file":"120123_0","type":1,"time":180}
					sscanf(line,"%d_%d.mp4(%d)",&hhmmss, &type,&du_time);
					cur_n++;
					memset(list_buf,0,sizeof(list_buf));
					snprintf(list_buf,sizeof(list_buf)-1,"{\"file\":\"%06d_%d\",\"type\":%d,\"time\":%d},",hhmmss,type,type,du_time);
					snprintf(&reclist.filelist[filelist_len],DATA_SIZE-filelist_len-1,"%s",list_buf);
					filelist_len += strlen(list_buf);

					if(0 == strncmp(line,RECORD_INDEX_END_MARK,strlen(RECORD_INDEX_END_MARK))) // 这两个分支可以优化 有点冗余
					{
						snprintf(reclist.date,sizeof(reclist.date)-1,"%s",rec.date);
						reclist.filenum = cur_n;
						reclist.endflag = 1;
						
						reclist.filelist[filelist_len-1] = '\0';//去掉最后一个逗号 filelist_len 理论上不会为0
						snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"date\":\"%s\",\"filenum\":%d,\"endflag\":%d,\"filelist\":[%s]}",
							reclist.date, reclist.filenum, reclist.endflag, reclist.filelist);	
						printf("%s\n",resp_data);
						vava_msg_header->cmdLength = strlen(resp_data);
						PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
						memset(resp_data, 0, DATA_SIZE);
						filelist_len = 0;
						cur_n = 0;
					}
					else if(cur_n==200)
					{
						snprintf(reclist.date,sizeof(reclist.date)-1,"%s",rec.date);
						reclist.filenum = cur_n;
						if( fgetc(fp) == '[' )//200个可能刚好最好一个包 [END]
						{
							reclist.endflag = 1;
						}
						else
						{
							reclist.endflag = 0;
							fseek(fp,-1,SEEK_CUR);//回退一个字符
						}

						reclist.filelist[filelist_len-1] = '\0';//去掉最后一个逗号 filelist_len 理论上不会为0
						snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"date\":\"%s\",\"filenum\":%d,\"endflag\":%d,\"filelist\":[%s]}",
							reclist.date, reclist.filenum, reclist.endflag, reclist.filelist);	
						//printf("%s\n",resp_data);
						vava_msg_header->cmdLength = strlen(resp_data);
						PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
						memset(resp_data, 0, DATA_SIZE);
						filelist_len = 0;
						cur_n = 0;
					}

					if(reclist.endflag == 1)
						break;
				}
			}
			record_indexfile_Mutex_unlock();
			return ;
        }
        break;
        case VAVA_CMD_RECORD_PLAY_BYTIME:
        {
            recode_play_bytime_t recplay;
            int token = 0;
            parse_storage_cmd_json(data_buf, VAVA_CMD_RECORD_PLAY_BYTIME, &recplay);

            //按时间段回放录像...

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"token\":%d}", token);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);           
        }
        break;
        case VAVA_CMD_RECORD_PLAY_CTRL:
        {
            record_play_ctrl_t rec_play;
			playback_info_t *pb_info = get_pb_info(iGNo);
			
            parse_storage_cmd_json(data_buf, VAVA_CMD_RECORD_PLAY_CTRL, &rec_play);

            //回放控制...
			play_ctrl(pb_info,rec_play);
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"ctrl\":%d}", rec_play.ctrl);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_RECORD_IMG:
        {
            record_img_t rec_i;
            parse_storage_cmd_json(data_buf, VAVA_CMD_RECORD_IMG, &rec_i);
            
            //获取录像I帧或抓图片...
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d,\"dirname\":\"%s\",\"filename\":\"%s\"}", 1, dirname, filename);             
        }
        break;
        case VAVA_CMD_RECORD_IMG_STOP:
        {
            //停止获取录像I帧或抓图片...
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_RECORD_DEL:
        {
            record_del_handle(data_buf);
        
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_EXPORT_TIME_LAPSE:
        {
            time_lapse_t lapse;
            int token = 0;
            parse_storage_cmd_json(data_buf, VAVA_CMD_EXPORT_TIME_LAPSE, &lapse);  

            //导出缩时录影...

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"token\":%d}", token);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SDINFO:
        {
            char sdpath[SD_PATH_LEN]={0};

            //获取SD卡状态
            sdinfo.sdstatus = get_sd_status();

            //获取SD卡容量信息...
            if(!get_sd_path(sdpath, SD_PATH_LEN)){
                get_sd_strorage(sdpath,&sdinfo.totolsize,&sdinfo.freesize);
            }
            
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"diskstatus\":%lld,\"totolsize\":%lld,\"usedsize\":%lld,\"freesize\":%lld}",
                sdinfo.sdstatus, sdinfo.totolsize, sdinfo.totolsize-sdinfo.freesize, sdinfo.freesize);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_FORMAT_SDCARD:
        {
            //格式化SD卡...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);              
        }
        break;
        case VAVA_CMD_POP_SDCARD:
        {
            //弹出SD卡...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1);              
        }
        break;
        case VAVA_CMD_PLAY_SINGLE_FILE:
        {	
        	int ret;
        	playback_info_t *pb_info = get_pb_info(iGNo);
            //单个录像回放或下载...
			parse_storage_cmd_json(data_buf, VAVA_CMD_PLAY_SINGLE_FILE, &pb_info->psf);

			//pb_info->SID = SID;
			pb_info->pb_mode = 1;
			ret = start_sd_playback(iGNo);
			if(ret != VAVA_ERR_CODE_SUCCESS)
			{
				snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", ret);
				break;
			}
            //snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"token\":%d,\"filesize\":%d}", pb_info->token,pb_info->file_size);
			LOG("%s\n",resp_data);
        }
        break;		
        default:
            printf("type [%d] not support\n",vava_msg_header->cmd);
        break;
    }

    vava_msg_header->cmdLength = strlen(resp_data);

    PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
}

void network_cmd(int SID, int channel, char *buf, int iGNo)
{
	VAVA_MSG_HEADER_COMMAND *recv_header;

    VAVA_MSG_HEADER_COMMAND *vava_msg_header;
    char payload[PAYLOAD_SIZE] = {0};
	char *resp_data;

	memset(payload,0,PAYLOAD_SIZE);
	resp_data = &payload[sizeof(VAVA_MSG_HEADER_COMMAND)];
	memset(resp_data,0,DATA_SIZE);
	recv_header=(VAVA_MSG_HEADER_COMMAND *)buf;

    LOG("network_cmd type %d sn %d\n",recv_header->cmd, iGNo);

    vava_msg_header = (VAVA_MSG_HEADER_COMMAND *)&payload ;
    vava_msg_header->cmd = recv_header->cmd;
    vava_msg_header->tag = VAVA_CMD_TAG;
    vava_msg_header->sn = recv_header->sn;

	char *data_buf = buf + sizeof(VAVA_MSG_HEADER_COMMAND);
	if(recv_header->cmdLength)
		LOG("%s\n",data_buf);
	
    switch(recv_header->cmd)
    {
        case VAVA_CMD_WIFI_SCAN_AP:
        {
            scan_ap_t ap = {0};
			memset((void *)&ap, 0, sizeof(ap));
            iwlist_wifi_scan(&ap);

			ap.curnum = 0;
            do{
	            //获取ap搜索热点结果...
	            package_network_cmd_json(&ap, VAVA_CMD_WIFI_SCAN_AP, &resp_data);

				vava_msg_header->cmdLength = strlen(resp_data);
				PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
				
				ap.curnum += 50;
				memset(resp_data,0,DATA_SIZE);
        	}while( ap.curnum < ap.apnum );

			return ;//需要再此返回

            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"ok\",\"apnum\":%d,\"endflag\":%d,\"aplist\":[%s]}",
            //    ap.apnum, ap.endflag, ap.aplist);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1);  
        }
        break;
        case VAVA_CMD_WIFI_CONNECT_AP:
        {
            connect_ap_t connect;
            sscanf(buf, "{\"ssid\":\"%[^\"]\",\"encrypt\":%d,\"password\":\"%[^\"]\"}",
                connect.ssid, &connect.encrypt, connect.password);

            //设置wifi连接信息...
            
            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_WIFI_STATUS:
        {
            wifi_status_t status;
            //获取wifi连接状态...

            if(status.connect == 0){
                snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"connect\":%d,\"ssid\":\"%s\",\"mac\":\"%s\"}",
                    status.connect, status.ssid, status.mac);
            }else{
                snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"connect\":%d,\"mac\":\"%s\"}",
                    status.connect, status.mac);
            }
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_GET_NASSERVER:
        {
            nas_server_t nasserver = {0};
            //获取nas服务器信息...

            snprintf(resp_data, DATA_SIZE-1, "{\"result\":\"ok\",\"ctrl\":%d,\"ip\":\"%s\",\"path\":\"%s\"}",
                    nasserver.ctrl, nasserver.ip, nasserver.path);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        case VAVA_CMD_SET_NASSERVER:
        {
            nas_server_t nasservers;
            sscanf(buf, "{\"ctrl\":%d,\"ip\":\"%[^\"]\",\"path\":\"%[^\"]\"}", &nasservers.ctrl, nasservers.ip, nasservers.path);
            //设置nas服务器...

            snprintf(resp_data, DATA_SIZE-1, RESULT_OK);
            //snprintf(resp_data, DATA_SIZE, "{\"result\":\"fail\",\"errno\":%d}", 1); 
        }
        break;
        default:
            printf("type [%d] not support\n",vava_msg_header->cmd);
        break;
    }
    
    vava_msg_header->cmdLength = strlen(resp_data);

    PPCS_Write(SID, channel, (CHAR*)payload, sizeof(VAVA_MSG_HEADER_COMMAND)+vava_msg_header->cmdLength);
}





