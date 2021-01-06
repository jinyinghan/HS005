#ifndef __IVS_INF_SMOKE_H__
#define __IVS_INF_SMOKE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

/*
 * smoke 算法的输入结构体
 */
typedef struct {
    int zero;
}smoke_param_input_t;

/*
 * smoke 算法的输出结构体
 */
typedef struct {
    int sum;        /**< the number of smoke areas in each frame */
    IVSRect* rects;  /**< the rectangles' information for our target or smoking ROI */
}smoke_param_output_t;

IMPIVSInterface *SmokeInterfaceInit(void* param);
void SmokeInterfaceExit(IMPIVSInterface *inf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_SMOKE_H__ */
