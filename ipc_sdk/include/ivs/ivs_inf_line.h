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
  IVSLine line;			/**< 检测线信息 */
  uint64_t alarm_time_last;/**< 检测线报警持续时间 */
  int min_h;		/**< 最小越线物体高度 */
  int min_w;		/**< 最小越线物体宽度 */
  int sense;		/**< 检测灵敏度 */
  int ratio;		/**< 越显物体长宽最小限定值 */
}IVSlineDetector;

/*
 * 越线检测算法的输入结构体
 */
typedef struct{
  IVSlineDetector lineDetector[IVS_MAX_SIZE];	/**< 所有检测线信息 */
  int cntLine;									/**< 需要检测几条线 */
}line_param_input_t;

/*
 * 越线检测算法的输出结构体
 */
typedef struct{
  int ret;							/**< 是否出现报警 */
  int is_alarmed[IVS_MAX_SIZE];		/**< 那条线报警 */
  int count;						/**< 检测出越线物体的数量 */
  IVSRect rects[20];				/**< 检测出越线物体的区域大小 */
}line_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_LINE_H__ */

