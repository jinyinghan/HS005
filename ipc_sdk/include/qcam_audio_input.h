//-----------------------------------------------------
// 360Cam Ӳ�������
// ��Ƶ�ɼ� Audio Input
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------


#ifndef _QCAM_AUDIO_INPUT_H_
#define _QCAM_AUDIO_INPUT_H_

#define MIC_VOLUME_RANGE		   		100	

// �ɼ��ص�����
// tv ����Ϊ�ɼ�ʱ��ʱ����gettimeofday(&tv, NULL); ������ʵ
typedef void (*QCamAudioInputCallback)(const struct timeval *tv, const void *pcm_buf, const int pcm_len, const void *spk_buf);

typedef struct QCamAudioInputAttribute_t
{
	int sampleRate;		// sample Hz������֧��48Khz��������ѡ֧��8k��16Khz��
	int sampleBit;		// 16 or 8 (bits)
	int volume;			// ���� [0~100]�� -1 ��ʾʹ��ϵͳĬ�ϣ��������޸�
	QCamAudioInputCallback cb;	// callback
}QCamAudioInputAttr;

// ���豸
int QCamAudioInputOpen_ysx(QCamAudioInputAttr *pAttr);

// �ر��豸
int QCamAudioInputClose_ysx();

// start/stop ���ڿ����Ƿ�ɼ�callback����
int QCamAudioInputStart();
int QCamAudioInputStop();

// ���� [0~100]
int QCamAudioInputSetVolume(int vol);

// �������ر�AEC
int QCamAudioAecEnable(int enable);
int QCamAudioDenoiseEnable_ysx(int enable);

//add by xc for t10
void QCamAudioInputSetGain(int gain);

#endif
