#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "ysx_video.h"
#include "queue_buf.h"

#define JPEG_BUF_SIZE (2592 * 1944 * 3)
#define MAX_FRMS (15*100-1)
#define JPEG_ON 


typedef struct {
    int index;
    int started;
    pthread_t thread_id;
    queue_info buf;
} media_video_stream;


media_video_stream g_video_stream[MAX_FS] = {0};


#define CACHE_FRAME_NUM         60
#define CACHE_DATA_SIZE         (1024 * 1024 * 3) 


// 打开该宏,使用缓冲区,从缓冲区读取数据保存到文件,否则直接写文件
//#define USE_QUEUE_BUF
//#define VIDEO_RAW_STREAM_DBG
static int g_start = 1;




void stream_main_cb(int channel, const struct timeval *tv, const void *data, const int len, const int keyframe)
{
//    printf("channel: %d\n", channel);
    int ret = 0;
	printf("timestamp: %d\n", tv -> tv_sec * 1000 + tv -> tv_usec);


#ifdef USE_QUEUE_BUF   
    if (data && len > 0) {
        ret = queue_push(&g_video_stream[0].buf, data, len);
        if (ret < 0) {
            printf("queue push failed!\n");
        }
    }

	sleep(5);

    static FILE * fp = NULL;
    static int isfisrt = 1;

    if (isfisrt) {
        fp = fopen("stream-0-f0.h265", "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    fwrite(data, sizeof(char), len, fp);
    fflush(fp);
#endif   
}


void stream_second_cb(int channel, const struct timeval *tv, const void *data, const int len, const int keyframe)
{
//    printf("channel: %d\n", channel);

    int ret = 0;
#ifdef USE_QUEUE_BUF   
    if (data && len > 0) {
        ret = queue_push(&g_video_stream[1].buf, data, len);
        if (ret < 0) {
            printf("queue push failed!\n");
        }
    }
    static FILE * fp = NULL;
    static int isfisrt = 1;

    if (isfisrt) {
        fp = fopen("stream-f1.h265", "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    fwrite(data, sizeof(char), len, fp);
    fflush(fp);
#endif 
}



void stream_third_cb(int channel, const struct timeval *tv, const void *data, const int len, const int keyframe)
{
//    printf("channel: %d\n", channel);

    int ret = 0;
#ifdef USE_QUEUE_BUF   
    if (data && len > 0) {
        ret = queue_push(&g_video_stream[2].buf, data, len);
        if (ret < 0) {
            printf("queue push failed!\n");
        }
    }

    static FILE * fp = NULL;
    static int isfisrt = 1;

    if (isfisrt) {
        fp = fopen("stream-2-f2.h265", "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    fwrite(data, sizeof(char), len, fp);
    fflush(fp);
#endif 
}





void write_stream_proc(void * arg)
{
    int ret = 0;
    static FILE * fp = NULL;
    static int isfisrt = 1;
    int item_data_len = 0;
    static char data_buf[300 * 1024] = {0};
    int frame_index = 0;
    int item_num  = 0; 
    int channel = -1;
    char stream_name[128] = {0};

    EMGCY_LOG("video stream: %x\n", (media_video_stream *)arg);

    media_video_stream * pstream = (media_video_stream *)arg;
    channel = pstream -> index;
    snprintf(stream_name, sizeof(stream_name) -1, "stream-%d.h265", pstream -> index);
    printf("@@@@@@@@@@@ streams data proc: %s\n", stream_name);

    if (isfisrt) {
        fp = fopen(stream_name, "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    EMGCY_LOG("@@@@ schannel: %d, buf addr: %x\n", channel, (queue_info *)(&pstream -> buf));

    for (;;) {
        item_num = queue_item_num(&pstream -> buf);
        if (item_num <= 0) {
            usleep(50 * 1000);
            continue;
        }
    
        item_data_len = queue_pop(&pstream -> buf, &data_buf, sizeof(data_buf), frame_index);
        if (item_data_len > 0) {
//            printf("pop item data size: %d, index: %d\n", item_data_len, frame_index);
            fwrite(data_buf, sizeof(char), item_data_len, fp);
            fflush(fp);       
            frame_index ++;
        }
        else {
            if (frame_index < pstream -> buf.tail_id) {
                frame_index = pstream -> buf.head_id;
            }
            else {
                usleep(50 * 1000);
            }
        }


        if (!pstream -> started) {
            if (fp) {
                fclose(fp);
            }            
            break;
        }
    }

    fprintf(stderr, "exit writing stream...\n");
}



void write_stream_proc2(void * arg)
{
    media_video_stream * stream = (media_video_stream *)arg;
    printf("stream addr: %x\n", stream);
    printf("stream index: %x\n", stream -> index);
    printf("stream buf addr: %x\n", &stream -> buf);
    printf("buf max item size: %d\n", stream -> buf.max_item_num);

}

//#define QUEUE_API_TEST
int main(int argc, char **argv)
{
	printf("...\n");
	int ret;
    int index = 0;

	EMGCY_LOG("-------------------------------------\n");

    EMGCY_LOG("g_video_stream addr: %x\n", &g_video_stream);    

    QCamVideoInputChannel ysx_chn[3];
    ysx_chn[0].bitrate = 1024 * 1.5;
    ysx_chn[0].cb = stream_main_cb;
    ysx_chn[0].channelId = 0;
    ysx_chn[0].fps = 15;
    ysx_chn[0].gop = 1;
    ysx_chn[0].res = QCAM_VIDEO_RES_500W;
    ysx_chn[0].vbr = 0;     
    ysx_chn[0].encoding = 1;

    ysx_chn[1].bitrate = 512;
    ysx_chn[1].cb = stream_second_cb;
    ysx_chn[1].channelId = 1;
    ysx_chn[1].fps = 15;
    ysx_chn[1].gop = 1;
    ysx_chn[1].res = QCAM_VIDEO_RES_720P;
    ysx_chn[1].vbr = 0;     
    ysx_chn[1].encoding = 0;
	
    ysx_chn[2].bitrate = 128;
    ysx_chn[2].cb = stream_third_cb;
    ysx_chn[2].channelId = 2;
    ysx_chn[2].fps = 15;
    ysx_chn[2].gop = 1;
    ysx_chn[2].res = QCAM_VIDEO_RES_360P;
    ysx_chn[2].vbr = 0;     
    ysx_chn[2].encoding = 0;

    ret = QCamVideoInput_Init();
    if(ret < 0){
        QLOG(FATAL,"QCamVideoInput_Init ERROR !\n");
        return -1;
    }
 
    for (index = 0; index < MAX_FS-1; index ++) {
        ret = QCamVideoInput_AddChannel(ysx_chn[index]);
        if(ret < 0){
            QLOG(FATAL,"QCamVideoInput_AddChannel[0] ERROR !\n");
            goto exit;
        }
        EMGCY_LOG("add video channel %02d over...\n", index);
    }
    
    ret = QCamVideoInput_Start();
    if(ret < 0){
        QLOG(FATAL,"QCamVideoInput_Start ERROR !\n");
        goto exit;
    }

    printf("ha ha!!!\n");
    sleep(30);

exit:  
    // 先释放掉视频相关资源,防止继续使用队列资源
    QCamVideoInput_Uninit();
    sleep(3);
    return 0;
}



