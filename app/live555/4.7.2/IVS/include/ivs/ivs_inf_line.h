#ifndef __IVS_INF_LINE_H__
#define __IVS_INF_LINE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define IVS_MAX_SIZE 4

/*
 * 越线检测单线检测设置信息
 */
typedef struct {
  int min_h;		/**< 最小越线物体高度,>=0 */
  int min_w;		/**< 最小越线物体宽度,>=0 */
  int sense;		/**< 检测灵敏度,[0,5] */
  int mode;       /**< 越线方向 0:触线 1:逆时针 2: 顺时针 3:双向. 给定警报线的两个端点，以x轴坐标较小的点为圆心，若x轴坐标相等则以y轴坐标最小的点为圆心画圆，沿逆时针轨迹穿越警报线即为mode1, 反之，沿顺时针轨迹穿越警报线为mode2*/
  IVSLine line;			/**< 检测线信息 */
  uint64_t alarm_time_last;/**< 检测线报警持续时间(ms) */
}IVSlineDetector;

/*
 * 越线检测算法的输入结构体
 */
typedef struct{
	int height;    /**< 输入图像高度 */
	int width;    /**< 输入图像宽度 */
	int sense;    /**< 整体检测灵敏度, [0,3] */
	int skipFrameCnt;    /**< 跳帧数目, >=0, mode[1,3]只支持skipFrameCnt<=1 */
	int cntLine;    /**< 需要检测几条线 */
	IVSlineDetector lineDetector[IVS_MAX_SIZE];	/**< 所有检测线信息 */
}line_param_input_t;

/*
 * 越线检测算法的输出结构体
 */
typedef struct{
  int ret;							/**< 针对跳帧检测设置的参数，1表示该帧被处理，反之0表示该帧并未处理，被跳过*/
  int is_alarmed[IVS_MAX_SIZE];		/**< 线报警信息 */
  int count;						/**< 检测出越线物体的数量, for debug */
  IVSRect rects[20];				/**< 检测出越线物体的区域大小, for debug */
}line_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_LINE_H__ */

