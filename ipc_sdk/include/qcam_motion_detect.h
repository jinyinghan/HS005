//-----------------------------------------------------
// 360Cam Ӳ�������
// �ƶ���� Motion Detect
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------


#ifndef _QCAM_MOTION_DETECT_H
#define _QCAM_MOTION_DETECT_H

// һ����360ʵ�֣���ҪӲ��֧���ƶ����

typedef enum
{
    QCAM_MD_NONE = 0, 
    QCAM_MD_FACE = 1,                   // ����
    QCAM_MD_WINDOW = 2,                 // ����
    QCAM_MD_RECORD = 4,                 // ȫ��������⣬������¼��ʼ/ֹͣ
    QCAM_MD_FULLSCREEN = 8,             // ȫ��������⣬���ڷ���˽���δ�����μ��
    // QCAM_MD_XXX = 8
} QCAM_MOTION_DETECT_TYPE;

// �ƶ����ص�����
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
