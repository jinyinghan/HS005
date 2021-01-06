#ifndef __IVS_INF_SKIPFACE_H__
#define __IVS_INF_SKIPFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

/*
 * 人脸检测算法的输入结构体
 */
typedef struct {
  int zero;					/**< 预留信息 */
  IVSFrameInfo frameInfo;   /**< 帧信息 */
  int skipFrame;
  bool isMove;
} skipFace_param_input_t;


/*
 * 人脸检测算法的输出结构体
 */
typedef struct {
  int count;		/**< 检测到的矩形框目标个数 */
  IVSRect* rects;	/**< 检测到的目标矩形框 */
  int64_t timeStamp; /**< 时间戳*/
} skipFace_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_SKIPFACE_H__ */
