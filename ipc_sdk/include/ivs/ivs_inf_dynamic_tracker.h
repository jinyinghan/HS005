#ifndef __IVS_INF_DYNAMIC_TRACKER_H__
#define __IVS_INF_DYNAMIC_TRACKER_H__

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
  IVSFrameInfo frameInfo;   /**< å¸§ä¿¡æ¯ */
  int stable_time_out;
  int tracking_object; /**< 0:move, 1:face, 2:human shape*/
  int move_sense;
  int obj_min_width;
  int obj_min_height;

  int skip_rule;
  int skip_duration;

  int is_feedback_motor_status;
  int is_motor_stop;
} dynamic_tracker_param_input_t;

/*
 * 人形检测算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect rect[20];	/**< 检测到的目标矩形框 */
  int dx;              /**< 检测到的目标dx */
  int dy;        /**< 检测到的目标dy */
  int step;   /**< 检测到的目标step */
} dynamic_tracker_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_DYNAMIC_TRACKER_H__ */
