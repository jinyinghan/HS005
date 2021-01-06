//-----------------------------------------------------
// 360Cam Ӳ�������
// ϵͳ���ƣ��ƿأ���������¼�����Ź��ȣ�
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------

#ifndef _QCAM_SYS_H
#define _QCAM_SYS_H

// KEY STATUS
typedef enum
{
    QCAM_KEY_INVALID = -1,
	QCAM_KEY_RELEASED = 0,
	QCAM_KEY_PRESSED = 1,
}QCAM_KEY_STATUS;

//Callback for key status
typedef void (*QCam_Key_Status_cb)(const int status);

// LED �ƿ�
typedef enum
{
	LED_MODE_OFF = 0,
	LED_MODE_GREEN = 1,
	LED_MODE_BLUE = 2,
	LED_MODE_RED = 3,
	LED_MODE_FLIGHT = 4,
}QCAM_LED_MODE;

int QCamLedSet(QCAM_LED_MODE mode, int blink);

// ��ȡ����״̬ 0=δ���� 1=���� -1=���ʧ��
int QCamGetKeyStatus();

void QCamRegKeyListener(QCam_Key_Status_cb cb);

int QCamFlashBurn(const char *firmwarePath);

#endif
