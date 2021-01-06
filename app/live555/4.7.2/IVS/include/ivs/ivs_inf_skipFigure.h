#ifndef __IVS_INF_SKIPFIGURE_H__
#define __IVS_INF_SKIPFIGURE_H__

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
  int skipFrame;	    /**< 隔帧数目 */
  IVSFrameInfo frameInfo;   /**< 帧信息 */
} skipFigure_param_input_t;

/*
 * 人形检测算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect* rects;	/**< 检测到的目标矩形框 */
  int64_t timeStamp; /**< 时间戳 */
} skipFigure_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_SKIPFIGURE_H__ */
