#ifndef __IVS_INF_IMAGE_MATCH_H__
#define __IVS_INF_IMAGE_MATCH_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

/*
 * 人形检测算法的输入结构体
 */
typedef struct {
	IVSFrameInfo frameInfo;
	double match_rate_thresh;
	int skip;
} image_match_param_input_t;

/*
 * 人形检测算法的输出结构体
 */
typedef struct {
	double sRate;
	double rRate;
} image_match_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_IMAGE_MATCH_H__ */
