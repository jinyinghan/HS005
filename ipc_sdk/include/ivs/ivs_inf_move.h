#ifndef __IVS_INF_MOVE_H__
#define __IVS_INF_MOVE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define NUM_OF_RECTS 8

/*
 * 高级移动侦测算法的输入结构体
 */
typedef struct {
  int sense; /**< 高级移动侦测的灵敏度，范围为0-4 */

  int min_h; /**< 高级移动侦测物体的最小高度 */
  int min_w; /**< 高级移动侦测物体的最小宽度 */

  IVSRect* rois; /**< 高级移动侦测待检测的区域信息 */
  int cntRoi; /**< 高级移动侦测待检测区域的数量 */

  int isSkipFrame; /**< 高级移动侦测跳帧处理开关 */
  int isLightRemove; /**< 高级移动侦测光照处理开关 */

  IVSFrameInfo frameInfo; /**< 帧信息 */
}move_param_input_t;

/*
 * 高级移动侦测算法的输出结构体
 */
typedef struct {
  int ret;		/**< 是否检测出移动区域 */
  int count; /**< 检测出移动区域的数量 */
  IVSRect rects[NUM_OF_RECTS]; /**< 检测出移动区域的信息 */
  int blockSize; /**< 检测出移动区域块大小 */
  IVSPoint *pt; /**< 移动区域左上角点坐标 */
}move_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_MOVE_H__ */
