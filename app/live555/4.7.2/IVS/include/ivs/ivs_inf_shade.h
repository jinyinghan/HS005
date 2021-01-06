#ifndef __IVS_INF_SHADE_H__
#define __IVS_INF_SHADE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"
//#define _DEBUG
/*
 * shade 算法的输入结构体
 */

typedef struct {
    int night_sense; /**< sensitivity */
    IVSFrameInfo frameInfo;   /**<     */
    int night_flag; /* day: night_flag = 0, night: night_flag = 1,  dim: night_mode = 2 */
    int start_delay;
    int update_flag; /*update_flag =1:  reset background */
}shade_param_input_t;

/*
 * shade 算法的输出结构体
 */
#ifdef _DEBUG
typedef struct {
    float diff_frame;
    float diff_background;
}ShadeDebugInfo;
#endif

typedef struct {
    int count;
    int shade_enable;
    int flag;  /** 输出结果标志, 0-未遮挡, 1-遮挡 */
    int64_t timeStamp; /**< 时间戳 */
#ifdef _DEBUG
    ShadeDebugInfo debugInfo;
#endif
}shade_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_SHADE_H__ */
