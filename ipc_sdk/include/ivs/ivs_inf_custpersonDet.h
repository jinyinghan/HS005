#ifndef __IVS_INF_CUSTPERSONDET_H__
#define __IVS_INF_CUSTPERSONDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define NUM_OF_PERSONS 10

/*
 * 人形信息角度结构体
 */
typedef struct {
    float yaw;
    float pitch;
    float roll;
}custpreson_face_pose;


/*
 * 人脸信息结构体
 */
typedef struct {
    IVSRect box; /**< 人脸区域坐标,用来做人脸识别 */
    IVSRect show_box; /**< 人脸区域真实坐标 */
    custpreson_face_pose face_pose_res; /**< 人脸角度**/
    float confidence; /**< 人脸检测结果的置信度 */
    float blur_score; /**< 人脸模糊得分 1:模糊 0:清晰 */
}custpreson_face_info;

/*
 * 人形信息结构体
 */
typedef struct {
    IVSRect box; /**< ROI区域坐标 */
    IVSRect show_box; /**< 人形区域真实坐标 */
    float confidence; /**< 人形检测结果的置信度 */
}custperson_info;

/*
 * 人形检测输入结构体
 */
typedef struct {
    bool ptime; /**< 是否打印检测时间 */
    int skip_num; /**< 跳帧数目 */
    unsigned int delay; /**< 延时识别时间，默认为0 */
    IVSRect win; /**< 人形检测区域坐标信息 */
    IVSFrameInfo frameInfo; /**< 帧信息 */
    bool rotate; /**< rotate 270 or -90 */
    unsigned int max_face_box; /**< 人脸检测处理过程中保留的框数量 */
	float scale_factor; /**< 图像缩放倍数: new_size = ori_size * scale_factor */
    bool switch_face_pose; /**< 人脸角度检测模块开关, 当true时会做人脸角度检测, 当false时不做人脸角度检测返回-1 */
    bool switch_face_blur; /**< 人脸模糊检测模块开关, 当true时会做人脸模糊检测, 当false时不做人脸模糊检测返回-1 */
}custpersondet_param_input_t;

/*
 * 人形检测输出结构体
 */
typedef struct {
    int count; /**< 识别出人形的个数 */
    int face_count; /**< out face nums */
    custperson_info person[NUM_OF_PERSONS]; /**< 识别出的人形信息 */
    custpreson_face_info face[NUM_OF_PERSONS]; /**< 识别出的人脸信息 */
    int64_t timeStamp; /**< 时间戳 */
    bool alarm; /**< 报警信号,动作幅度较大或出现新目标时置true否则为false */
}custpersondet_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_CUSTPERSONDET_H__ */
