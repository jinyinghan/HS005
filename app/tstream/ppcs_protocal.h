#ifndef __PPCS_PROTOCAL_H_
#define __PPCS_PROTOCAL_H_

#include "vava_header.h"

//同步头tag
#define VAVA_CMD_TAG            0x1D000001
#define VAVA_REALTIME_VIDEO_TAG 0x1D000002
#define VAVA_REALTIME_AUDIO_TAG 0x1D000003
#define VAVA_TALK_AUDIO_TAG     0x1D000004
#define VAVA_RECORD_VIDEO_TAG   0x1D000005
#define VAVA_RECORD_AUDIO_TAG   0x1D000006


#define HARDVER "VA-HS005"  //硬件版本
#define SOFTVER "HS-HS05"  //软件版本
#define OTAVER  "V2.0.0"  //ota版本


#define RESULT_OK "{\"result\":\"ok\"}"
#define RESULT_FAIL "{\"result\":\"fail\",\"errno\":1}"



#define OPEN  1
#define CLOSE 0

#define DATA_SIZE    (16*1024)
#define PAYLOAD_SIZE DATA_SIZE+sizeof(VAVA_MSG_HEADER_COMMAND)

#define RECORD_MODE_DEFAULT  "{\"recmode\":1,\"starttime\":\"0800\",\"endtime\":\"1800\"}"
#define DETECT_REGION_DEFAULT  "{\"start_x\":0,\"end_x\":100,\"start_y\":0,\"end_y\":100}"
#define REC_TIMER_DEFAULT "{\"week\":1,\"time\":1}"


typedef void (*ppcs_cmd_func)(int SID, int channel, char *buf, int iGNo);

typedef struct ppcs_cmd
{
    int type;
    ppcs_cmd_func cfunc;
}ppcs_cmd_t;

//会话认证
typedef struct session_auth
{
    int random;
    char auth[64];
    char key[64];
}__attribute__((packed)) session_auth_t;

//布防列表参数
typedef struct arminglist
{
    int enable;
    char start_time[8];
    char end_time[8];
    char days[8];
}__attribute__((packed)) arminglist_t;

//布防
typedef struct arminginfo
{
    int type;
    arminglist_t arming_list[10]; 
}__attribute__((packed)) arminginfo_t;

//人形
typedef struct pdparam
{
    int enable;
    int rect;
}__attribute__((packed)) pdparam_t;

//人脸
typedef struct fdparam
{
    int enable;
    int rect;
}__attribute__((packed)) fdparam_t;

//AI检测区域
typedef struct ai_region
{
    int start_x;
    int end_x;
    int start_y;
    int end_y;
}__attribute__((packed)) ai_region_t;

//报警模式
typedef struct alarm_mode
{
    int mode;
    int pirsens;
    int pirwait;
    int rectime;
}__attribute__((packed)) alarm_mode_t;

//时区信息
typedef struct ntpinfo
{
    int timezoom;
    char region[64];
    char data[64]; //时区值
}__attribute__((packed)) ntpinfo_t;

//ota
typedef struct ota_info
{
    char url[128];
    int type;
    char otaver[16];
}__attribute__((packed)) ota_info_t;

//ota状态
typedef struct ota_status
{
    int status;
    int type;
    char otaver[16];
    int loaddata;
}__attribute__((packed)) ota_status_t;

//视频流信息
typedef struct video_info
{
    int videocodec;
    int videores;
    int videoframerate;
    int videobitrate;
    int videonum;
}__attribute__((packed)) video_info_t;

//音频信息
typedef struct audio_info
{
    int audiocodec;
    int audiorate;
    int audiobitper;
    int audiochannel;
    int audioframerate;
}__attribute__((packed)) audio_info_t;

//定时录像时间段
typedef struct rectimer
{
    int week;
    int time;
}__attribute__((packed)) rectimer_t;

//指定日期录像
typedef struct recdate
{
    char date[16];
    int type;
}__attribute__((packed)) recdate_t;

typedef struct recorddate_search
{
    int datenum;
    int endflag;
    char datelist[DATA_SIZE]; //暂时简单定义一下  200 条{"date":"20171012"},差不多4KB
}__attribute__((packed)) recorddate_search_t;

typedef struct recordlist_search
{
    char date[16];
    int filenum;
    int endflag;
    char filelist[DATA_SIZE]; //暂时简单定义一下 {"file":"120123_0","type":1,"time":180} 200条差不多16KB
}__attribute__((packed)) recordlist_search_t;

//按时间段进行录像回放
typedef struct recode_play_bytime
{
    char dirname[16];
    int type;
    int rectype;
    char starttime[8];
    char endtime[8];
}__attribute__((packed)) recode_play_bytime_t;

//回放控制
typedef struct record_play_ctrl
{
    int token;
    int ctrl;
}__attribute__((packed)) record_play_ctrl_t;

//获取录像I帧或抓图片
typedef struct record_img
{
    char dirname[16];
    char filename[16];
}__attribute__((packed)) record_img_t;

//删除录像文件
typedef struct recode_del
{
    char dirname[16];
    int flag;
}__attribute__((packed)) recode_del_t;

//导出缩时录影
typedef struct time_lapse
{
    char dirname[16];
    int lastsec;
    char starttime[8];
    char endtime[8];    
}__attribute__((packed)) time_lapse_t;

//单个录像回放或者下载
typedef struct play_single_file
{
	char dirname[16];
	char filename[16];
	VAVA_RECFILE_TRANSPORT type;
}__attribute__((packed)) play_single_file_t;

typedef struct sd_info
{
    VAVA_SD_STATUS sdstatus;
    unsigned long long totolsize;
    unsigned long long usedsize;
    unsigned long long freesize;   
}__attribute__((packed)) sd_info_t;

typedef struct aplist_type
{
    char ssid[64];
    int signal;
    int encrypt;
}__attribute__((packed)) aplist_type;

typedef struct scan_ap
{
    char result[3];
	int curnum;//curnum;
    int apnum;//all number
    int endflag;
    aplist_type aplist[256]; //暂时简单定义一下
}__attribute__((packed)) scan_ap_t;

//连接热点的信息
typedef struct connect_ap
{
    char ssid[128];
    char password[128];
    int encrypt;
}__attribute__((packed)) connect_ap_t;

typedef struct wifi_status
{
    int connect;
    char ssid[128];
    char mac[32];
}__attribute__((packed)) wifi_status_t;

//nas服务器信息
typedef struct nas_server
{
    int ctrl;
    char ip[32];
    char path[128];
}__attribute__((packed)) nas_server_t;


// 设置录像模式 VAVA_CMD_GET_RECMODE
typedef struct record_mode
{
	VAVA_REC_MODE r_mod;
	char starttime[5]; //0800
	char endtime[5];   //1800
}__attribute__((packed))record_mode_t;



void ppcs_demo(void);


#endif

