#ifndef __IVS_INF_SHADE_H__
#define __IVS_INF_SHADE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

/*
 * shade 算法的输入结构体
 */

typedef struct {
    int zero; /**< sensitivity */
}shade_param_input_t;

/*
 * shade 算法的输出结构体
 */

typedef struct {
    int count;
    int flag;
}shade_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_SHADE_H__ */
