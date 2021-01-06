#ifndef __IVS_INF_VEHICLEDET_H__
#define __IVS_INF_VEHICLEDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define NUM_OF_PLATES 10

/*
 * 车辆信息结构体
 */
typedef struct {
    IVSRect box; /**< 车辆区域坐标 */
    float confidence; /**< 车辆检测结果的置信度 */
}vehicle_info;

/*
 * 车辆检测输入结构体
 */
typedef struct {
    bool is_roi; /**< 输入的图像是否为检测区域的ROI */
    int skip_num; /**< 跳帧数目 */
    unsigned int delay; /**< 延时识别时间，默认为0 */
    IVSRect win; /**< 车辆检测区域坐标信息 */
    IVSFrameInfo frameInfo; /**< 帧信息 */
}vehicledet_param_input_t;

/*
 * 车辆检测输出结构体
 */
typedef struct {
    int count; /**< 识别出车辆的个数 */
    vehicle_info vehicle[NUM_OF_PLATES]; /**< 识别出的车辆信息 */
    int64_t timeStamp; /**< 时间戳 */
}vehicledet_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_VEHICLEDET_H__ */
