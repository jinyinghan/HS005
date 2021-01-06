#ifndef _QCAM_SMART_H_
#define _QCAM_SMART_H_

//================人形&&人脸检测相关 start======================//
#define SMART_MAX_RECT_NUM		(10)
#define SMART_MAX_FD_JPEG_NUM	(SMART_MAX_RECT_NUM)

typedef struct QCamSmartInParam_t
{
	int smart_enable;		 /* 人形、人脸使能开关, 0: 关闭; 1: 开启*/
	unsigned int width;		 /* 图像的宽，固定640 */
	unsigned int height;	 /* 图像的高，固定360 */
	int smart_pq_enable;	 /* 智能PQ使能开关,设置高级ISP效果的开启状态。高级ISP效果主要优化人脸欠曝和过曝的效果,0: 关闭; 1: 开启 */
	int low_bitrate_enable;  /* 智能PQ低码率使能开关,0: 关闭; 1: 开启 */
	int iso_adaptive_enable; /* 基于ISO的质量码率自适应调节开关,0: 关闭; 1: 开启*/
	int anti_flicker_enable; /* 图像抗闪使能开关,0: 关闭; 1: 开启 */
	float pd_threshold;		 /* 阈值，范围[0.0,1.0)，值越大SMD 告警、人形检测越不灵敏 */
	float fd_threshold;		 /* 阈值，范围[0.0,1.0)，值越大人脸检测越不灵敏 */
} QCamSmartInParam;

typedef struct QCamSmartRect_t
{
	// 坐标都是相对于整个画面的比例，取值范围0~0.999999
	double x1; /* 区域左上角x坐标 */
	double y1; /* 区域左上角y坐标 */
	double x2; /* 区域右下角x坐标 */
	double y2; /* 区域右下角y坐标 */
} QCamSmartRect;

typedef struct QCamSmartTarget_t
{
	int pd_target_num;						   /* 检测到的人形目标个数 */
	int fd_target_num;						   /* 检测到的人脸目标个数 */
	QCamSmartRect pd_rect[SMART_MAX_RECT_NUM]; /* 检测到的人形目标坐标 */
	QCamSmartRect fd_rect[SMART_MAX_RECT_NUM]; /* 检测到的人脸目标坐标 */
	float pd_quality[SMART_MAX_RECT_NUM];      /* 人形目标质量评分，暂时无效，默认0 */
	float fd_quality[SMART_MAX_RECT_NUM];      /* 人脸目标质量评分，范围0 ~ 10 */
	int reserved[8];						   /* 保留 */
} QCamSmartTarget;

typedef struct QCamSmartJpegResult_t
{
	unsigned int width;		/* 抓拍图片实际的宽度 */
	unsigned int height;	/* 抓拍图片实际的高度 */
	char *buffer;			/* 抓拍图片的地址 */
	unsigned int size;		/* 返回实际抓拍图片的大小 */
	unsigned long long pts; /* 时间戳 */
	int reserved[8];		/* 保留 */
} QCamSmartJpegResult;

typedef struct QCamSmartFdJpegInfo_t
{
	unsigned int jpeg_num;									/* 抓拍人脸小图数量，最多为SMART_MAX_FD_JPEG_NUM*/
	QCamSmartJpegResult pJpegResult[SMART_MAX_FD_JPEG_NUM]; /* 抓拍人脸小图结果  */
} QCamSmartFdJpegInfo;

/*****************************************************************************
 函数名		: QCamSmartCreate
 功能描述	: 创建人脸、人形检测功能，其他初始化
 输入参数   : pInParam[in]: 人脸、人形检测算法配置参数的指针
 输出参数   : 无
 返回值     : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartCreate(QCamSmartInParam *pInParam);

/*****************************************************************************
 函数名		: QCamSmartSetPDThreshold QCamSmartSetFDThreshold
 功能描述	: 设置人形检测、人脸检测阈值。可在算法运行中修改
 输入参数   : th[in]: 算法阈值
 输出参数   : 无
 返回值     : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartSetPDThreshold(double th);
int QCamSmartSetFDThreshold(double th);

/*****************************************************************************
 函 数 名  : QCamSmartGetTarget
 功能描述  : 获取人脸、人形目标个数及坐标信息，坐标信息是相对(640 * 360 ) 的坐标体系
 输入参数  : 无
 输出参数  : pTarget[out]: 人脸、人形检测输出目标信息指针
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartGetTarget(QCamSmartTarget *pTarget);

/*****************************************************************************
 函 数 名  : QCamSmartGetTargetV2
 功能描述  : 获取人脸、人形目标个数及坐标信息，坐标信息是相对(640 * 360 ) 的坐标体系，版本二，相比LibXmMaQue_SmartGetTarget 增加人形人脸质量评分。
 输入参数  : 无
 输出参数  : pTarget[out]: 人脸、人形检测输出目标信息指针
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartGetTargetV2(QCamSmartTarget *pTarget);

/*****************************************************************************
 函 数 名  : QCamSmartDestory
 功能描述  : 销毁人脸、人形检测功能
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartDestory();

/*****************************************************************************
 函 数 名  : QCamDrawRect
 功能描述  : 在一帧上画多个矩形框，并清除之前的画框。如果传num=0，表示清除上一帧的所有框
 输入参数  : pRect[in]: 矩形框的坐标
		    num[in]: 框的个数
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamDrawRect(QCamSmartRect *pRect, int num);

/*****************************************************************************
 函 数 名  : QCamSmartCaptureFdJpeg
 功能描述  : 抓拍人脸小图功能
 输入参数  : 
 输出参数  : pFdJpegInfo[out]: 抓拍人脸小图结果指针
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartCaptureFdJpeg(QCamSmartFdJpegInfo *pFdJpegInfo);

//设置AI人形、人脸算法
int QCamSmartSetAiAlg(int enable, QCamSmartInParam *pInParam);

//==================人形&&人脸检测相关 end======================//


//=====================手势识别相关 start=======================//

#define SMART_HGR_TARGET_NUM   (3)

typedef struct
{
	int   enable;		  /* 手势识别使能开关, 0: 关闭; 1: 开启*/
	float threshold;      /* 阈值，范围[0.0, 1.0 ), 值越大越不灵敏，推荐值0.55 */
} QCamSmartHgrInParam;

typedef enum
{
	HGR_PALM_TIGHT,     /* 手掌紧闭 */
	HGR_THUMB_INDEX_L,  /* 大拇指与食指成 L 状态 */
	HGR_THUMB_PINKY_Y,  /* 大拇指与小指成 Y 状态 */
	HGR_UNKNOW          /* 未知的手势类别 */
} QCamHgrType_E;

typedef struct
{
	QCamHgrType_E  hgr_type; /* 手势类型 */
	QCamSmartRect  hgr_rect; /* 手势目标坐标 */
} QCamSmartHgrInfo;

typedef struct
{
	int hgr_target_num; 							 /* 手势目标个数 */
	QCamSmartHgrInfo hgr_info[SMART_HGR_TARGET_NUM]; /* 手势目标信息 */
	int reserved[8];
} QCamSmartHgrTarget;

/*****************************************************************************
 函数名		: QCamSmartHgrCreate
 功能描述	: 创建手势识别功能
 输入参数   : 无
 输出参数   : 无
 返回值     : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartHgrCreate(QCamSmartHgrInParam *pInParam);

/*****************************************************************************
 函数名		: QCamSmartSetHgrParam
 功能描述	: 设置手势检测参数
 输入参数   : enable [in] 配置参数，使能控制。
 			  th     [in] 配置参数，范围[0.0, 1.0 ), 值越大越不灵敏。
 输出参数   : 无
 返回值     : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartSetHgrParam(int enable, float th);

/*****************************************************************************
 函 数 名  : QCamSmartGetHgrTarget
 功能描述  : 获取手势目标个数及坐标信息，坐标信息是相对(640 * 360) 的坐标体系
 输入参数  : 无
 输出参数  : pTarget[out]: 手势检测输出目标信息指针
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartGetHgrTarget(QCamSmartHgrTarget *pTarget);

/*****************************************************************************
 函 数 名  : QCamSmartHgrDestory
 功能描述  : 销毁手势识别功能
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int QCamSmartHgrDestory();

//=====================手势识别相关 end=======================//
int create_smt_draw();
void destroy_smt_draw();

typedef struct 
{
   int x1;     
   int y1;   
   int x2; 
   int y2;
}Rect_t;
typedef struct 
{
        int pd_num;                         /* 靠靠靠靠靠 */
        Rect_t pd_rect[SMART_MAX_RECT_NUM]; /* 靠靠靠靠靠 */
        int score;      /* 靠靠靠靠靠靠靠靠0 */
}Figure_Target;
typedef struct {
    pthread_t fig_thread_id ;
    ///IMPIVSInterface *inteface ;
    int figure_exit ;
    pthread_mutex_t smart_lock;
    Figure_Target ft;
    int ivs_enable;
}smart_init_t ;

#endif
