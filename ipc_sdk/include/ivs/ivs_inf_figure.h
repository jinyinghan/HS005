#ifndef __IVS_INF_FIGURE_H__
#define __IVS_INF_FIGURE_H__

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
  int b_mix_face;			/**< 是否混合人脸检测 */
  IVSFrameInfo frameInfo;   /**< 帧信息 */
} figure_param_input_t;

/*
 * 人形检测算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect* rects;	/**< 检测到的目标矩形框 */
  int* ID;              /**< 检测到的目标ID */
  int* score;        /**< 检测到的目标score */
} figure_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_FIGURE_H__ */
