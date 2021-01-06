#ifndef __PLAY_H
#define __PLAY_H

#include "include/common_env.h"
#include "include/common_func.h"
#include "ppcs_protocal.h"
#include "mp4v2/mp4v2.h"



typedef enum
{
	PLAY_STOP,
	PLAY_START,
	PLAY_PAUSE,
	PLAY_CHANGE,
	PLAY_AUDIO_OVER,
	PLAY_VIDEO_OVER,
	PLAY_OVER
}YSX_PLAY;

typedef struct playback_info
{
    int SID;// >=0 为使用中     -1为未使用
    char pb_file[128];//录像回放文件的绝对路径 当前播放文件
    struct timeval pb_tv;//当前录像文件的 起始时间
	
    int file_size;
	pthread_mutex_t pb_mutex;//录像回放资源锁
	//录像回放模式 0 时间段播放 recode_play_bytime_t   1 单个文件播放 play_single_file_t
	int pb_mode;
	play_single_file_t  psf;
	recode_play_bytime_t rpb;

	YSX_PLAY  pb_status;

	//mp4v2
	MP4FileHandle oMp4File;
	int VTrackId;
	int ATrackId;
	int VTotalNum;
	int ATotalNum;
	int video_index;
	int audio_index;
	int samplerate;//音频采样率

	int audio_framenum; //音频帧序
	int video_framenum; //视频帧序
	int wait_keyframe;
	int totol_byte;//所有h264+aac长度
	
	int token;//随机数
	char _loop;// 播放线程控制 1 running 0 stop
}playback_info_t;

playback_info_t *get_pb_info(int iGNo);


#endif
