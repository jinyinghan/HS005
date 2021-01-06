#ifndef __IVS_INF_CLOUD_TRACKER_H__
#define __IVS_INF_CLOUD_TRACKER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define NUM_OF_RECTS 8

/*
 * 人形追踪算法的输入结构体
 */
typedef struct {
  int sense; /**< 人形追踪侦测的灵敏度，范围为0-4 */

  int min_h; /**< 人形追踪侦测物体的最小高度 */
  int min_w; /**< 人形追踪侦测物体的最小宽度 */

  IVSRect* rois; /**< 人形追踪侦测待检测的区域信息 */
  int cntRoi; /**< 人形追踪侦测待检测区域的数量 */

  int isSkipFrame; /**< 人形追踪侦测跳帧处理开关 */
  int isLightRecloud_tracker; /**< 人形追踪侦测光照处理开关 */

  IVSFrameInfo frameInfo; /**< 帧信息 */
  int transOneInterval;    /**<隔帧传输结果，计算不隔帧*/

}cloud_tracker_param_input_t;

/*
 * 人形追踪算法的输出结构体
 */
typedef struct {
  int ret;		/**< 是否检测出移动区域 */
  int count; /**< 检测出移动区域的数量 */
  IVSRect rects[NUM_OF_RECTS]; /**< 检测出移动区域的信息 */
  int blockSize; /**< 检测出移动区域块大小 */
  IVSPoint *pt; /**< 移动区域左上角点坐标 */
  int ID[NUM_OF_RECTS];              /**track ID */
  //  int* score;        /**score */
}cloud_tracker_param_output_t;

/*
 * 人形追踪目标数据编码算法的输入结构体
 */
typedef struct
{
    int Main_Width;/**< 主码流图像宽度 */
	int Main_Height;/**< 主码流图像高度 */
	int Second_Width;/**< 次码流图像宽度 */
	int Second_Height;/**< 次码流图像高度 */
	char* data;/**< 数据指针 */
	IVSRect* rect;/**< 人形追踪目标区域的信息 */
}cloud_tracker_encode_in;
/*
 * 人形追踪目标数据编码算法的输出结构体
 */
typedef struct
{
	int width;/**< jpege格式图像宽度 */
	int height;/**< jpege格式图像高度 */
	int size;/**< jpege格式图像大小 */
	int key[48];/**< jpege格式图像验证信息 */
	char* data;/**< jpege格式图像数据指针 */
}cloud_tracker_encode_out;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_CLOUD_TRACKER_H__ */
