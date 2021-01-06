//-----------------------------------------------------
// 360Cam 硬件抽象层
// 音频播放 Audio Output
// author : zhaohuaqiang@360.cn
//-----------------------------------------------------

#ifndef _QCAM_AUDIO_OUTPUT_H_
#define _QCAM_AUDIO_OUTPUT_H_

#define SPEAKER_VOLUME_RANGE      		100

typedef struct QCamAudioOutputAttribute_t
{
	int sampleRate;		// sample Hz
	int sampleBit;		// 16 or 8 bit
	int volume;			// 音量 [0~100]， -1 表示使用系统默认，不进行修改
}QCamAudioOutputAttribute;

// 打开设备
int QCamAudioOutputOpen(QCamAudioOutputAttribute *pAttr);

typedef struct QCamAudioOutputBufferStatus_t
{
	int total;	// buffer最多存储时长 (ms)
	int busy;	// buffer正在播放时长 (ms)
}QCamAudioOutputBufferStatus;
int QCamAudioOutputQueryBuffer(QCamAudioOutputBufferStatus *pStat);
// 关闭设备
int QCamAudioOutputClose();

// 播放PCM音频（阻塞式播放）
// 播放格式为原始PCM，无需编解码
// 需要支持任意大小的PCM数据播放。自动切割并缓存。
// 即：多余一帧时开始播放，少于一帧时需要缓存，待下次调用时拼接数据，攒够一帧再开始播
int QCamAudioOutputPlay_ysx(char *pcm_data, int len);

// 音量 [0~100]
int QCamAudioOutputSetVolume(int vol);


#endif
