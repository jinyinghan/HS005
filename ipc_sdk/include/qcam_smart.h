#ifndef _QCAM_SMART_H_
#define _QCAM_SMART_H_

//================����&&���������� start======================//
#define SMART_MAX_RECT_NUM		(10)
#define SMART_MAX_FD_JPEG_NUM	(SMART_MAX_RECT_NUM)

typedef struct QCamSmartInParam_t
{
	int smart_enable;		 /* ���Ρ�����ʹ�ܿ���, 0: �ر�; 1: ����*/
	unsigned int width;		 /* ͼ��Ŀ��̶�640 */
	unsigned int height;	 /* ͼ��ĸߣ��̶�360 */
	int smart_pq_enable;	 /* ����PQʹ�ܿ���,���ø߼�ISPЧ���Ŀ���״̬���߼�ISPЧ����Ҫ�Ż�����Ƿ�غ͹��ص�Ч��,0: �ر�; 1: ���� */
	int low_bitrate_enable;  /* ����PQ������ʹ�ܿ���,0: �ر�; 1: ���� */
	int iso_adaptive_enable; /* ����ISO��������������Ӧ���ڿ���,0: �ر�; 1: ����*/
	int anti_flicker_enable; /* ͼ����ʹ�ܿ���,0: �ر�; 1: ���� */
	float pd_threshold;		 /* ��ֵ����Χ[0.0,1.0)��ֵԽ��SMD �澯�����μ��Խ������ */
	float fd_threshold;		 /* ��ֵ����Χ[0.0,1.0)��ֵԽ���������Խ������ */
} QCamSmartInParam;

typedef struct QCamSmartRect_t
{
	// ���궼���������������ı�����ȡֵ��Χ0~0.999999
	double x1; /* �������Ͻ�x���� */
	double y1; /* �������Ͻ�y���� */
	double x2; /* �������½�x���� */
	double y2; /* �������½�y���� */
} QCamSmartRect;

typedef struct QCamSmartTarget_t
{
	int pd_target_num;						   /* ��⵽������Ŀ����� */
	int fd_target_num;						   /* ��⵽������Ŀ����� */
	QCamSmartRect pd_rect[SMART_MAX_RECT_NUM]; /* ��⵽������Ŀ������ */
	QCamSmartRect fd_rect[SMART_MAX_RECT_NUM]; /* ��⵽������Ŀ������ */
	float pd_quality[SMART_MAX_RECT_NUM];      /* ����Ŀ���������֣���ʱ��Ч��Ĭ��0 */
	float fd_quality[SMART_MAX_RECT_NUM];      /* ����Ŀ���������֣���Χ0 ~ 10 */
	int reserved[8];						   /* ���� */
} QCamSmartTarget;

typedef struct QCamSmartJpegResult_t
{
	unsigned int width;		/* ץ��ͼƬʵ�ʵĿ�� */
	unsigned int height;	/* ץ��ͼƬʵ�ʵĸ߶� */
	char *buffer;			/* ץ��ͼƬ�ĵ�ַ */
	unsigned int size;		/* ����ʵ��ץ��ͼƬ�Ĵ�С */
	unsigned long long pts; /* ʱ��� */
	int reserved[8];		/* ���� */
} QCamSmartJpegResult;

typedef struct QCamSmartFdJpegInfo_t
{
	unsigned int jpeg_num;									/* ץ������Сͼ���������ΪSMART_MAX_FD_JPEG_NUM*/
	QCamSmartJpegResult pJpegResult[SMART_MAX_FD_JPEG_NUM]; /* ץ������Сͼ���  */
} QCamSmartFdJpegInfo;

/*****************************************************************************
 ������		: QCamSmartCreate
 ��������	: �������������μ�⹦�ܣ�������ʼ��
 �������   : pInParam[in]: ���������μ���㷨���ò�����ָ��
 �������   : ��
 ����ֵ     : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartCreate(QCamSmartInParam *pInParam);

/*****************************************************************************
 ������		: QCamSmartSetPDThreshold QCamSmartSetFDThreshold
 ��������	: �������μ�⡢���������ֵ�������㷨�������޸�
 �������   : th[in]: �㷨��ֵ
 �������   : ��
 ����ֵ     : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartSetPDThreshold(double th);
int QCamSmartSetFDThreshold(double th);

/*****************************************************************************
 �� �� ��  : QCamSmartGetTarget
 ��������  : ��ȡ����������Ŀ�������������Ϣ��������Ϣ�����(640 * 360 ) ��������ϵ
 �������  : ��
 �������  : pTarget[out]: ���������μ�����Ŀ����Ϣָ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartGetTarget(QCamSmartTarget *pTarget);

/*****************************************************************************
 �� �� ��  : QCamSmartGetTargetV2
 ��������  : ��ȡ����������Ŀ�������������Ϣ��������Ϣ�����(640 * 360 ) ��������ϵ���汾�������LibXmMaQue_SmartGetTarget �������������������֡�
 �������  : ��
 �������  : pTarget[out]: ���������μ�����Ŀ����Ϣָ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartGetTargetV2(QCamSmartTarget *pTarget);

/*****************************************************************************
 �� �� ��  : QCamSmartDestory
 ��������  : �������������μ�⹦��
 �������  : ��
 �������  : ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartDestory();

/*****************************************************************************
 �� �� ��  : QCamDrawRect
 ��������  : ��һ֡�ϻ�������ο򣬲����֮ǰ�Ļ��������num=0����ʾ�����һ֡�����п�
 �������  : pRect[in]: ���ο������
		    num[in]: ��ĸ���
 �������  : ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamDrawRect(QCamSmartRect *pRect, int num);

/*****************************************************************************
 �� �� ��  : QCamSmartCaptureFdJpeg
 ��������  : ץ������Сͼ����
 �������  : 
 �������  : pFdJpegInfo[out]: ץ������Сͼ���ָ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartCaptureFdJpeg(QCamSmartFdJpegInfo *pFdJpegInfo);

//����AI���Ρ������㷨
int QCamSmartSetAiAlg(int enable, QCamSmartInParam *pInParam);

//==================����&&���������� end======================//


//=====================����ʶ����� start=======================//

#define SMART_HGR_TARGET_NUM   (3)

typedef struct
{
	int   enable;		  /* ����ʶ��ʹ�ܿ���, 0: �ر�; 1: ����*/
	float threshold;      /* ��ֵ����Χ[0.0, 1.0 ), ֵԽ��Խ���������Ƽ�ֵ0.55 */
} QCamSmartHgrInParam;

typedef enum
{
	HGR_PALM_TIGHT,     /* ���ƽ��� */
	HGR_THUMB_INDEX_L,  /* ��Ĵָ��ʳָ�� L ״̬ */
	HGR_THUMB_PINKY_Y,  /* ��Ĵָ��Сָ�� Y ״̬ */
	HGR_UNKNOW          /* δ֪��������� */
} QCamHgrType_E;

typedef struct
{
	QCamHgrType_E  hgr_type; /* �������� */
	QCamSmartRect  hgr_rect; /* ����Ŀ������ */
} QCamSmartHgrInfo;

typedef struct
{
	int hgr_target_num; 							 /* ����Ŀ����� */
	QCamSmartHgrInfo hgr_info[SMART_HGR_TARGET_NUM]; /* ����Ŀ����Ϣ */
	int reserved[8];
} QCamSmartHgrTarget;

/*****************************************************************************
 ������		: QCamSmartHgrCreate
 ��������	: ��������ʶ����
 �������   : ��
 �������   : ��
 ����ֵ     : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartHgrCreate(QCamSmartHgrInParam *pInParam);

/*****************************************************************************
 ������		: QCamSmartSetHgrParam
 ��������	: �������Ƽ�����
 �������   : enable [in] ���ò�����ʹ�ܿ��ơ�
 			  th     [in] ���ò�������Χ[0.0, 1.0 ), ֵԽ��Խ��������
 �������   : ��
 ����ֵ     : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartSetHgrParam(int enable, float th);

/*****************************************************************************
 �� �� ��  : QCamSmartGetHgrTarget
 ��������  : ��ȡ����Ŀ�������������Ϣ��������Ϣ�����(640 * 360) ��������ϵ
 �������  : ��
 �������  : pTarget[out]: ���Ƽ�����Ŀ����Ϣָ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartGetHgrTarget(QCamSmartHgrTarget *pTarget);

/*****************************************************************************
 �� �� ��  : QCamSmartHgrDestory
 ��������  : ��������ʶ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : �ɹ�����0��ʧ�ܷ���-1
*****************************************************************************/
int QCamSmartHgrDestory();

//=====================����ʶ����� end=======================//
int create_smt_draw();
void destroy_smt_draw();

typedef struct 
{
   int x1;     
   int y1;   
   int x2; 
   int y2;
}Rect_t;
typedef struct 
{
        int pd_num;                         /* ���������� */
        Rect_t pd_rect[SMART_MAX_RECT_NUM]; /* ���������� */
        int score;      /* ����������������0 */
}Figure_Target;
typedef struct {
    pthread_t fig_thread_id ;
    ///IMPIVSInterface *inteface ;
    int figure_exit ;
    pthread_mutex_t smart_lock;
    Figure_Target ft;
    int ivs_enable;
}smart_init_t ;

#endif
