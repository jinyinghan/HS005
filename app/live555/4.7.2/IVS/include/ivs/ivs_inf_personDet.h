#ifndef __IVS_INF_PERSONDET_H__
#define __IVS_INF_PERSONDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define NUM_OF_PERSONS 1024

/*
 * 人形信息角度结构体
 */
typedef struct {
    float yaw;
    float pitch;
    float roll;
}person_pose;

/*
 * 人形信息结构体
 */
typedef struct {
    IVSRect box; /**< 人形区域坐标,用来做人脸识别 */
    IVSRect show_box; /**< 人形区域真实坐标 */
    float confidence; /**< 人形检测结果的置信度 */
}person_info;

/*
 * 人形检测输入结构体
 */
typedef struct {
    bool ptime; /**< 是否打印检测时间 */
    int skip_num; /**< 跳帧数目 */
    unsigned int delay; /**< 延时识别时间，默认为0 */
    IVSRect win; /**< 人形检测区域坐标信息 */
    IVSFrameInfo frameInfo; /**< 帧信息 */
    int rwidth; /**< 检测宽度  */
    int sense; /**< 检测灵敏度 */
}persondet_param_input_t;

/*
 * 人形检测输出结构体
 */
typedef struct {
    int count; /**< 识别出人形的个数 */
    person_info person[NUM_OF_PERSONS]; /**< 识别出的人形信息 */
    int64_t timeStamp; /**< 时间戳 */
}persondet_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_PERSONDET_H__ */
