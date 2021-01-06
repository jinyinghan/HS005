#ifndef __IVS_INF_LPR_H__
#define __IVS_INF_LPR_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define NUM_OF_RECTS 8

/*
 * lpr 算法的输入结构体
 */
typedef struct {
	IVSFrameInfo frameInfo;
} lpr_param_input_t;

/*
 * lpr 算法的输出结构体
 */
typedef struct {
	int count;
	IVSRect rects[NUM_OF_RECTS];
} lpr_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif	/*__IVS_INF_LPR_H__*/
