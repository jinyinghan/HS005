#ifndef __IVS_INF_OPT_FLOW_H__
#define __IVS_INF_OPT_FLOW_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"


typedef struct {
    int zero;
    IVSFrameInfo frameInfo;
} opt_flow_param_input_t;

typedef struct {
    int count;
    IVSPoint* points;
} opt_flow_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_OPT_FLOW_H__ */
