//
//  VAVA_Header.h
//  common
//
//  Created by luoshimei on 2020/11/3.
//  Copyright © 2020 yingshixun. All rights reserved.
#ifndef __VAVA_HEADER_H_
#define __VAVA_HEADER_H_


//消息头之信令
#include "PPCS_Type.h"

typedef struct str_vava_msg_header_command{
    UINT32 tag;         // 同步头 固定值 0x1D000001
    UINT32 cmd;         // 命令字 见协议分类
    UINT32 cmdLength;   // 消息内容长度 无消息内容时填 0 且不传消息内容
    UINT32 sn;          // 具体含义要两端协商即可，待定
}__attribute__((packed)) VAVA_MSG_HEADER_COMMAND;

//消息头之音视频流传输
typedef struct str_vava_msg_header_av_stream{
    UINT32 tag;         // 同步头 实时视频流同步头 0x1D000002;实时音频流同步头 0x1D000003;对讲音频流同步头 0x1D000004;回放视频同步头 0x1D000005;回放音频同频头 0x1D000006;
    UINT8  encodeType;  // 消息内容长度 视频编码类型 0:H264 1:H265;音频编码类型 0:PCM 1:G711A 2:G711U 3:AAC 4:OPUS
    UINT8  frameType;   // 帧类型 0:P帧 1:I帧 8:音频帧
    UINT8  frameRate;   // 帧率 1~120
    UINT8  res;         // 分辨率 仅视频有效 VAVA_VIDEO_RESOLUTION
    UINT32 size;        // 帧数据长度 仅音视频帧数据的长度，不包含消息头
    UINT32 ntsampSec;   // 时间戳 秒
    UINT32 ntsampUsec;  // 时间戳 毫秒，注意:秒+毫秒才是一个完整的毫 秒级时间戳
    UINT32 frameNum;    // 帧序号 从0开始累加
}__attribute__((packed)) VAVA_MSG_HEADER_AV_STREAM;

//消息头之图片传输
typedef struct str_vava_msg_header_image{
    UINT32 tag;             // 同步头 固定值 0x1D000007
    UINT32 type;            // 图片类型 0 是I帧数据 1是JPG数据
    UINT32 dataType;        // 数据类型 参考 VAVA_REC_TYPE
    UINT32 size;            // 图片/I 帧数据长度 仅图片数据长度，不包含消息头
    INT32 result;         // 返回值 0 成功 非 0 失败
    UINT8  date[9];         // 日期 年月日，比如:20200922
    UINT8  time[11];        // 时间 时分秒，比如:171129
}__attribute__((packed)) VAVA_MSG_HEADER_IMAGE;


// 6 协议分类
// 6.1 系统消息 命令字1~199
typedef enum
{
	// 下行消息
	VAVA_CMD_SESSION_AUTH = 1,//会话认证
	VAVA_CMD_BUZZER_OPEN =  2,//开启报警语音
	VAVA_CMD_BUZZER_CLOSE = 3,//关闭报警语音
	VAVA_CMD_GET_IRMODE =   4,//获取夜视模式
	VAVA_CMD_SET_IRMODE =   5,//设置夜视模式
	VAVA_CMD_GET_PIR_SENSITIVITY = 6, //获取 PIR 灵敏度
	VAVA_CMD_SET_SENSITIVITY = 7,     //设置 PIR 灵敏度
	VAVA_CMD_GET_ARMINGINFO_V1 = 8,   //获取布防配置信息 
	VAVA_CMD_SET_ARMINGINFO_V1 = 9,   //设置布防配置信息
	VAVA_CMD_GET_PDPARAM = 10, //获取人形检测参数
	VAVA_CMD_SET_PDPARAM = 11, //设置人形检测参数
	VAVA_CMD_GET_FDPARAM = 12, //获取人脸检测参数
	VAVA_CMD_SET_FDPARAM = 13, //设置人脸检测参数
	VAVA_CMD_STATUSLED_GET = 14, // 获取 LED 状态灯状态
	VAVA_CMD_STATUSLED_SET = 15, // 设置 LED 状态灯开关
	VAVA_CMD_GET_AI_REGION = 16, // 获取自定义 AI 检测区域
	VAVA_CMD_SET_AI_REGION = 17, // 设置自定义 AI 检测区域
	VAVA_CMD_GET_IPC_SPEAKERVOL = 18, //获取喇叭音量
	VAVA_CMD_SET_IPC_SPEAKERVOL = 19, //设置喇叭音量
	VAVA_CMD_GET_OSD_STATUS = 20, //获取 LOGO 水印状态
	VAVA_CMD_SET_OSD_STATUS = 21, //设置 LOGO 水印状态
	VAVA_CMD_GET_ALARM_MODE = 22, //获取报警模式
	VAVA_CMD_SET_ALARM_MODE = 23, //设置报警模式
	VAVA_CMD_GET_NTPINFO = 24, //获取时区信息
	VAVA_CMD_SET_NTPINFO = 25, //设置时区信息
	VAVA_CMD_SYSTEM_NEWVESION = 26,  //升级
	VAVA_CMD_GET_UPDATE_STATUS = 27, //获取升级状态
	VAVA_CMD_CAMERA_RESET = 28, //设备端恢复默认参数
	VAVA_CMD_PAIR_MODE = 29, //设备端进入待绑定状态
	VAVA_CMD_GET_LANGUAGE = 30, //获取语言
	VAVA_CMD_SET_LANGUAGE = 31, //设置语言
	VAVA_CMD_SET_CAMERA_ENABLE = 32, //设置摄像头状态 开关
	VAVA_CMD_GET_MD_SENSITIVITY = 33, //获取移动侦测灵敏度
	VAVA_CMD_SET_MD_SENSITIVITY = 34, //设置移动侦测灵敏度
	VAVA_CMD_GET_RECORD_MODE = 35, //获取录像形式
	VAVA_CMD_SET_RECORD_MODE = 36, //设置录像形式 
	VAVA_CMD_GET_DETECT_MODE = 37, //获取侦测类型
	VAVA_CMD_SET_DETECT_MODE = 38, //设置侦测类型
	VAVA_CMD_SET_SOUND_DETECTION = 39,// 设置声音侦测状态
	VAVA_CMD_SET_DETECTION_TRACK = 40,// 设置人形/移动追踪
	VAVA_CMD_CAMERA_RESTART = 41,  //设备重启
	VAVA_CMD_CAMERAINFO = 42,  //获取摄像机信息

	
}VAVA_SYSTEM_CMD;

// 6.2 流媒体消息 命令字200~299
typedef enum
{
	//下行消息
	VAVA_CMD_OPENVIDEO =   200, //开启视频流
	VAVA_CMD_CLOSEVIDEO =  201, //关闭视频流
	VAVA_CMD_AUDIO_OPEN =  202, //开启音频流
	VAVA_CMD_AUDIO_CLOSE = 203, //关闭音频流
	VAVA_CMD_TALK_OPEN = 204, //开启对讲
	VAVA_CMD_TALK_CLOSE = 205, //关闭对讲
	VAVA_CMD_MIC_OPEN = 206, // 设备端麦克风开启
	VAVA_CMD_MIC_CLOSE = 207, // 设备端麦克风关闭
	VAVA_CMD_SPEAKER_OPEN = 208, //设备端喇叭开启
	VAVA_CMD_SPEAKER_CLOSE = 209, //设备端喇叭关闭
	VAVA_CMD_GET_VIDEO_QUALITY = 210, //获取视频质量
	VAVA_CMD_SET_VIDEO_QUALITY = 211, //设置视频质量
	VAVA_CMD_GET_MIRRORMODE = 212, //获取视频镜像
	VAVA_CMD_SET_MIRRORMODE = 213, //设置视频镜像
	VAVA_CMD_GET_VIDIOCODE = 214, //获取视频编码格式
	VAVA_CMD_SET_VIDIOCODE = 215, //设置视频编码格式
	VAVA_CMD_SNAPSHOT = 216, //抓图
	
	//上行消息
	//暂无
}VAVA_MEDIA_CMD;

// 6.3 录像、存储消息 命令字300~399	
typedef enum
{
	//下行消息
	VAVA_CMD_GET_RECMODE = 300, //获取录像模式
	VAVA_CMD_SET_RECMODE = 301, //设置录像模式
	VAVA_CMD_GET_REC_QUALITY = 302, //获取录像质量
	VAVA_CMD_SET_REC_QUALITY = 303, //设置录像质量
	VAVA_CMD_GET_RECTIMER = 304, //获取定时录像的时间段
	VAVA_CMD_SET_RECTIMER = 305, //设置定时录像的时间段
	VAVA_CMD_GET_RECTIME = 306, //获取报警录像时长
	VAVA_CMD_SET_RECTIME = 307, //设置报警录像时长
	VAVA_CMD_RECORDDATE_SEARCH = 308, //搜索存在录像的日期
	VAVA_CMD_RECORDLIST_SEARCH = 309, //搜索指定日期的录像
	VAVA_CMD_RECORD_PLAY_BYTIME = 310, //按时间段进行录像回放
	VAVA_CMD_RECORD_PLAY_CTRL = 311, //回放控制
	VAVA_CMD_RECORD_IMG = 312, //获取录像 I 帧数据或抓图图片
	VAVA_CMD_RECORD_IMG_STOP = 313, //停止处理录像 I 帧或抓图图片的传输
	VAVA_CMD_RECORD_DEL = 314, //删除录像文件
	VAVA_CMD_EXPORT_TIME_LAPSE = 315, //导出缩时录影
	VAVA_CMD_SDINFO = 316, //获取 SD 卡信息
	VAVA_CMD_FORMAT_SDCARD = 317, //格式化 SD 卡
	VAVA_CMD_POP_SDCARD = 318, //弹出 SD 卡
	VAVA_CMD_PLAY_SINGLE_FILE = 319, //单个录像回放基站TF卡种存储的录像，支持回放模式和文件传输模式
	
	//上行消息
	//VAVA_CMD_PLAYBACK_END = 350, //录像文件传输完成
	//VAVA_CMD_RECORD_DEL_RESP = 351, //录像文件删除结果
	
}VAVA_STORAGE_CMD;


// 6.4 网络消息 命令字400~499	
typedef enum
{
	//下行消息
	 VAVA_CMD_WIFI_SCAN_AP = 401, //获取 wifi 搜索热点的结果
	 VAVA_CMD_WIFI_CONNECT_AP = 402, //设置 wifi 连接热点的信息
	 VAVA_CMD_GET_WIFI_STATUS = 403, //获取 wifi 连接状态
	 VAVA_CMD_GET_NASSERVER = 404, //获取 NAS 服务器信息
	 VAVA_CMD_SET_NASSERVER = 405, //设置 NAS 服务器信息
	 
	
	//上行消息
	//暂无
}VAVA_NETWORK_CMD;


// 6.5 上下消息  600 ~ 700
typedef enum
{	
	VAVA_CMD_PLAYBACK_END = 601,// 录像文件传输完成
	VAVA_CMD_RECORD_DEL_RESP = 602, // 录像文件删除结果
	VAVA_CMD_BUZZER_STATUS = 603, //设备端语音报警状态上报
	
	VAVA_CMD_UPGRATE_STATUS = 608, //升级状态改变
	
}VAVA_UPDOWN_CMD;


// 7. 数据格式

// 1. 控制类型 VAVA_CTRL_TYPE
typedef enum {
    VAVA_CTRL_DISABLE   = 0,    // 关闭
    VAVA_CTRL_ENABLE    = 1     // 开启
} VAVA_CTRL_TYPE;

// 2. PIR 灵敏度 VAVA_PIR_SENSITIVITY
typedef enum {
    VAVA_PIR_SENSITIVITY_HIGH      = 1,    // 高
    VAVA_PIR_SENSITIVITY_MIDDLE    = 2,    // 中
    VAVA_PIR_SENSITIVITY_LOW       = 3     // 低
} VAVA_PIR_SENSITIVITY;

// 3. 移动侦测灵敏度 VAVA_MD_SENSITIVITY
typedef enum {
    VAVA_MD_SENSITIVITY_HIGH      = 10,    // 高
    VAVA_MD_SENSITIVITY_MIDDLE    = 5,     // 中
    VAVA_MD_SENSITIVITY_LOW       = 1      // 低
} VAVA_MD_SENSITIVITY;

// 4. PIR 报警模式 VAVA_ALARMMODE
typedef enum {
    VAVA_ALARMMODE_POWERSAVE    = 0,    // 省电模式
    VAVA_ALARMMODE_MONITOR      = 1,    // 监控模式
    VAVA_ALARMMODE_CUSTOM       = 2     // 自定义模式
} VAVA_ALARMMODE;

// 5. 视频编码格式 VAVA_VIDEO_CODEC
typedef enum {
    VAVA_VIDEO_CODEC_H264   = 0,    // H264
    VAVA_VIDEO_CODEC_H265   = 1,    // H265
} VAVA_VIDEO_CODEC;

// 6. 视频分辨率 VAVA_VIDEO_RESOLUTION
// 注意事项:目前只定义了几个标准分辨率，后续需要扩展则在标准分辨率之间预 留的数值间进行插值即可;排列原则:值越大，分辨率越大;
typedef enum {
    VAVA_VIDEO_RESOLUTION_QVGA  = 5,    // 320 * 240
    VAVA_VIDEO_RESOLUTION_VGA   = 10,   // 640 * 480
    VAVA_VIDEO_RESOLUTION_720P  = 20,   // 1280 * 720
    VAVA_VIDEO_RESOLUTION_1080P = 30,   // 1920 * 1080
    VAVA_VIDEO_RESOLUTION_DCI2K = 40,   // 2048 * 1280
    VAVA_VIDEO_RESOLUTION_DCI4K = 50    // 4096 * 2160
} VAVA_VIDEO_RESOLUTION;

// 7. 视频质量 VAVA_VIDEO_QUALITY
typedef enum {
    VAVA_VIDEO_QUALITY_BEST     = 0,    // 最佳画质(高)
    VAVA_VIDEO_QUALITY_HIGH     = 1,    // 高清画质(中)
    VAVA_VIDEO_QUALITY_RENEWAL  = 2,    // 最佳流畅(低)
    VAVA_VIDEO_QUALITY_AUTO          = 3,     // 自动模式
    VAVA_VIDEO_QUALITY_AUTO_BEST     = 4,     // 自动模式 最佳画质 在AUTO模式下 
    VAVA_VIDEO_QUALITY_AUTO_HIGH     = 5,     
    VAVA_VIDEO_QUALITY_AUTO_RENEWAL  = 6,     
    VAVA_VIDEO_QUALITY_AUTO_2K       = 7,     
    VAVA_VIDEO_QUALITY_AUTO_4K       = 8,     
    VAVA_VIDEO_QUALITY_2K            = 10,     
    VAVA_VIDEO_QUALITY_4K            = 11,    
} VAVA_VIDEO_QUALITY;

// 8. 视频翻转类型 VAVA_MIRROR_TYPE
typedef enum {
    VAVA_MIRROR_TYPE_NORMAL             = 0,    // 不翻转
    VAVA_MIRROR_TYPE_HORIZONTALLY       = 1,    // 水平翻转
    VAVA_MIRROR_TYPE_VERTICALLY         = 2,    // 垂直翻转
    VAVA_MIRROR_TYPE_BOTH               = 3     // 双向都翻转
} VAVA_MIRROR_TYPE;

// 9. 音频编码格式 VAVA_AUDIO_CODEC
typedef enum {
    VAVA_AUDIO_CODEC_PCM        = 0,    // PCM
    VAVA_AUDIO_CODEC_G711A      = 1,    // G711A
    VAVA_AUDIO_CODEC_G711U      = 2,    // G711U
    VAVA_AUDIO_CODEC_AAC        = 3,    // AAC
    VAVA_AUDIO_CODEC_OPUS       = 4     // OPUS
} VAVA_AUDIO_CODEC;

// 10. 音视频传输模式 VAVA_AV_SEND_MODE
typedef enum {
    VAVA_AV_SEND_NORMAL     = 0,    // 正常模式
    VAVA_AV_SEND_LOST_P     = 1,    // 仅传输 I 帧(音频不适用该条)
    VAVA_AV_SEND_LOST_ALL   = 2     // 暂停传输
} VAVA_AV_SEND_MODE;

// 11. 录像模式 VAVA_REC_MODE
typedef enum {
    VAVA_REC_MODE_NORMAL    = 0,    // 开机录像
    VAVA_REC_MODE_TIMER     = 1,    // 定时录像
    VAVA_REC_MODE_ALARM     = 2     // 报警录像
} VAVA_REC_MODE;

// 12. 磁盘类型 VAVA_DISK_TYPE
typedef enum {
    VAVA_DISK_TYPE_EMMC     = 0,    // 内部 EMMC 存储
    VAVA_DISK_TYPE_TF       = 1,    // TF 卡存储
    VAVA_DISK_TYPE_NAS      = 2     // NAS 存储
} VAVA_DISK_TYPE;

// 13. 磁盘状态 VAVA_DISK_STATUS
typedef enum {
    VAVA_DISK_STATUS_NOCRAD     = 0,    // 无有效磁盘
    VAVA_DISK_STATUS_HAVECARD   = 1,    // 有磁盘并且挂载成功，读写正常
    VAVA_DISK_STATUS_BADCARD    = 2,    // 磁盘损坏/异常
    VAVA_DISK_STATUS_UNFORMAT   = 3,    // 磁盘未格式化
    VAVA_DISK_STATUS_FULL       = 4,    // 磁盘已满，不可再写入
    VAVA_DISK_STATUS_READONLY   = 5     // 磁盘只读
} VAVA_DISK_STATUS;

// 14. 设备端在线状态 VAVA_CAMERA_STATUS
typedef enum {
    VAVA_CAMERA_STATUS_OFFLINE  = 0,   // 离线
    VAVA_CAMERA_STATUS_ONLINE   = 1    // 在线
} VAVA_CAMERA_STATUS;

// 15. 录像类型 VAVA_REC_TYPE
// 说明:目前最大支持 32 种类型的报警，每一种类型的报警占一位，多种报警可 以按位组合一起形成最终的报警状态;
typedef enum {
    VAVA_REC_TYPE_ALL           = -1,       // 全部类型录像
    VAVA_REC_TYPE_NORMAL        = 1 << 0,   // 普通录像
    VAVA_REC_TYPE_TIMER         = 1 << 1,   // 定时录像
    VAVA_REC_TYPE_ALARM         = 1 << 2,   // 通用报警录像
    VAVA_REC_TYPE_MANAUL        = 1 << 3,   // 人性报警
    VAVA_REC_TYPE_SNAPSHOT      = 1 << 4,   // 人脸报警
    VAVA_REC_TYPE_MDETECT       = 1 << 5,   // 移动报警
    VAVA_REC_TYPE_HUMAN         = 1 << 6,   // 移动侦测报警
    VAVA_REC_TYPE_FACE          = 1 << 7,   // GSENSOR移动报警
    VAVA_REC_TYPE_TIME_LAPSE    = 1 << 9    // 缩时录影
} VAVA_REC_TYPE;

// 16. 录像回放控制类型 VAVA_REPLAY_CTRL
typedef enum {
    VAVA_REPLAY_CTRL_START          = 0,    // 播放
    VAVA_REPLAY_CTRL_PAUSE          = 1,    // 暂停
    VAVA_REPLAY_CTRL_STOP           = 2,    // 停止
    VAVA_REPLAY_CTRL_CONTINUE       = 3,    // 暂停继续
    VAVA_REPLAY_CTRL_FAST_FOWRD     = 4,    // 快进
    VAVA_REPLAY_CTRL_FAST_BACKWARD  = 5     // 快退
} VAVA_REPLAY_CTRL;

// 17. 录像回放模式 VAVA_RECFILE_TRANSPORT
typedef enum {
    VAVA_RECFILE_TRANSPORT_NORMA    = 0,   // 正常模式(边传边播，有速率控制)
    VAVA_RECFILE_TRANSPORT_FAST     = 1    // 快速模式(文件下载)
} VAVA_RECFILE_TRANSPORT;

// 18. 提示音语言 VAVA_LANGUAGE
typedef enum {
    VAVA_LANGUAGE_ENGLISH   = 0,   // 英文
    VAVA_LANGUAGE_CHINESE   = 1    // 中文
} VAVA_LANGUAGE;

// 19. 夜视模式 VAVA_IRLED_MODE
typedef enum {
    VAVA_IRLED_MODE_CLOSE       = 0,    // 关闭夜视功能
    VAVA_IRLED_MODE_AUTO        = 1,    // 黑白夜视模式
    VAVA_IRLED_MODE_COLORFUL    = 2     // 全彩夜视模式
} VAVA_IRLED_MODE;

// 20. 白色补光灯亮度等级 VAVA_WHIGHTLED_LEVEL
typedef enum {
    VAVA_WHIGHTLED_LEVEL_LOW    = 0,    // 低
    VAVA_WHIGHTLED_LEVEL_MIDDLE = 1,    // 中
    VAVA_WHIGHTLED_LEVEL_HIGH   = 2     // 高
} VAVA_WHIGHTLED_LEVEL;

// 21. 布防撤防类型 VAVA_ARMING_STATUS
typedef enum {
    VAVA_ARMING_STATUS_DISABLE      = 0,    // 撤防
    VAVA_ARMING_STATUS_ENABLE       = 1,    // 布防
    VAVA_ARMING_STATUS_ONTIME_ON    = 2,    // 定时布防(最多添加 10 条)
    VAVA_ARMING_STATUS_ONTIME_OFF   = 3     // 定时布防下撤防
} VAVA_ARMING_STATUS;

// 22. 录像删除模式 VAVA_RECFILE_DEL_FLAG
typedef enum {
    VAVA_RECFILE_DEL_NORMAL     = 0,    // 正常模式(按录像文件列表删除)
    VAVA_RECFILE_DEL_ALLDIR     = 1,    // 文件夹模式(按录像日期删除)
} VAVA_RECFILE_DEL_FLAG;

// 23. OTA 升级包类型 VAVA_UPDATE_TYPE
typedef enum {
    VAVA_UPDATE_TYPE_CAMERA     = 13    // 设备端升级包
} VAVA_UPDATE_TYPE;

// 24. OTA 升级状态 VAVA_UPDATE_STATUS
typedef enum {
    VAVA_UPDATE_IDLE                = 0,    // 空闲状态
    VAVA_UPDATE_START               = 1,    // 进入升级模式
    VAVA_UPDATE_LOADING             = 2,    // 升级包下载中
    VAVA_UPDATE_LOAD_ERR            = 3,    // 升级包下载失败或超时
    VAVA_UPDATE_LOAD_FINISH         = 4,    // 升级包下载完成
    VAVA_UPDATE_UPFILE_NOSUPPORT    = 5,    // 非 VAVA 升级包
    VAVA_UPDATE_CRCERR              = 6,    // 升级包校验失败
    VAVA_UPDATE_TYPE_ERR            = 7,    // 升级包类型错误
    VAVA_UPDATE_FILE_OPEN_FAIL      = 8,    // 升级包打开失败
    VAVA_UPDATE_REQ_FAIL            = 9,    // 发送设备端升级请求失败
    VAVA_UPDATE_RESP_TIMEOUT        = 10,   // 等待设备端响应超时
    VAVA_UPDATE_TRANSMITTING        = 11,   // 正在向设备端传输升级数据
    VAVA_UPDATE_TRANS_FIAL          = 12,   // 向设备端传输升级数据失败
    VAVA_UPDATE_CHECK_TIMEOUT       = 13,   // 设备端校验升级包超时
    VAVA_UPDATE_CHECK_FAIL          = 14,   // 设备端校验升级包失败
    VAVA_UPDATE_TIMEOUT             = 15,   // 设备端升级超时
    VAVA_UPDATE_UPGRADING           = 16,   // 升级中
    VAVA_UPDATE_SUCCESS             = 17,   // 升级成功
    VAVA_UPDATE_FAIL                = 18,   // 升级失败
    VAVA_UPDATE_NOPAIR              = 19,   // 通道未发现设备端(设备端升级)
    VAVA_UPDATE_POWERLOW            = 20    // 电量低(设备端升级)
} VAVA_UPDATE_STATUS;

// 25. 抓图质量 VAVA_SNAPSHOT_QUALITY
typedef enum {
    VAVA_SNAPSHOT_QUALITY_DEFAULT       = 0,    // 默认
    VAVA_SNAPSHOT_QUALITY_HIGH          = 1,    // 高
    VAVA_SNAPSHOT_QUALITY_MIDDLE        = 2,    // 中
    VAVA_SNAPSHOT_QUALITY_LOW           = 3     // 低
} VAVA_SNAPSHOT_QUALITY;

// 26. 配对信息清除类型 VAVA_PAIR_CLEAR
typedef enum {
    VAVA_PAIR_CLEAR_CHANNEL     = 0,    // 清除
    VAVA_PAIR_CLEAR_ALL         = 1     // 清除全部通道
} VAVA_PAIR_CLEAR;

// 27. 配对状态 VAVA_PAIR_STATUS
typedef enum {
    VAVA_PAIR_NONE     = 0,    // 未配对
    VAVA_PAIR_USED     = 1     // 已配对
} VAVA_PAIR_STATUS;

// 28. NAS 服务器状态 VAVA_NAS_STATUS
typedef enum {
    VAVA_NAS_STATUS_IDLE            = 0,    // 空闲
    VAVA_NAS_STATUS_CONFIGING       = 1,    // 正在配置 NAS
    VAVA_NAS_STATUS_SYNC            = 2,    // 自动同步状态
    VAVA_NAS_STATUS_SD_FAIL         = 3,    // 磁盘检测失败
    VAVA_NAS_STATUS_PARAM_FAIL      = 4,    // 参数无效
    VAVA_NAS_STATUS_MOUNT_FAIL      = 5,    // NAS 服务器挂载失败
    VAVA_NAS_STATUS_NOWRITE         = 6,    // NAS 服务器不可写
    VAVA_NAS_STATUS_CONNECT_FAIL    = 7,    // NAS 服务器连接失败
    VAVA_NAS_STATUS_LACKOF_SPACE    = 8     // NAS 空间不足
} VAVA_NAS_STATUS;

// 29. 报警语音开启类型 VAVA_BUZZER_TYPE
typedef enum {
    VAVA_BUZZER_TYPE_OPEN           = 1,    // 持续鸣叫
    VAVA_BUZZER_TYPE_INTERVAL       = 2     // 间隔性鸣叫
} VAVA_BUZZER_TYPE;

// 30. 配对结果类型 VAVA_PAIR_ERRCODE
typedef enum {
    VAVA_PAIR_ERRCODE_SUCCESS       = 0,    // 配对成功
    VAVA_PAIR_ERRCODE_FAIL          = 1,    // 配对失败
    VAVA_PAIR_ERRCODE_TIMEOUT       = 2     // 等待设备端超时
} VAVA_PAIR_ERRCODE;

// 31. WIFI 连接状态 VAVA_WIFI_CONNECT_STATUS
typedef enum {
    VAVA_WIFI_CONNECT_SUCCESS   = 0,    // Wi-Fi 连接成功
    VAVA_WIFI_CONNECT_FAIL      = 1     // Wi-Fi 连接失败
} VAVA_WIFI_CONNECT_STATUS;

// 32. WIFI 热点加密方式 VAVA_WIFI_AP_ENCRYPT
typedef enum {
    VAVA_WIFI_AP_NO_ENCRYPT     = 0,    // 未加密模式
    VAVA_WIFI_AP_WPA2           = 1     // 未加密模式 WPA/WPA2 加密模式
} VAVA_WIFI_AP_ENCRYPT;

//侦测类型
typedef enum{
    VAVA_DETECT_MOVEMNT = 0,    //移动侦测
    VAVA_DETECT_HUMAN   = 1     //人形侦测
}VAVA_DETECT_MODE;

//SD卡状态
typedef enum{
    VAVA_SD_STATUS_NOCRAD       = 0,  //无卡
    VAVA_SD_STATUS_HAVECARD     = 1,  //有卡
    VAVA_SD_STATUS_BADCARD      = 2,  //坏卡
    VAVA_SD_STATUS_UNFORMAT     = 3,  //卡未格式化
    VAVA_SD_STATUS_FULL         = 4,  //卡满
    VAVA_SD_STATUS_READONLY     = 5   //卡写保护（只读）
}VAVA_SD_STATUS;

// 7.2 错误码
#define VAVA_ERR_CODE_SUCCESS               0   // 成功
#define VAVA_ERR_CODE_MEM_MALLOC_FIAL       1   // 申请内存资源失败
#define VAVA_ERR_CODE_JSON_PARSE_FAIL       2   // JSON 解析失败
#define VAVA_ERR_CODE_JSON_NODE_NOFOUND     3   // JSON 节点未找到
#define VAVA_ERR_CODE_JSON_MALLOC_FIAL      4   // JSON 资源申请失败
#define VAVA_ERR_CODE_SESSION_AUTH_FAIL     5   // 会话验证失败
#define VAVA_ERR_CODE_AUTH_KEY_TIMEOUT      6   // 会话 key 已超时
#define VAVA_ERR_CODE_AUTH_KEY_TIMEOUT      6   // 会话 key 已超时
#define VAVA_ERR_CODE_UNAUTHORIZED          7   // 会话未授权
#define VAVA_ERR_CODE_PARAM_INVALID         8   // 参数无效
#define VAVA_ERR_CODE_CAMERA_OFFLINE        9   // 设备端离线
#define VAVA_ERR_CODE_CAMERA_DORMANT        10  // 设备端未唤醒
#define VAVA_ERR_CODE_CAMERA_NOREADY        11  // 设备端未准备就绪
#define VAVA_ERR_CODE_MIC_CLOSED            12  // 麦克被关闭
#define VAVA_ERR_CODE_SPEAKER_CLOSED        13  // 喇叭被关闭
#define VAVA_ERR_CODE_CMD_NOSUPPORT         14  // 不支持的信令
#define VAVA_ERR_CODE_CONFIG_FAIL           15  // 设备端配置失败
#define VAVA_ERR_CODE_CONFIG_TIMEOUT        16  // 设备端配置超时
#define VAVA_ERR_CODE_RECFILE_NOTFOUND      17  // 录像文件未找到
#define VAVA_ERR_CODE_TIME_SET_ERR          18  // 时间配置失败
#define VAVA_ERR_CODE_SD_NOFOUND            19  // 未找到有效磁盘
#define VAVA_ERR_CODE_SD_NEED_FORMAT        20  // 磁盘需要格式化
#define VAVA_ERR_CODE_SD_FORMAT_FAIL        21  // 磁盘格式化失败
#define VAVA_ERR_CODE_SD_MOUNT_FAIL         22  // 磁盘挂载失败
#define VAVA_ERR_CODE_IDX_OPEN_FAIL         23  // 打开索引文件失败
#define VAVA_ERR_CODE_IDX_HEAD_ERR          24  // 索引文件头检测失败
#define VAVA_ERR_CODE_IDX_OPERAT_FAIL       25  // 索引文件操作失败
#define VAVA_ERR_CODE_UPDATE_NOIDLE         26  // 当前处于升级状态
#define VAVA_ERR_CODE_UPDATE_TYPE_ERR       27  // 升级类型不支持
#define VAVA_ERR_CODE_INTO_UPDATE_FAIL      28  // 进入升级模式失败
#define VAVA_ERR_CODE_RECPLAY_REREQ         29  // 当前会话已开启一路回放
#define VAVA_ERR_CODE_RECPLAY_NOIDLE        30  // 无空闲回放通道
#define VAVA_ERR_CODE_RECPLAY_FAIL          31  // 录像回放失败
#define VAVA_ERR_CODE_RECFILE_DAMAGE        32  // 录像文件损坏
#define VAVA_ERR_CODE_TOKEN_FAIL            33  // 录像回放控制 token 校验失败
#define VAVA_ERR_CODE_IP_NOTCONNECT         34  // IP 地址无法连接
#define VAVA_ERR_CODE_PAHT_INVALID          35  // 路径不是有效的 Linux 路径
#define VAVA_ERR_CODE_INSET_LIST_FAIL       36  // 插入队列失败
#define VAVA_ERR_CODE_CAMERA_DISABLE        37  // 设备端被禁用
#define VAVA_ERR_CODE_NOSUPPORT             38  // 不支持该指令
#define VAVA_ERR_CODE_NOAUSH                39  // 无权限
#define VAVA_ERR_CODE_CMD_DOFAIL            30  // 命令执行失败


#endif





