//-----------------------------------------------------
// 360Cam Ӳ�������
// ��Ƶ�ɼ� Video Input
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------

#ifndef _QCAM_VIDEO_CTRL_H
#define _QCAM_VIDEO_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif


// H264��Ƶ�ɼ��ص�
// tv ����Ϊ�ɼ�ʱ��ʱ����gettimeofday(&tv, NULL);  ������ʵ
// data ΪNALU���ݣ��� 00 00 00 01��ʼ�ģ�
// keyframe Ϊ�ؼ�֡��־������ǹؼ�֡��data��Ҫͬʱ����NALU SPS��PPS 
typedef void (*QCam_Video_Input_cb)(const struct timeval *tv, const void *data, const int len, const int keyframe);

// ��Ƶ���ֱ���
typedef enum
{
	QCAM_VIDEO_RES_INVALID = 0,
	QCAM_VIDEO_RES_720P,		// �ֱ��� 1280x720
	QCAM_VIDEO_RES_480P,		// �ֱ��� 720x480
	QCAM_VIDEO_RES_360P,		// �ֱ��� 640x360
	QCAM_VIDEO_RES_1080P,       // �ֱ��� 1920x1080 
	QCAM_VIDEO_RES_1296P,  		// �ֱ��� 2304x1296
	QCAM_VIDEO_RES_300W,		// 2048 * 1520
	QCAM_VIDEO_RES_400W,		// 2592 * 1520
	QCAM_VIDEO_RES_500W		// 2592 * 1944
}QCAM_VIDEO_RESOLUTION;

typedef enum
{
	QCAM_VIDEO_NIGHT = 0,
	QCAM_VIDEO_DAY,
}QCAM_VIDEO_HASLIGHT;

typedef enum
{
	QCAM_IR_MODE_UNSUPPORT   = -1, // Ӳ����֧�ֺ���
	QCAM_IR_MODE_AUTO        = 0,  // �Զ����⡣��SDK���Զ��л�
	QCAM_IR_MODE_ON          = 1,  // ǿ�ƿ����⣬SDK���Զ��л�
	QCAM_IR_MODE_OFF         = 2,  // ǿ�ƹغ��⣬SDK���Զ��л�
	QCAM_IR_MODE_AUTO_COLOR  = 3,  // �Զ�ȫ��ҹ�ӣ���SDK���Զ��л�+ҵ��㿪���ص�
	QCAM_IR_MODE_SMART_COLOR = 4   // ����ȫ��ҹ�ӣ�ҹ���⵽����仯�л���ȫ�ʻ���
}QCAM_IR_MODE;

typedef enum
{
	QCAM_VIDEO_H264 = 0,  // H264
	QCAM_VIDEO_H265 = 1   // H265
}QCAM_VIDEO_ENCODE;

// ��Ƶ�ɼ�-Ƶ������
// ��Ϊ��ͬ�豸Ӳ����һ��, ����IQ��QCAM���ڲ�����
// �ⲿ�����ֻ���û�������
typedef struct QCamVideoInputChannel_t
{
	int channelId;				// channel ID�����ڿ��ƽӿڣ����޸�bitrate
	QCAM_VIDEO_RESOLUTION  res;	// resolution
	int fps;					// fps
	int bitrate;				// h264 bitrate (kbps)
	int gop;					// h264 I֡������룩�����sdk���Զ���֡�߼���Ҫ��֤I֡������䡣
	short vbr;					// VBR=1, CBR=0
	short encoding;				// QCAM_VIDEO_H264/QCAM_VIDEO_H265
	QCam_Video_Input_cb cb;		// callback
}QCamVideoInputChannel;


// ʹ�����̣�Open -> Add���Ƶ������->Start(). �������� Close();
int QCamVideoInput_Init();
int QCamVideoInput_AddChannel(QCamVideoInputChannel ch);
int QCamVideoInput_Start();
int QCamVideoInput_Uninit();


// ����������
// isVBR: 0 -- CBR, 1 -- VBR��AVBR
int QCamVideoInput_SetBitrate(int channel, int bitrate, int isVBR);

// �����ֱ���  resolution: �ο�QCAM_VIDEO_RESOLUTION
int QCamVideoInput_SetResolution (int channel, int resolution);

// ����֡��gop : �ο�QCamVideoInputChannel �е�gop
int QCamVideoInput_SetFps(int channel, int fps, int gop);
//������Ƶ�������ͣ�codecȡֵ��ΧQCAM_VIDEO_ENCODE��
int QCamVideoInput_SetCodec(int channel, int codec);

// ͼ���ã�ȫ����Ч
int QCamVideoInput_SetInversion(int enable);

// ǿ���´γ�I֡
int QCamVideoInput_SetIFrame(int channel);


// OSD
typedef struct QCamVideoInputOSD_t
{
	int pic_enable;			// ��/�ر� ͼƬˮӡ OSD
	char pic_path[128];		// ͼƬ·��
	int pic_x;				// ͼƬ����Ļ����ʾλ�ã����Ͻ����꣩
	int pic_y;

	int time_enable;		// �򿪡��ر� ʱ��OSD
	int time_x;				// ʱ������Ļ����ʾλ�ã����Ͻ����꣩
	int time_y;
}QCamVideoInputOSD;


int QCamVideoInput_SetOSD(int channel, QCamVideoInputOSD *pOsdInfo);

// ץͼ
// bufLen �������Ϊbuf��󳤶�(APIӦ��鳤���Ƿ��㹻�������Ļ�����ʧ��)��
// bufLen ͬʱҲ�����������Ϊʵ�ʳ���
// ��ͼ��񣺷ֱ���720p Ҫ����ˮӡ
// ���ʣ�720pͼƬ���100KB
// ����QCAM_FAIL/QCAM_OK;
int QCamVideoInput_CatchJpeg(char *buf, int *bufLen);

// ץYUV����ʽΪ YUV420 Planar
// buf: Y������bufǰ�棬��СΪ(W*H)�ֽڣ�UV������Y���ݺ��棬��СΪ(W*H/2)�ֽ�
// size: �����Դ���w*h������ȡY��Ҳ���Դ���w*h*1.5����ȡȫ����
// ����QCAM_FAIL/QCAM_OK;
int QCamVideoInput_CatchYUV(int w, int h, char *buf, int bufLen);

// ��⻷���Ƿ��й���
// return 1=�й� 0=û�й� -1=���ʧ��
int QCamVideoInput_HasLight();


// ���������ؽӿ�
// SDK���������������Ӳ�������������������ʽ�������ж���Ҫ��ҹ��ģʽ�����а���ģʽʱ�ص��ϲ�ӿ�
// �ϲ�ӿڸ��������Ʒ���󣬾���ִ�к��ֲ������к���ģʽ���򿪰׹�ƣ��������������򲻲�����

// ����/��ҹ�л��ص�
// has_light Ϊ QCAM_VIDEO_NIGHT ��ʾ��ǰ��⵽�������������˺�ҹ
// has_light Ϊ QCAM_VIDEO_DAY ��ʾ��ǰ��⵽�������������˰���
typedef void (*QCam_Light_Detect_cb)(int has_light);

// ����ȫ��Ψһ�Ĺ����������ص�
// ����QCAM_FAIL/QCAM_OK
int QCamSetLightDetectCallback(QCam_Light_Detect_cb cb);

// ���������
// ir Ϊ 0���رպ���ƣ�ir-cut�л�����״̬
// ir Ϊ 1���򿪺���ƣ�ir-cut�е�����ģʽ
// ����QCAM_FAIL/QCAM_OK
int QCamSetIRCut(int ir);

// ��ʱ����������㷨������Ҫ���Բ�ʵ�֣�
// ������̨����������ģʽ����ʱ������㷨֪ͨ�ϲ�Ӧ����ҹ����
// �����ϲ��߼���ǿ�ƴ��ڰ���ģʽ������򿪺�������ӹ��ʺͷ��ȣ�
// �˳�����ģʽʱ���ָ�����ҹ���л��߼�
// suspend Ϊ 1����������״̬
// suspend Ϊ 0���˳�����״̬
// ����QCAM_FAIL/QCAM_OK
int QCamSuspendLightDetect(int suspend);

// ������������׹�ģʽ��ר�ýӿ�,��ǰ����׹⣨��ɫ���������ģʽ����ɫ�ڰף��������ͺ�֧�֡�
// modeȡֵ:  
// 0: Ӳ����֧�ֺ���
// 1: �����ģʽ
// 2: ��׹�ģʽ
// 3: ǿ�ƿ���ҹ��ģʽ
// 4: ǿ�ƹر�ҹ��ģʽ(��ǿ�Ʋ�ɫģʽ)
// ����QCAM_FAIL/QCAM_OK
int QCamSetDNCMode(int mode);

// �����׹��
// mode Ϊ 0���׹�ƹر�
// mode Ϊ 1���׹�Ƴ���
// mode Ϊ 2���׹��0.2����һ��
// mode Ϊ 3���׹��0.5����һ��
// mode Ϊ 4���׹��1����һ��
// ����QCAM_FAIL/QCAM_OK
int QCamSetWhiteLight(int mode);

// ���ð׹������
// level��ʾ���ȵ�λ��1~5������λԽ�ߵ�Խ��
// ����QCAM_FAIL/QCAM_OK
int QCamSetWhiteLightBrightness(int level);

int QCamGetSensorType(char * sensor);

//���ú��⿪�أ������ͺ�֧�֡�
//nOn �� 0 -- > �������ȫ��
//       1 -- > �������ȫ��
//       2 -- > ����һ����⿪������һ������
//       3 -- > ����һ�����أ�����һ����⿪
int QCamSetInfrared(int nOn);

// ��̬����
// wdr Ϊ 0����̬�ر�
// wdr Ϊ 1����̬����
int QCamSetWdr(int wdr);

void QCamSetIRMode(QCAM_IR_MODE mode);

// ���ص�ǰ�������ģʽ��
QCAM_IR_MODE QCamGetIRMode();

typedef enum
{
	NIGHT_MODE,
    DAY_MODE,
	UNKNOW_MODE,
} QCAM_IR_STATUS;

QCAM_IR_STATUS QCamGetIRStatus();


#ifdef __cplusplus
}
#endif

#endif
