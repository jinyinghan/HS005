//-----------------------------------------------------
// 360Cam 硬件抽象层
// 视频采集 Video Input
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------

#ifndef _QCAM_VIDEO_CTRL_H
#define _QCAM_VIDEO_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif


// H264视频采集回调
// tv 定义为采集时的时戳，gettimeofday(&tv, NULL);  必须真实
// data 为NALU数据（以 00 00 00 01开始的）
// keyframe 为关键帧标志。如果是关键帧，data需要同时包含NALU SPS、PPS 
typedef void (*QCam_Video_Input_cb)(const struct timeval *tv, const void *data, const int len, const int keyframe);

// 视频流分辨率
typedef enum
{
	QCAM_VIDEO_RES_INVALID = 0,
	QCAM_VIDEO_RES_720P,		// 分辨率 1280x720
	QCAM_VIDEO_RES_480P,		// 分辨率 720x480
	QCAM_VIDEO_RES_360P,		// 分辨率 640x360
	QCAM_VIDEO_RES_1080P,       // 分辨率 1920x1080 
	QCAM_VIDEO_RES_1296P,  		// 分辨率 2304x1296
	QCAM_VIDEO_RES_300W,		// 2048 * 1520
	QCAM_VIDEO_RES_400W,		// 2592 * 1520
	QCAM_VIDEO_RES_500W		// 2592 * 1944
}QCAM_VIDEO_RESOLUTION;

typedef enum
{
	QCAM_VIDEO_NIGHT = 0,
	QCAM_VIDEO_DAY,
}QCAM_VIDEO_HASLIGHT;

typedef enum
{
	QCAM_IR_MODE_UNSUPPORT   = -1, // 硬件不支持红外
	QCAM_IR_MODE_AUTO        = 0,  // 自动红外。由SDK层自动切换
	QCAM_IR_MODE_ON          = 1,  // 强制开红外，SDK不自动切换
	QCAM_IR_MODE_OFF         = 2,  // 强制关红外，SDK不自动切换
	QCAM_IR_MODE_AUTO_COLOR  = 3,  // 自动全彩夜视，由SDK层自动切换+业务层开、关灯
	QCAM_IR_MODE_SMART_COLOR = 4   // 智能全彩夜视，夜间检测到画面变化切换到全彩画面
}QCAM_IR_MODE;

typedef enum
{
	QCAM_VIDEO_H264 = 0,  // H264
	QCAM_VIDEO_H265 = 1   // H265
}QCAM_VIDEO_ENCODE;

// 视频采集-频道属性
// 因为不同设备硬件不一样, 所以IQ由QCAM层内部调整
// 外部抽象层只设置基本参数
typedef struct QCamVideoInputChannel_t
{
	int channelId;				// channel ID，用于控制接口，如修改bitrate
	QCAM_VIDEO_RESOLUTION  res;	// resolution
	int fps;					// fps
	int bitrate;				// h264 bitrate (kbps)
	int gop;					// h264 I帧间隔（秒），如果sdk有自动降帧逻辑，要保证I帧间隔不变。
	short vbr;					// VBR=1, CBR=0
	short encoding;				// QCAM_VIDEO_H264/QCAM_VIDEO_H265
	QCam_Video_Input_cb cb;		// callback
}QCamVideoInputChannel;


// 使用流程：Open -> Add多个频道参数->Start(). 最终用完 Close();
int QCamVideoInput_Init();
int QCamVideoInput_AddChannel(QCamVideoInputChannel ch);
int QCamVideoInput_Start();
int QCamVideoInput_Uninit();


// 调整比特率
// isVBR: 0 -- CBR, 1 -- VBR或AVBR
int QCamVideoInput_SetBitrate(int channel, int bitrate, int isVBR);

// 调整分辨率  resolution: 参考QCAM_VIDEO_RESOLUTION
int QCamVideoInput_SetResolution (int channel, int resolution);

// 调整帧率gop : 参考QCamVideoInputChannel 中的gop
int QCamVideoInput_SetFps(int channel, int fps, int gop);
//设置视频编码类型，codec取值范围QCAM_VIDEO_ENCODE。
int QCamVideoInput_SetCodec(int channel, int codec);

// 图像倒置，全局生效
int QCamVideoInput_SetInversion(int enable);

// 强制下次出I帧
int QCamVideoInput_SetIFrame(int channel);


// OSD
typedef struct QCamVideoInputOSD_t
{
	int pic_enable;			// 打开/关闭 图片水印 OSD
	char pic_path[128];		// 图片路径
	int pic_x;				// 图片在屏幕的显示位置（左上角坐标）
	int pic_y;

	int time_enable;		// 打开、关闭 时戳OSD
	int time_x;				// 时戳在屏幕的显示位置（左上角坐标）
	int time_y;
}QCamVideoInputOSD;


int QCamVideoInput_SetOSD(int channel, QCamVideoInputOSD *pOsdInfo);

// 抓图
// bufLen 输入参数为buf最大长度(API应检查长度是否足够，不够的话返回失败)。
// bufLen 同时也是输出参数，为实际长度
// 截图规格：分辨率720p 要求有水印
// 画质：720p图片大概100KB
// 返回QCAM_FAIL/QCAM_OK;
int QCamVideoInput_CatchJpeg(char *buf, int *bufLen);

// 抓YUV。格式为 YUV420 Planar
// buf: Y数据在buf前面，大小为(W*H)字节，UV数据在Y数据后面，大小为(W*H/2)字节
// size: 外界可以传入w*h来仅获取Y，也可以传入w*h*1.5来获取全部的
// 返回QCAM_FAIL/QCAM_OK;
int QCamVideoInput_CatchYUV(int w, int h, char *buf, int bufLen);

// 检测环境是否有光亮
// return 1=有光 0=没有光 -1=检测失败
int QCamVideoInput_HasLight();


// 红外控制相关接口
// SDK负责检测光照条件（硬光敏或软光敏或其他方式），当判断需要切夜视模式或者切白天模式时回调上层接口
// 上层接口根据自身产品需求，决定执行何种操作（切红外模式，或开白光灯，或其他操作，或不操作）

// 白天/黑夜切换回调
// has_light 为 QCAM_VIDEO_NIGHT 表示当前检测到光照条件进入了黑夜
// has_light 为 QCAM_VIDEO_DAY 表示当前检测到光照条件进入了白天
typedef void (*QCam_Light_Detect_cb)(int has_light);

// 设置全局唯一的光照条件检测回调
// 返回QCAM_FAIL/QCAM_OK
int QCamSetLightDetectCallback(QCam_Light_Detect_cb cb);

// 操作红外灯
// ir 为 0，关闭红外灯，ir-cut切回正常状态
// ir 为 1，打开红外灯，ir-cut切到红外模式
// 返回QCAM_FAIL/QCAM_OK
int QCamSetIRCut(int ir);

// 临时挂起软光敏算法（不需要可以不实现）
// 用于云台机进入休眠模式，此时软光敏算法通知上层应该切夜视了
// 但是上层逻辑会强制处于白天模式（避免打开红外灯增加功率和发热）
// 退出休眠模式时，恢复正常夜视切换逻辑
// suspend 为 1，进入休眠状态
// suspend 为 0，退出休眠状态
// 返回QCAM_FAIL/QCAM_OK
int QCamSuspendLightDetect(int suspend);

// 区别软红外和软白光模式的专用接口,当前是软白光（彩色），软红外模式（彩色黑白），部分型号支持。
// mode取值:  
// 0: 硬件不支持红外
// 1: 软红外模式
// 2: 软白光模式
// 3: 强制开启夜视模式
// 4: 强制关闭夜视模式(即强制彩色模式)
// 返回QCAM_FAIL/QCAM_OK
int QCamSetDNCMode(int mode);

// 操作白光灯
// mode 为 0，白光灯关闭
// mode 为 1，白光灯常亮
// mode 为 2，白光灯0.2秒闪一次
// mode 为 3，白光灯0.5秒闪一次
// mode 为 4，白光灯1秒闪一次
// 返回QCAM_FAIL/QCAM_OK
int QCamSetWhiteLight(int mode);

// 设置白光灯亮度
// level表示亮度档位（1~5），档位越高灯越亮
// 返回QCAM_FAIL/QCAM_OK
int QCamSetWhiteLightBrightness(int level);

int QCamGetSensorType(char * sensor);

//设置红外开关，部分型号支持。
//nOn ： 0 -- > 两组红外全关
//       1 -- > 两组红外全开
//       2 -- > 上面一组红外开，下面一组红外关
//       3 -- > 上面一组红外关，下面一组红外开
int QCamSetInfrared(int nOn);

// 宽动态配置
// wdr 为 0，宽动态关闭
// wdr 为 1，宽动态开启
int QCamSetWdr(int wdr);

void QCamSetIRMode(QCAM_IR_MODE mode);

// 返回当前红外控制模式。
QCAM_IR_MODE QCamGetIRMode();

typedef enum
{
	NIGHT_MODE,
    DAY_MODE,
	UNKNOW_MODE,
} QCAM_IR_STATUS;

QCAM_IR_STATUS QCamGetIRStatus();


#ifdef __cplusplus
}
#endif

#endif
