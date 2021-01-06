#ifndef __IVS_INF_HEAD_H__
#define __IVS_INF_HEAD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

/*
 * head 算法的输入结构体
 */
typedef struct {
  int zero;
} head_param_input_t;

/*
 * head 算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect* rects;	/**< 检测到的目标矩形框 */
} head_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_HEAD_H__ */
