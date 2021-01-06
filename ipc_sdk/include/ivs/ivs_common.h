#ifndef __IVS_COMMON_H__
#define __IVS_COMMON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include <stdbool.h>

/*
 * 输入帧的信息
 */
typedef struct {
    unsigned char *data; /**< 帧的起始地址 */
    int width;		 /**< 帧分辨率的宽 */
    int height;		 /**< 帧分辨率的高 */
    int pixfmt;		 /**< 帧的图像格式 */
	int64_t timeStamp; /**< 帧的时间戳 */
}IVSFrame;

typedef IVSFrame IVSFrameInfo;

/*
 * 点的定义
 */
typedef struct{
    int x;    /**< 横向的坐标值 */
    int y;    /**< 纵向的坐标值 */
}IVSPoint;

/*
 * 矩形框的定义
 */
typedef struct{
  IVSPoint ul;    /**< 左上角点坐标 */
  IVSPoint br;    /**< 右下角点坐标 */
}IVSRect;

/*
 * 线的定义
 */
typedef struct{
  IVSPoint p0;    /**< 线第一个端点坐标 */
  IVSPoint p1;    /**< 线第二个端点坐标 */
}IVSLine;

typedef struct
{
    unsigned char *data; /**< 音频帧数据地址 */
	int len;				 /**< 音频帧长度 */
	int seq;                /**< 音频帧序号 */
	int soundmode;      /**< 音频声道模式 1 mono 2 stereo*/
	int64_t timeStamp; /**< 帧的时间戳 */
} IVSAudio;
typedef IVSAudio IVSAudioInfo;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_COMMON_H__ */
