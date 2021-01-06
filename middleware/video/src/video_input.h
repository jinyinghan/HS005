/*************************************************************************
	> File Name: video_input.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月20日 星期四 14时27分08秒
 ************************************************************************/

#ifndef _VIDEO_INPUT_H_
#define _VIDEO_INPUT_H_

typedef void (* video_input_cb)(int channel, const struct timeval * tv, const void * data, const int len, const int keyframe);


typedef struct {
    int channel;
    int fps;
    int resolution;
    int bitrate;
    int started;
    int destroyed;
    int cbr;
    video_input_cb cb;
    pthread_t thread_id;
    char thread_name[64];
} video_input_context;



int video_input_init(void);
int video_input_add_channel(video_input_context * vc);
int video_input_start(video_input_context * vc);

#ifdef USE_IVS_FREE
void * figure_thread(void *arg);
//int QCamSmartCreate(QCamSmartInParam *pInParam);
int QCamSmartDestory();
#endif

#endif 
