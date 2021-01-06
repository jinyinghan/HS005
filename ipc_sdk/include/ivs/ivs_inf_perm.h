#ifndef __IVS_INF_PERM_H__
#define __IVS_INF_PERM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include "ivs_common.h"

#define IVS_PERM_MAX_ROI  8
#define IVS_PERM_MAX_RECT 8

/*
 * 周界信息结构体
 */
typedef struct{
  IVSPoint *p;    /**< 周界各个顶点信息，不能有线交叉，最多6个点 */
  int pcnt;      /**< 周界顶点数量 */
  uint64_t alarm_last_time;	/**< 持续报警时间 */
}single_perm_t;

/*
 * 周界防范算法的输入结构体
 */
typedef struct{
  single_perm_t perms[IVS_PERM_MAX_ROI];  /**< 周界信息 */
  int permcnt;                            /**< 周界数量 */
  int sense;							  /**< 灵敏度 */

  IVSFrameInfo frameInfo;				  /**< 帧信息 */
}perm_param_input_t;

/*
 * 周界防范算法的输出结构体
 */
typedef struct{
  int ret;								/**< 是否出现越界 */
  int is_alarmed[IVS_PERM_MAX_ROI];     /**< 那个周界出现越界 */
  IVSRect rects[IVS_PERM_MAX_RECT];     /**< 越界物体的矩形信息 */
  int rectcnt;							/**< 越界物体的数量 */
}perm_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_PERM_H__ */
