#ifndef __IVS_INF_TRACKER_H__
#define __IVS_INF_TRACKER_H__

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
  IVSFrameInfo frameInfo;   /**< 帧信息 */
} tracker_param_input_t;

/*
 * 人形检测算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect rect;	/**< 检测到的目标矩形框 */
  int dx;              /**< 检测到的目标dx */
  int dy;        /**< 检测到的目标dy */
  int step;   /**< 检测到的目标step */
} tracker_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_TRACKER_H__ */
