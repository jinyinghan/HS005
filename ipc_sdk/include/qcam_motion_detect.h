//-----------------------------------------------------
// 360Cam 硬件抽象层
// 移动侦测 Motion Detect
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------


#ifndef _QCAM_MOTION_DETECT_H
#define _QCAM_MOTION_DETECT_H

// 一般由360实现，需要硬件支持移动侦测

typedef enum
{
    QCAM_MD_NONE = 0, 
    QCAM_MD_FACE = 1,                   // 人脸
    QCAM_MD_WINDOW = 2,                 // 安防
    QCAM_MD_RECORD = 4,                 // 全屏动作检测，用于云录开始/停止
    QCAM_MD_FULLSCREEN = 8,             // 全屏动作检测，用于服务端进行未见人形检测
    // QCAM_MD_XXX = 8
} QCAM_MOTION_DETECT_TYPE;

// 移动侦测回调函数
typedef void (*QCam_MD_cb)(int type);
typedef void (* ysx_motion_cb)(int md_value, int ir_mode);


void QCamInitMotionDetect(QCam_MD_cb cb);
void QCamInitMotionDetect2(ysx_motion_cb cb);
void QCamUninitMotionDetect();
void QCamStartFaceDetect();
void QCamStopFaceDetect();
void QCamStartWindowAlert(double p1x, double p1y, double p2x, double p2y);
void QCamStopWindowAlert();
void QCamStartCloudRecord();
void QCamStopCloudRecord();
void QCamStartFullScreenDetect();
void QCamStopFullScreenDetect();

#endif
