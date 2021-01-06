//-----------------------------------------------------
// 360Cam Ӳ�������
// ��Ƶ���� Audio Output
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------

#ifndef _QCAM_AUDIO_OUTPUT_H_
#define _QCAM_AUDIO_OUTPUT_H_

#define SPEAKER_VOLUME_RANGE      		100

typedef struct QCamAudioOutputAttribute_t
{
	int sampleRate;		// sample Hz
	int sampleBit;		// 16 or 8 bit
	int volume;			// ���� [0~100]�� -1 ��ʾʹ��ϵͳĬ�ϣ��������޸�
}QCamAudioOutputAttribute;

// ���豸
int QCamAudioOutputOpen(QCamAudioOutputAttribute *pAttr);

typedef struct QCamAudioOutputBufferStatus_t
{
	int total;	// buffer���洢ʱ�� (ms)
	int busy;	// buffer���ڲ���ʱ�� (ms)
}QCamAudioOutputBufferStatus;
int QCamAudioOutputQueryBuffer(QCamAudioOutputBufferStatus *pStat);
// �ر��豸
int QCamAudioOutputClose();

// ����PCM��Ƶ������ʽ���ţ�
// ���Ÿ�ʽΪԭʼPCM����������
// ��Ҫ֧�������С��PCM���ݲ��š��Զ��и���档
// ��������һ֡ʱ��ʼ���ţ�����һ֡ʱ��Ҫ���棬���´ε���ʱƴ�����ݣ��ܹ�һ֡�ٿ�ʼ��
int QCamAudioOutputPlay_ysx(char *pcm_data, int len);

// ���� [0~100]
int QCamAudioOutputSetVolume(int vol);


#endif
