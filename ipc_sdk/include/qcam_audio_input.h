//-----------------------------------------------------
// 360Cam 硬件抽象层
// 音频采集 Audio Input
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------


#ifndef _QCAM_AUDIO_INPUT_H_
#define _QCAM_AUDIO_INPUT_H_

#define MIC_VOLUME_RANGE		   		100	

// 采集回调函数
// tv 定义为采集时的时戳，gettimeofday(&tv, NULL); 必须真实
typedef void (*QCamAudioInputCallback)(const struct timeval *tv, const void *pcm_buf, const int pcm_len, const void *spk_buf);

typedef struct QCamAudioInputAttribute_t
{
	int sampleRate;		// sample Hz（必须支持48Khz采样，可选支持8k、16Khz）
	int sampleBit;		// 16 or 8 (bits)
	int volume;			// 音量 [0~100]， -1 表示使用系统默认，不进行修改
	QCamAudioInputCallback cb;	// callback
}QCamAudioInputAttr;

// 打开设备
int QCamAudioInputOpen_ysx(QCamAudioInputAttr *pAttr);

// 关闭设备
int QCamAudioInputClose_ysx();

// start/stop 用于控制是否采集callback发生
int QCamAudioInputStart();
int QCamAudioInputStop();

// 音量 [0~100]
int QCamAudioInputSetVolume(int vol);

// 开启、关闭AEC
int QCamAudioAecEnable(int enable);
int QCamAudioDenoiseEnable_ysx(int enable);

//add by xc for t10
void QCamAudioInputSetGain(int gain);

#endif
