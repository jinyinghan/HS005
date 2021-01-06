#ifndef __IVS_INF_PLATEREC_H__
#define __IVS_INF_PLATEREC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define LEN_OF_COLOR 7
#define LEN_OF_LICENCE 19
#define NUM_OF_PLATES 8

/*
 * 车牌信息结构体
 */
typedef struct {
  IVSRect box; /**< 车牌区域坐标 */
  char color[LEN_OF_COLOR]; /**< 车牌颜色 */
  char licence[LEN_OF_LICENCE]; /**< 车牌号 */
  float confidence; /**< 车牌识别结果的置信度 */
}plate_info;

/*
 * 车牌识别输入结构体
 */
typedef struct {
  bool is_roi; /**< 输入的图像是否为检测区域的ROI */
  bool is_bgra;  /**< 输入的图像格式是否为BGRA */
  int skip_num; /**< 跳帧数目 */
  unsigned int delay; /**< 延时识别时间，默认为0 */
  IVSRect win; /**< 车牌识别区域坐标信息 */
  IVSFrameInfo frameInfo; /**< 帧信息 */
}platerec_param_input_t;

/*
 * 车牌识别输出结构体
 */
typedef struct {
  int count; /**< 识别出车牌的个数 */
  plate_info plate[NUM_OF_PLATES]; /**< 识别出的车牌信息 */
  int64_t timeStamp;  /**< 帧的时间戳*/
  int64_t time_ms;    /**< 车牌识别整体时间(ms)*/

}platerec_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_PLATEREC_H__ */
