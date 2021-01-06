#ifndef COMMON
#define COMMON

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <time.h>
#include <pthread.h>

#include "openssl/aes.h"
#include "openssl/evp.h"
#include "http_curl.h"



#include "include/iniparser.h"
#include <stdbool.h>
#include "network_detection.h"

#include "log.h"
#include "play.h"
#include "ppcs_protocal.h"

#define FILE_JPEG "/tmp/mdpic.jpeg"
#define FILE_VIDEO "/tmp/mdrecord.mp4"
#define FILE_SNAP_JPEG "/tmp/snap.jpeg"

typedef struct _FRAMEINFO
{
	   unsigned short codec_id;    // Media codec type defined in sys_mmdef.h,
								   // MEDIA_CODEC_AUDIO_PCMLE16 for audio,
								   // MEDIA_CODEC_VIDEO_H264 for video.
	   unsigned char flags; 	   // Combined with IPC_FRAME_xxx.
	   unsigned char cam_index;    // 0 - n
 
	   unsigned char onlineNum;    // number of client connected this device
	   unsigned char reserve1[3];
 
	   unsigned int reserve2;  //
	   unsigned int timestamp; // Timestamp of the frame, in milliseconds

}FRAMEINFO_t;

#define PATRACK

#ifdef    ERR_DEBUG_SWITCH
#define pr_err(fmt,args...) YSXLOG(LOG_DETAIL,__FUNCTION__,__LINE__,fmt,##args)
#else
#define pr_err(fmt,args...) /*do nothing */
#endif

#ifdef CONFIG_PTZ
#include "ptz.h"
#endif

#define QCAM_AV_C_API
#include "qcam.h"
//#include "qcam_log.h"
#include "qcam_audio_input.h"
#include "qcam_audio_output.h"
//#define LOG printf

#define AES_KEY 								"Qi=+-ho!&(wniyeK"


#define AUDIO_SAMPLERATE_YSX					16000

#define MOTION_DTC_TIME_OUT						0x3C
#define MOTION_DTC_ALERT_TIME_OUT				0x5
#define AUDIO_SAMPLELEN_YSX						0x280
#define MAC_LEN									0x12
#define SSID_LEN								0x20
#define SD_PATH_LEN								0x32
#define MAIN_CHN   								0
#define SECOND_CHN 								1
#define THIRD_CHN 								2
#if (defined GT_G201) || (defined GT_G202)
#define FULL_COLOR_TIMTOUT  					(0x5A)
#else
#define FULL_COLOR_TIMTOUT  					(0x3C)
#endif

/*tutk net state*/
typedef enum {
	RESOURCE_TYPE_INVALID = -1,
    IMAGE_JPEG,
    VIDEO_MP4,
    RESOURCE_TYPE_MAX
}ResourceType;

#define MAX_UID_SIZE  25
#define MAX_USER_NUMBER 4  //最大用户数量

#define FILE_WPA 					"/etc/SNIP39/wpa_supplicant.conf"
#define FILE_WPA_BAK 				"/etc/SNIP39/wpa_supplicant.conf.bak"
#define CAMERA_STATE   				"/etc/SNIP39/default.conf"
#define DEV_BOOT_FLAG	 			"/tmp/sys_start"
#define FILE_UID					"/etc/SNIP39/SNIP39_UID.conf"
#define SD_INIT_FLAG	 			"/tmp/sd_init_position"
#define SD_LOG_FLAG 				"/tmp/mmcblk0p1/log_record"
#define RELAY_INVAILD_F 			"/etc/SNIP39/relay"

#define ONLINE_PCM					"online.pcm"
#define WELCOME_PCM                 "tanks.pcm"

#define EZ_PCM						"wifi_conf.pcm"
#define WIFI_PCM					"wifi_set.pcm"

#define VERSIONCONFIG				"/etc/ysx/version.conf"
#define CAMERA_CONF					"/etc/SNIP39/camera.conf"
#define DEV_CONF					"/etc/SNIP39/device.conf"
#define PTRACK_CONF					"/etc/SNIP39/ptrack.conf"
#define APMAC_CONF					"/etc/SNIP39/ap_mac.conf"

#define MAX_SIZE_IOCTRL_BUF			1024
#define SERVTYPE_STREAM_SERVER 0

typedef enum
{
	YSX_C_EN,				//1: en, 0: dis
	YSX_IR_MODE,			//red 2: en, 1: dis, 0: auto quancai 10:auto 11:dis 12:en
	YSX_MIC_EN,
	YSX_SPK_EN,
	YSX_VINFO_EN,
	YSX_LIGHT_EN,			//1:en, 0:dis default -1
	YSX_VOL,
	YSX_MOTION,				//1: en, 0: dis
	YSX_ROTATE,				//1: en, 0: dis
	YSX_AUTOTURN,
	YSX_CAMERA_MOVE,		//正在移动
	YSX_SHARE,
	YSX_VQUALITY,       // VAVA_VIDEO_QUALITY
	YSX_INVERSION,
	YSX_MD_SENSE,
	YSX_MD_TYPE,
	YSX_MD_REGION_BIT,		//14
	YSX_POINT_CPOINT_H,
	YSX_POINT_CPOINT_V,
	YSX_POINT_TRACK_H,
	YSX_POINT_TRACK_V,		//18
	YSX_POINT_RECORD_H,
	YSX_POINT_RECORD_V,
	YSX_DEV_INIT,
	YSX_DEV_BOOT,
	YSX_ATRACK_EN,		//auto_track
	YSX_PTRACK_EN,		//point_track
	YSX_MOTION_B_YSX_C_EN, 	//1: en, 0: dis
	YSX_IR_MODE_B_YSX_C_EN,	//2: en, 1: dis, 0: auto
	YSX_PTRACK_TT,		//point_track times//27
	YSX_PTRACK_TS,		//point_track start time
	YSX_PTRACK_TE,		//point_track end time
	YSX_MOTION_OSD,		//1: en, 0: dis
	YSX_WIFI_SWTICH_ST,
	YSX_APOINT_RECORD_H, //point before atrack begain, horizon
	YSX_APOINT_RECORD_V,
	YSX_CLOUD_ENV,  	// for cloud
	YSX_DEV_LOGIN_TYPE,	// for change the login type
	YSX_DEV_ALERT, // for change the login type
	YSX_CAM_TIME_ALARM_DAY,		//bit1 ~ bit7 分别代表周一到周日
	YSX_CAM_TIME_ALARM_TS, 		//关机开始时间，以分钟为单位
	YSX_CAM_TIME_ALARM_TE, 		//关机结束时间，以分钟为单位
	YSX_IVS_SENSE,				//0: disable; 1: low; 2: medium; 3: high
	YSX_STORAGE_RESOLUTION,		//0:low 1:middle 2:high	
	YSX_RECORD_MODE,
    YSX_NET_T_ONLINE,
    YSX_NET_D_ONLINE,           //0: default, -1:detach 1:attach
    YSX_VIEW_NUM,
    YSX_AUTO_BITRATE,
    YSX_FILL_LED_MODE,
    YSX_FILL_LED_OPEN,
    YSX_AI_MODE,
    YSX_MAINS_FREQUENCY,
    YSX_SOUND_DETECTION,
    YSX_DETECT_ALARM_TIMER,
    YSX_DETECT_REGION,
	YSX_RECORD_QUALITY,  // VAVA_VIDEO_QUALITY
	YSX_ALARM_RECORD_TIME, //10~180 s    
}YSX_CTRL;


/*LED status */
typedef enum
{
	YSX_IR_MODE_MIN = -1,
	YSX_IR_MODE_AUTO,
	YSX_IR_MODE_DISABLE,
	YSX_IR_MODE_ENABLE,
	YSX_IR_MODE_SMART = 10,
	YSX_IR_MODE_NORMAL,
	YSX_IR_MODE_FULLCOLOR,
	YSX_IR_MODE_MAX,
}YSX_IR_CTL;

/* VIDEO RESOLUTION */
typedef enum
{
	YSX_RESOLUTION_LOW = 0,
	YSX_RESOLUTION_MID = 1,
	YSX_RESOLUTION_HIGH = 2, //1296P
}YSX_VIDEO_RESOLUTION;


/* RECORD STATUS */
typedef enum
{
	YSX_RECORD_ING 	= 0,
	YSX_RECORD_END	= 1,
}YSX_RECORD_STATUS;

/* RECORD RESOLUTION STATUS */
typedef enum
{
	YSX_RESOLUTION_END 	= 0,
	YSX_RESOLUTION_SET	= 1,
}YSX_RESOLUTION_STATUS;

/*IR LED status */
typedef enum
{
	LED_STATUS_MIN      = -1,
    EZ_STATUS,          //0
	EZ_STATUS_ROUTE,
	ON_LINE_STATUS,
	OFF_LINE_STATUS,
	VIDEO_START_STATUS,	//4
	VIDEO_STOP_STATUS,
	BURN_FIRMWARE_STATUS,
	LED_DISABLE
}YSX_LEDCTL;



typedef struct {
    int started;
    pthread_t thread_id;
    int destroyed_thread;
} motion_track_context;

/*** main.c*********/
typedef struct _gEnviro
{
	char t_uid[MAX_UID_SIZE+1];
	unsigned char t_online;
	//unsigned char t_playinuse;
	unsigned char t_online_num;
	char t_sdpath[SD_PATH_LEN];
	unsigned char t_sdpathlen;
	unsigned int t_sdfree;
	unsigned int t_sdtotal;
	char t_name[10];
	char t_pass[20];
	char t_model[10];
	char t_vendor[10];
	char t_wifissid[32];
	char t_wifipass[64];
	char t_lan[10];
	unsigned char t_wifiencrypt;

	int ptz_autoturn_flag;
	int t_ledstatus;
	int t_zone;
	int t_zone_m;
	char t_version[16];
	pthread_mutex_t t_Mutex ;

	unsigned int vc_type; //1080 or 720 	/*支持的分辨率*/
	unsigned int vc_default;				/*默认分辨率*/
	unsigned int vc_mf; //mirror flip		/*镜像翻转*/
	const char **vc_index;

	unsigned char motion_en;	/*移动报警使能*/
	unsigned int region_bit;
	unsigned int t_sense;	/*移动侦测灵敏度*/
	unsigned int md_tStart;
	unsigned int md_tEnd;
	uint8_t motion_type; //移动侦测类型
	unsigned int track_sense;
	unsigned int atrack_en;		/*移动追踪使能*/
	unsigned int ptrack_en;		/*定点巡航使能*/
	unsigned int ptrack_tt;		//point_track times
	unsigned int ptrack_ts;		//point_track start time
	unsigned int ptrack_te;		//point_track end time
	unsigned int mosd_en;		//移动框选使能
	unsigned int wifi_switch_st;//0: default, 1: fail, 2: success
	unsigned int storage_resolution;//0:low, 1:middle, 2:high
	YSX_RECORD_STATUS record_status;
	YSX_RESOLUTION_STATUS resolution_status;
	record_mode_t record_mode; //录像模式  VAVA_RECORD_MODE   / VAVA_CMD_GET_RECORD_MODE  /YSX_RECORD_MODE
	
	unsigned char _loop;
	uint8_t camera_en;
	uint8_t ir_mode;
	uint8_t mic_en;
	uint8_t spk_en;  //所有用户无法开启 对讲
	int spk_vol;
	uint8_t light_en;		/*指示灯开关*/
	uint8_t t_volinfoEn;	/*提示音开关*/
	uint8_t share_en;
    uint8_t sound_detection; //声音侦测开关
    uint8_t detection_timer; //侦测告警录像的时间  长度（秒）
    ai_region_t detect_region; //侦测区域

	unsigned char record_quality;// 录像质量
	unsigned char  alarm_record_time;//告警录像时间长度 10~180
    uint8_t is_formating_sd; //1：SD卡正在格式化。0：SD卡没有在格式化
	
	char share_pass[20];
	char wifissid_bak[32];
	char t_apmac[MAC_LEN];
	char t_apssid[SSID_LEN];
    char server_env;         // 1--国内，0--国外
#if defined (DG201)
	char openlight_status;  //双光源标志
#endif
	char login_type;
	char dev_alert;
    unsigned int sensitivity;
	int time_alarm_day;
	int time_alarm_ts;
	int time_alarm_te;
	int d_online;       //device network status

	int t_nStart_o;
	int t_nStart_s;
	int t_nEnd_o;
	int t_nEnd_s;
	int t_fillMode;
	int t_fillOpen;
	int f_light_st;
	int t_aiMode;
	int t_mainFrequency;

	motion_track_context motion_track;
	NETWORK_MODE_E net_mode;
	NETWORK_STATUS_E net_status;
	unsigned char is_bind;
	unsigned char auto_bitrate;
	unsigned char standalone_mode;
	time_t motion_timep;

	#ifdef YSX_RTSP_WEBSOCK
	
	
	char rtsp_is_ok ;
	char rtsp_open ;
	RTSPS_ctx_t *rtsp_ctx;
	struct timeval rtsp_st_t;
	char rtspServPort[16];
	char rtspServHost[64];
	char rtspAddr[256] ;  //
	//int rtsp_cmd_flush;
	#endif
}G_StructEnviroment;

#if defined (DG201)
	pthread_cond_t twolight_cond;
	pthread_mutex_t twolight_lock;
#endif
extern G_StructEnviroment g_enviro_struct;

typedef struct _gMediaInfo
{
	/*video&mic*/
	int sessionid;// >=0:已经占用   -1表示未被占用  Session ID
	unsigned char is_VA; //视频是否开启
	unsigned char is_AA; //音频是否开启
	pthread_mutex_t va_Mutex ;
	pthread_mutex_t aa_Mutex ;
	int prep_framenum;
	int video_framenum ;//每个用户都 有一个视频帧序
	int audio_framenum;//音频帧率

	playback_info_t  pb_info;
}G_StructMediaInfo;

typedef struct _gRecordInfo
{
    int enable;
    int height;
    int width;
    int fps;
    int bitrate;		
    int stream_id;
	int playload;
}G_StructRecordInfo;
G_StructRecordInfo g_recordinfo_struct[2];

typedef struct _gMedia
{
	unsigned char g_isVideoNum;

	pthread_mutex_t g_vMutex ;
	pthread_mutex_t g_aMutex ;
	G_StructMediaInfo g_media_info[10];//理论上  仅仅会用到 4,5个用户

	/*play*/
	int playch;
    int now_pavIndex;

	unsigned char bChange;
	unsigned char bPlayStatus;

	/*pthread lock for playback*/
	pthread_mutex_t pa_Mutex ;

	/*speaker*/
	unsigned char is_SA;
	unsigned char is_playing;
	int avIndex_s;
	pthread_mutex_t sa_Mutex ;

	unsigned char bitrate_level;
	YSX_VIDEO_RESOLUTION resolution;//分辨率和质量有点交集
}G_StructMedia;

extern G_StructMedia g_media_struct;

typedef struct _recordindex
{
	unsigned int iy;
	char im;
	char id;
	char ih;
	char imi;
	unsigned int timelen;
}RIndex;

typedef struct _recordindexNode
{
	RIndex rtotal;
	unsigned long rtotal_stamp;
	struct _recordindexNode *pre,*next;
}LNodeRecord, *LNodeRecordList;

extern LNodeRecordList g_noderecord;
extern unsigned int irtotal_num;

typedef enum
{
	SIGNAL_LEVEL_0 = 0,
	SIGNAL_LEVEL_1,
	SIGNAL_LEVEL_2,
	SIGNAL_LEVEL_3,
	SIGNAL_LEVEL_4,
	SIGNAL_LEVEL_5,
	SIGNAL_LEVEL_6,
	SIGNAL_LEVEL_7,
	SIGNAL_LEVEL_8,
	SIGNAL_LEVEL_9,
	SIGNAL_LEVEL_FULL,
	SIGNAL_UNIT_10 = 10,
	SIGNAL_UNIT_20 = 20,
	SIGNAL_UNIT_25 = 25,
}YSX_Wifi_Level;

/*仅设备重启执行马达自检，声音播放*/
typedef struct _gDeviceflag
{
	bool ptz_reset_flag;
	bool wifi_conf_replay_flag;
	bool tanks_replay_flag;
	bool wifi_set_replay_flag;
}G_StructDeviceflag;
/**
 * LED行为命令.
 */
typedef enum {
	LED_OFF,		/**< LED关闭 */
	LED_ON,			/**< LED打开 */
} SULedCmd;

/*cam alarm status */
typedef enum
{//IPC开关-> C, 定时使IPC开关-> D.bit0-bit1: 00 CD off, 01 CD on, 10 C off D on, 11 C on D off;
	CAM_CLARM_MIN 	= -1,
	CAM_CLARM_OFF,
	CAM_CLARM_ON,
	CAM_CLARM_OFF_ON,
	CAM_CLARM_ON_OFF,
	CAM_CLARM_MAX,
}ysx_cam_status_e;

typedef enum {
        QCAM_MODE_NIGHT=0,
        QCAM_MODE_DAY,	
}day_Mode;

void DeInitAVInfo();


#endif
