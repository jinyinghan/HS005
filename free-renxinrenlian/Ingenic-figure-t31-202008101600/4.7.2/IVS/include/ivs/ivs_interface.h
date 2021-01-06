#ifndef __IVS_INTERFACE_H__
#define __IVS_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include <imp/imp_common.h>
#include <imp/imp_audio.h>
#include <imp/imp_ivs.h>

/**
 * IVS功能枚举定义
 */
typedef enum {
	IVS_MOVE_DETECT = 0,                            /**< 移动侦测 */
	IVS_LINE_DETECT,                                /**< 越线检测 */
	IVS_PERM_DETECT,                                /**< 周界防范 */
	IVS_REGION_DETECT,                              /**< 区域检测 */
	IVS_FIGURE_DETECT,                              /**< CV人形检测 */
	IVS_SKIPFIGURE_DETECT,                          /**< CV隔frame人脸检测 */
	IVS_FACE_DETECT,                                /**< CV人脸检测 */
	IVS_FACEDET_DETECT,                             /**< DL人脸检测 */
	IVS_PERSON_DETECT,                              /**< DL人形检测 */
	IVS_PROPERSON_DETECT,                           /**< DL专业人形检测 */
	IVS_CUSTPERSON_DETECT,                          /**< DL定制人形检测 */
	IVS_SITPOSDET_DETECT,                           /**< DL坐姿检测 */
	IVS_SKIPFACE_DETECT,                            /**< CV隔frame人脸检测 */
	IVS_CAR_DETECT,                                 /**< CV车辆检测 */
	IVS_VEHICLEDET_DETECT,                          /**< DL车辆检测 */
	IVS_PETDET_DETECT,                              /**< DL宠物检测 */
	IVS_PETBODYDET_DETECT,                          /**< DL宠物body检测 */
	IVS_PETFACEDET_DETECT,                          /**< DL宠物face检测 */
	IVS_DOGFACEDET_DETECT,                          /**< DL dog face宠物检测 */
	IVS_CATFACEDET_DETECT,                          /**< DL cat face检测 */
	IVS_SMOKE_DETECT,                               /**< 烟雾检测 */
	IVS_SHADE_DETECT,                               /**< 遮挡检测 */
	IVS_HEAD_DETECT,                                /**< 人数统计 */
	IVS_LPR_DETECT,                                 /**< 车牌检测 */
	IVS_DYNAMIC_TRACKER_DETECT,                     /**< 动态运动追踪检测 */
	IVS_MOTION_TRACKER_DETECT,                      /**< 静态运动追踪检测 */
	IVS_FAKE_DETECT,                                /**< 虚假检测 */
	IVS_OPT_FLOW_DETECT,                            /**< 光流检测 */
	IVS_CLOUD_TRACKER_DETECT,                       /**< 云端多目标跟踪检测 */
	IVS_PLATEREC_DETECT,                            /**< 车牌识别 */
	IVS_FALL_DETECT_DETECT,                         /**< 跌倒检测 */
	IVS_IMAGE_MATCH_DETECT,                         /**< 图像匹配检测 */
    IVS_BARCODE,                                    /**< 条形码识别 */
	IVS_INGENIC_END,                                /**< 君正内部最大功能号 */
	IVS_FUNC_MAX  = 1024,                           /**< 最大功能枚举值 */
} IMPIVSFuncEnum;

/**
 * @fn IMPIVSInterface *FaceInterfaceInit(void *param)
 *
 * CV人脸检测接口初始化结构体
 *
 * @param[in] CV人脸检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回CV人脸检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *FaceInterfaceInit(void *param);

/**
 * @fn void FaceInterfaceExit(IMPIVSInterface *inf)
 *
 * CV人脸检测接口去初始化结构体
 *
 * @inf[in] CV人脸检测接口结构体指针句柄
 *
 * @attention 无。
 */
void FaceInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *FigureInterfaceInit(void *param)
 *
 * CV人形检测接口初始化结构体
 *
 * @param[in] CV人形检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回CV人形检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *FigureInterfaceInit(void *param);

/**
 * @fn void FigureInterfaceExit(IMPIVSInterface *inf)
 *
 * CV人形检测接口去初始化结构体
 *
 * @inf[in] CV人形检测接口结构体指针句柄
 *
 * @attention 无。
 */
void FigureInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *PersonDetInterfaceInit(void *param)
 *
 * DL人形检测接口初始化结构体
 *
 * @param[in] DL人形检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回DL人形检测接口结构体指针句柄
 *
 * @attention 无。
 */

IMPIVSInterface *PersonDetInterfaceInit(void* param);

/**
 * @fn void PersonDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL人形检测接口去初始化结构体
 *
 * @inf[in] DL人形检测接口结构体指针句柄
 *
 * @attention 无。
 */
void PersonDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *ProPersonDetInterfaceInit(void *param)
 *
 * DL专业人形检测接口初始化结构体
 *
 * @param[in] DL专业人形检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回DL专业人形检测接口结构体指针句柄
 *
 * @attention 无。
 */

IMPIVSInterface *ProPersonDetInterfaceInit(void* param);

/**
 * @fn void ProPersonDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL专业人形检测接口去初始化结构体
 *
 * @inf[in] DL专业人形检测接口结构体指针句柄
 *
 * @attention 无。
 */
void ProPersonDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *CustPersonDetInterfaceInit(void *param)
 *
 * DL定制人形检测接口初始化结构体
 *
 * @param[in] DL定制人形检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回DL定制人形检测接口结构体指针句柄
 *
 * @attention 无。
 */

IMPIVSInterface *CustPersonDetInterfaceInit(void* param);

/**
 * @fn void CustPersonDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL定制人形检测接口去初始化结构体
 *
 * @inf[in] DL定制人形检测接口结构体指针句柄
 *
 * @attention 无。
 */
void CustPersonDetInterfaceExit(IMPIVSInterface *inf);
/**
 * @fn IMPIVSInterface *FigureInterfaceInit(void *param)
 *
 * 光流人形检测接口初始化结构体
 *
 * @param[in] 光流人形检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回光流人形检测接口结构体指针句柄
 *
 * @attention 无。
 */

IMPIVSInterface *OptFlowInterfaceInit(void* param);

/**
 * @fn void FigureInterfaceExit(IMPIVSInterface *inf)
 *
 * 光流检测接口去初始化结构体
 *
 * @inf[in] 光流检测接口结构体指针句柄
 *
 * @attention 无。
 */

void OptFlowInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *MoveInterfaceInit(void *param)
 *
 * 移动侦测接口初始化结构体
 *
 * @param[in] 移动侦测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回移动侦测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *MoveInterfaceInit(void *param);

/**
 * @fn void MoveInterfaceExit(IMPIVSInterface *inf)
 *
 * 移动侦测接口去初始化结构体
 *
 * @inf[in] 移动侦测接口结构体指针句柄
 *
 * @attention 无。
 */
void MoveInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *LineInterfaceInit(void *param)
 *
 * 越线检测接口初始化结构体
 *
 * @param[in] 越线检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回越线检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *LineInterfaceInit(void *param);

/**
 * @fn void LineInterfaceExit(IMPIVSInterface *inf)
 *
 * 越线检测接口去初始化结构体
 *
 * @inf[in] 越线检测接口结构体指针句柄
 *
 * @attention 无。
 */
void LineInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *RegionInterfaceInit(void *param)
 *
 * 区域检测接口初始化结构体
 *
 * @param[in] 区域检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回区域检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *RegionInterfaceInit(void *param);

/**
 * @fn void RegionInterfaceExit(IMPIVSInterface *inf)
 *
 * 区域检测接口去初始化结构体
 *
 * @inf[in] 区域检测接口结构体指针句柄
 *
 * @attention 无。
 */
void RegionInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *PermInterfaceInit(void *param)
 *
 * 周界防范接口初始化结构体
 *
 * @param[in] 周界防范初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回周界防范接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *PermInterfaceInit(void *param);

/**
 * @fn void PermInterfaceExit(IMPIVSInterface *inf)
 *
 * 周界防范接口去初始化结构体
 *
 * @inf[in] 周界防范接口结构体指针句柄
 *
 * @attention 无。
 */
void PermInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *SmokeInterfaceInit(void *param)
 *
 * 烟雾检测接口初始化结构体
 *
 * @param[in] 烟雾检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回烟雾检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *SmokeInterfaceInit(void* param);

/**
 * @fn void SmokeInterfaceExit(IMPIVSInterface *inf)
 *
 * 烟雾检测接口去初始化结构体
 *
 * @inf[in] 烟雾检测接口结构体指针句柄
 *
 * @attention 无。
 */
void SmokeInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *HeadInterfaceInit(void *param)
 *
 * 人头检测接口初始化结构体
 *
 * @param[in] 人头检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回人头检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *HeadInterfaceInit(void *param);

/**
 * @fn void HeadInterfaceExit(IMPIVSInterface *inf)
 *
 * 人头检测接口去初始化结构体
 *
 * @inf[in] 人头检测接口结构体指针句柄
 *
 * @attention 无。
 */
void HeadInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *ShadeInterfaceInit(void *param)
 *
 * 遮挡检测接口初始化结构体
 *
 * @param[in] 遮挡检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回遮挡检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *ShadeInterfaceInit(void *param);

/**
 * @fn void ShadeInterfaceExit(IMPIVSInterface *inf)
 *
 * 遮挡检测接口去初始化结构体
 *
 * @inf[in] 遮挡检测接口结构体指针句柄
 *
 * @attention 无。
 */
void ShadeInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *LprInterfaceInit(void *param)
 *
 * 车牌检测接口初始化结构体
 *
 * @param[in] 车牌检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回车牌检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *LprInterfaceInit(void *param);

/**
 * @fn void LprInterfaceExit(IMPIVSInterface *inf)
 *
 * 车牌检测接口去初始化结构体
 *
 * @inf[in] 车牌检测接口结构体指针句柄
 *
 * @attention 无。
 */
void LprInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *TrackerInterfaceInit(void* param)
 *
 * 人形追踪检测接口初始化结构体
 *
 * @param[in] 人形追踪检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回人形追踪检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *TrackerInterfaceInit(void* param);

/**
 * @fn void TrackerInterfaceExit(IMPIVSInterface *inf)
 *
 * 人形追踪检测接口去初始化结构体
 *
 * @inf[in] 人形追踪检测接口结构体指针句柄
 *
 * @attention 无。
 */
void TrackerInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *Motion_TrackerInterfaceInit(void* param)
 *
 * 静态运动追踪检测接口初始化结构体
 *
 * @param[in] 静态运动追踪检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回静态运动追踪检测接口结构体指针句柄
 *
 * @attention 适用于全景摄像机。
 */
IMPIVSInterface *Motion_TrackerInterfaceInit(void* param);

/**
 * @fn void Motion_TrackerInterfaceExit(IMPIVSInterface *inf)
 *
 * 静态运动追踪检测接口去初始化结构体
 *
 * @inf[in] 静态运动追踪检测接口结构体指针句柄
 *
 * @attention 适用于全景摄像机。
 */
void Motion_TrackerInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *Dynamic_TrackerInterfaceInit(void* param)
 *
 * 动态运动追踪检测接口初始化结构体
 *
 * @param[in] 动态运动追踪检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回动态运动追踪检测接口结构体指针句柄
 *
 * @attention 适用于适用于摇头机。
 */
IMPIVSInterface *Dynamic_TrackerInterfaceInit(void* param);

/**
 * @fn void Dynamic_TrackerInterfaceExit(IMPIVSInterface *inf)
 *
 * 动态运动追踪检测接口去初始化结构体
 *
 * @inf[in] 动态运动追踪检测接口结构体指针句柄
 *
 * @attention 适用于适用于摇头机。
 */
void Dynamic_TrackerInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *Cloud_TrackerInterfaceInit(void* param)
 *
 * 人形追踪检测接口初始化结构体
 *
 * @param[in] 运动追踪检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回运动追踪检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *Cloud_TrackerInterfaceInit(void* param);

/**
 * @fn void Cloud_TrackerInterfaceExit(IMPIVSInterface *inf)
 *
 * 人形追踪检测接口去初始化结构体
 *
 * @inf[in] 人形追踪检测接口结构体指针句柄
 *
 * @attention 无。
 */
void Cloud_TrackerInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn int Cloud_TrackerEncodeInterfaceInit(IMPIVSInterface *inf)
 *
 * 人形追踪检测目标数据编码接口初始化结构体
 *
 * @inf[in] 人形追踪检测接口结构体指针句柄
 *
 * @retval 非0 失败
 * @retval 0 成功
 *
 * @attention 无。
 */
int Cloud_TrackerEncodeInterfaceInit(IMPIVSInterface *inf);

/**
 * @fn int Cloud_TrackerEncodeInterfaceProcess(IMPIVSInterface *inf, void* param, void* output)
 *
 * 人形追踪检测目标数据编码接口处理目标数据
 *
 * @inf[in] 人形追踪检测接口结构体指针句柄
 * @param[in] 人形追踪检测目标数据编码初始化参数
 * @output[out] 人形追踪检测目标数据编码结果
 *
 * @retval 非0 失败
 * @retval 0 成功
 *
 * @attention 无。
 */
int Cloud_TrackerEncodeInterfaceProcess(IMPIVSInterface *inf, void* param, void* output);

/**
 * @fn void Cloud_TrackerInterfaceExit(IMPIVSInterface *inf)
 *
 * 人形追踪检测目标数据编码接口释放中间资源
 *
 * @inf[in] 人形追踪检测接口结构体指针句柄
 *
 * @retval 非0 失败
 * @retval 0 成功
 *
 * @attention 无。
 */
int Cloud_TrackerEncodeInterfaceRelease(IMPIVSInterface *inf);

/**
 * @fn int Cloud_TrackerEncodeInterfaceExit(IMPIVSInterface *inf)
 *
 * 人形追踪检测目标数据编码接口去初始化结构体
 *
 * @inf[in] 人形追踪检测接口结构体指针句柄
 *
 * @retval 非0 失败
 * @retval 0 成功
 *
 * @attention 无。
 */
int Cloud_TrackerEncodeInterfaceExit(IMPIVSInterface *inf);

IMPIVSInterface *SkipFaceInterfaceInit(void *param);

/**
 * @fn void SkipFaceInterfaceExit(IMPIVSInterface *inf)
 *
 *
 *
 * @inf[in]
 *
 * @attention
 */
void SkipFaceInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *PlateRecInterfaceInit(void *param)
 *
 * 车牌识别接口初始化结构体
 *
 * @inf[in] 车牌识别接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回车牌识别接口结构体句柄
 *
 * @attention 无
 */

IMPIVSInterface *PlateRecInterfaceInit(void *param);

/**
 * @fn void PlateRecInterfaceExit(IMPIVSInterface *inf)
 *
 * 车牌识别去初始化结构体句柄
 *
 * @inf[in] 车牌识别接口结构体句柄
 *
 * @attention 无
 */
void PlateRecInterfaceExit(IMPIVSInterface *inf);
/**
 * @fn IMPIVSInterface *PlateRecInterfaceInit(void *param)
 *
 *             
 *
 * @inf[in]            
 *
 * @retval NULL   
 *
 * @retval  NULL                    
 *
 * @attention  
 */

IMPIVSInterface *SkipFigureInterfaceInit(void *param);

/**
 * @fn void SkipFigureInterfaceExit(IMPIVSInterface *inf)
 *
 *
 *
 * @inf[in]
 *
 * @attention
 */
void SkipFigureInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *Fall_DetectInterfaceInit
 *
 * 跌倒检测接口初始化结构体
 *
 * @param[in] 跌倒检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回跌倒检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *Fall_DetectInterfaceInit(void* param);

/**
 * @fn Fall_DetectInterfaceExit(IMPIVSInterface *inf)
 *
 * 跌倒检测接口去初始化结构体
 *
 * @inf[in] 跌倒检测接口结构体指针句柄
 *
 * @attention 无。
 */
void Fall_DetectInterfaceExit(IMPIVSInterface *inf);
/**
 * @fn IMPIVSInterface *Image_MatchInterfaceInit
 *
 * 相似度检测接口初始化结构体
 *
 * @param[in] 相似度检测初始化参数
 *
 * @retval NULL 失败
 * @retval 非NULL 初始化成功，返回相似度检测接口结构体指针句柄
 *
 * @attention 无。
 */
IMPIVSInterface *Image_MatchInterfaceInit(void* param);

/**
 * @fn Image_MatchInterfaceExit(IMPIVSInterface *inf)
 *
 * 相似度检测接口去初始化结构体
 *
 * @inf[in] 相似度检测接口结构体指针句柄
 *
 * @attention 无。
 */
void Image_MatchInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *FaceDetInterfaceInit(void *param)
 *
 * DL人脸检测接口初始化结构体
 *
 * @inf[in] DL人脸检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL人脸检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *FaceDetInterfaceInit(void *param);

/**
 * @fn void FaceDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL人脸检测去初始化结构体句柄
 *
 * @inf[in] DL人脸检测接口结构体句柄
 *
 * @attention 无
 */
void FaceDetInterfaceExit(IMPIVSInterface *inf);



IMPIVSInterface *BarCodeInterfaceInit(void* param);
void BarCodeInterfaceExit(IMPIVSInterface *barcodeInterface);


/**
 * @fn IMPIVSInterface *VehicleDetInterfaceInit(void *param)
 *
 * DL车辆检测接口初始化结构体
 *
 * @inf[in] DL车辆检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL车辆检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *VehicleDetInterfaceInit(void *param);

/**
 * @fn void VehicleDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL车辆检测去初始化结构体句柄
 *
 * @inf[in] DL车辆检测接口结构体句柄
 *
 * @attention 无
 */
void VehicleDetInterfaceExit(IMPIVSInterface *inf);
/**
 * @fn IMPIVSInterface *SitPosDetInterfaceInit(void *param)
 *
 * DL坐姿检测接口初始化结构体
 *
 * @inf[in] DL坐姿检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *SitPosDetInterfaceInit(void *param);
/**
 * @fn void SitPosDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL坐姿检测去初始化结构体句柄
 *
 * @inf[in] DL坐姿检测接口结构体句柄
 *
 * @attention 无
 */
void SitPosDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *CryDetInterfaceInit(void *param)
 *
 * DL哭声检测接口初始化结构体
 *
 * @inf[in] DL哭声检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *CryDetInterfaceInit(void *param);
/**
 * @fn void CryDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL哭声检测去初始化结构体句柄
 *
 * @inf[in] DL哭声检测接口结构体句柄
 *
 * @attention 无
 */
void CryDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *PetDetInterfaceInit(void *param)
 *
 * DL宠物检测接口初始化结构体
 *
 * @inf[in] DL宠物检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL宠物检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *PetDetInterfaceInit(void *param);

/**
 * @fn void PetDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL宠物检测去初始化结构体句柄
 *
 * @inf[in] DL宠物检测接口结构体句柄
 *
 * @attention 无
 */
void PetDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *PetbodyDetInterfaceInit(void *param)
 *
 * DL宠物body检测接口初始化结构体
 *
 * @inf[in] DL宠物body检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL宠物body检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *PetbodyDetInterfaceInit(void *param);

/**
 * @fn void PetbodyDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL宠物body检测去初始化结构体句柄
 *
 * @inf[in] DL宠物body检测接口结构体句柄
 *
 * @attention 无
 */
void PetbodyDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *PetfaceDetInterfaceInit(void *param)
 *
 * DL宠物face检测接口初始化结构体
 *
 * @inf[in] DL宠物face检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL宠物face检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *PetfaceDetInterfaceInit(void *param);

/**
 * @fn void PetfaceDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL宠物face检测去初始化结构体句柄
 *
 * @inf[in] DL宠物face检测接口结构体句柄
 *
 * @attention 无
 */
void PetfaceDetInterfaceExit(IMPIVSInterface *inf);

/**
 * @fn IMPIVSInterface *DogfaceDetInterfaceInit(void *param)
 *
 * DL dog face检测接口初始化结构体
 *
 * @inf[in] DL dog face检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL dog face检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *DogfaceDetInterfaceInit(void *param);

/**
 * @fn void DogfaceDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL dog face检测去初始化结构体句柄
 *
 * @inf[in] DL dog face检测接口结构体句柄
 *
 * @attention 无
 */
void DogfaceDetInterfaceExit(IMPIVSInterface *inf);


/**
 * @fn IMPIVSInterface *CatfaceDetInterfaceInit(void *param)
 *
 * DL cat face检测接口初始化结构体
 *
 * @inf[in] DL cat face检测接口初始化参数
 *
 * @retval NULL 失败
 *
 * @retval 非NULL 初始化成功，返回DL cat face检测接口结构体句柄
 *
 * @attention 无
 */
IMPIVSInterface *CatfaceDetInterfaceInit(void *param);

/**
 * @fn void CatfaceDetInterfaceExit(IMPIVSInterface *inf)
 *
 * DL cat face检测去初始化结构体句柄
 *
 * @inf[in] DL cat face检测接口结构体句柄
 *
 * @attention 无
 */
void CatfaceDetInterfaceExit(IMPIVSInterface *inf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INTERFACE_H__ */
