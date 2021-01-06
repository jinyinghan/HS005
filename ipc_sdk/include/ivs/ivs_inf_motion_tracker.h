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
 * 移动跟踪算法的输入结构体
 */
typedef struct {
  IVSFrameInfo frameInfo;   /**< 帧信息 */
  int stable_time_out;
  int tracking_object; /**< 0:move, 1:face, 2:human shape*/
  int move_sense;
  int obj_min_width;
  int obj_min_height;

  int skip_rule;
  int skip_duration;

}motion_tracker_param_input_t;

/*
 * 移动跟踪算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect rect[20];	/**< 检测到的目标矩形框 */
  int dx;              /**< 检测到的目标dx */
  int dy;        /**< 检测到的目标dy */
  int step;   /**< 检测到的目标step */
} motion_tracker_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_TRACKER_H__ */
