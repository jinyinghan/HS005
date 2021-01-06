#ifndef __IVS_INF_REGION_H__
#define __IVS_INF_REGION_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define IVS_MAX_SIZE 1

/*
 * 区域的定义
 */
typedef struct{
  IVSPoint p0;    /**< 左上点坐标 */
  IVSPoint p1;    /**< 左下点坐标 */
  IVSPoint p2;    /**< 右下点坐标 */
  IVSPoint p3;    /**< 右上点坐标 */
}IVSRegion;

/*
 * 单区域检测设置信息
 */
typedef struct {
  int min_h;		/**< 最小穿越物体高度,>=0 */
  int min_w;		/**< 最小穿越物体宽度,>=0 */
  int sense;		/**< 检测灵敏度,[0,5] */
  IVSRegion region;			/**< 检测穿越信息 */
  uint64_t alarm_time_last;/**< 报警持续时间(ms) */
}IVSregionDetector;

/*
 * 区域检测算法的输入结构体
 */
typedef struct{
	int height;    /**< 输入图像高度 */
	int width;    /**< 输入图像宽度 */
	int sense;    /**< 整体检测灵敏度, [0,3] */
	int mode;       /**< 方向 0:进入 1:出去 2:双向*/
	int skipFrameCnt;    /**< 只支持0<=skipFrameCnt<=1 */
	int cntRegion;    /**< 需要检测几个区域 */
	IVSregionDetector regionDetector[IVS_MAX_SIZE];	/**< 所有检测区域信息 */
}region_param_input_t;

/*
 * 区域检测算法的输出结构体
 */
typedef struct{
  int ret;							/**< 是否是计算后的结果 */
  int is_alarmed[IVS_MAX_SIZE];		/**< 区域报警信息 */
  int count;						/**< 检测出物体的数量, for debug */
  IVSRect rects[20];				/**< 检测出物体的区域大小, for debug */
}region_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_REGION_H__ */

