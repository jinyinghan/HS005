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
  int stable_time_out;  /**< 稳定期最大间隔 */
  int move_sense;      /**< 检测灵敏度 0-4 ascend*/
  int obj_min_width;   /**< 最小检测目标大小宽度*/
  int obj_min_height;  /**< 最小检测目标大小高度*/

  int move_thresh_denoise; /**< 降噪处理阈值*/
  int add_smooth; /**< whether open the denoise process*/
  float conf_thresh; /**<跟踪约束，跟踪置信度变化幅度阈值，大于该阈值跳出检测状态，[0,1]*/
  float conf_lower; /**<跟踪约束，跟踪置信度最小阈值，小于该阈值跳出检测状态，[0,1]*/
  int frozen_in_boundary; /**<在左右边界处物体连续移动帧数小于该阈值，保持静止状态*/
  float boundary_ratio; /**<左/右边界占图像宽度比例，[0,1]*/
  int is_feedback_motor_status; /**< 电机回馈信息*/
  int is_motor_stop;            /**< 电机停止状态*/

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
  int capture_lost; /**< 何种状态下跟踪失效 */
  int tracking;  /**< 是否处于跟踪状态 */
  int64_t timeStamp; /**<时间戳*/
} motion_tracker_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_TRACKER_H__ */

