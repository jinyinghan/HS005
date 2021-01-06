/*************************************************************************
	> File Name: test_list.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Tue 23 Oct 2018 11:21:37 AM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <pthread.h>


#include "ysx_video.h"
#include "list.h"



typedef struct {
    int index;
    int started;
    pthread_t thread_id;
    char thread_name[32];
    SINGLY_LINKED_LIST_INFO_ST buf;
} media_video_stream;

media_video_stream g_video_stream[MAX_FS] = {0};



int set_thread_name(pthread_t thread, const char * name)
{
    if (thread < 0 || NULL == name) {
        fprintf(stderr, "argument error!\n");
        return -1;
    }
    prctl(PR_SET_NAME, name);
    return 0;
}


void stream_main_cb(const struct timeval *tv, const void *data, const int len, const int keyframe)
{
    queue_push2(&g_video_stream[0].buf, data, len);
}

void stream_second_cb(const struct timeval *tv, const void *data, const int len, const int keyframe)
{
    queue_push2(&g_video_stream[1].buf, data, len);
}

void stream_third_cb(const struct timeval *tv, const void *data, const int len, const int keyframe)
{
    queue_push2(&g_video_stream[2].buf, data, len);
}


void * write_stream_proc(void * arg)
{
    int ret = 0;
    int channel = 0;
    int data_len = 0;
    unsigned int frame_index = 0;
    int item_count = 0;
    char data_buf[512 * 1024] = {0};
    char stream_name[128] = {0};
    int isfirst = 1;
    FILE * fp = NULL;
    media_video_stream * video = (media_video_stream *)arg;
    if (strlen(video -> thread_name) > 0) {
        set_thread_name(video -> thread_id, video -> thread_name);      
    }
    

    printf("stream index: %d\n", video -> index);
    
    channel = video -> index;
    snprintf(stream_name, sizeof(stream_name) -1, "stream-%d.h265", video -> index);
  
    if (isfirst) {
        fp = fopen(stream_name, "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfirst = 0;
    }

	for (;;) {
		item_count = sll_list_item_num(&video -> buf);
		if (item_count <= 0) {
			usleep(50 * 1000);
			continue;
		}

//      fprintf(stderr, "chaennel: %d, item num: %d\n", channel, item_count);
      
		memset(data_buf, '\0', sizeof(data_buf));
		data_len = sll_get_list_item(&video -> buf, data_buf, 512 * 1024, frame_index);
		if (data_len > 0) {
            fwrite(data_buf, sizeof(char), data_len, fp);
            fflush(fp);  
            frame_index ++;
            if (channel == 0) {
 //               printf("item num: %d, frame index: %d, len: %d\n", item_count, frame_index, data_len);
            }
        }
		else {          
			if (frame_index < video -> buf.tail_id) {
				frame_index = video -> buf.head_id;
			}
			else if (frame_index >= video -> buf.tail_id) {
                usleep(50 * 1000);
            }
        }
   }

}


void video_motion_cb(int value, int irmode)
{
    printf("motion type: %d, ircut mode: %d\n", value, irmode);
}

//#define TEST_QRSCAN_VIDEO_SOURCE
int main(int argc, char ** argv)
{
	int ret = 0;
    int index = 0;
    unsigned int frame_count = 100;


#ifdef TEST_QRSCAN_VIDEO_SOURCE
    QCamVideoInput_Init();
    sleep(3);
    QCamVideoInput_Uninit();
    return 0;
#endif 

    for (index = 0; index < MAX_FS; index ++) {
        memset(&g_video_stream[index], '\0', sizeof(media_video_stream));
        g_video_stream[index].index = index;
        g_video_stream[index].started = 1;
        snprintf(g_video_stream[index].thread_name, sizeof(g_video_stream[index].thread_name) - 1, "stream-%02d", index);
        sll_init_list(&g_video_stream[index].buf, 60, 2 * 1024 * 1024);
    }


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


    for (index = 0; index < MAX_FS; index ++) {
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


    for (index = 0; index < MAX_FS; index ++) {
       ret = pthread_create(&g_video_stream[index].thread_id, NULL, write_stream_proc, (void *)&g_video_stream[index]);
       if (ret < 0) {
           printf("create thread failed!\n");
           return -1;
       }
       pthread_detach(g_video_stream[index].thread_id);
    }



    
//#define TEST_IRCUT_API   
#ifdef TEST_IRCUT_API
    QCAM_IR_MODE irmode;
    irmode = QCamGetIRMode();
    printf("##### irmode = %d\n", irmode);
    sleep(10);

    QCamSetIRMode(QCAM_IR_MODE_AUTO);
    irmode = QCamGetIRMode();
    printf("##### set auto, get irmode = %d\n", irmode);
    sleep(10);

    QCamSetIRMode(QCAM_IR_MODE_ON);
    irmode = QCamGetIRMode();
    printf("##### set on, get irmode = %d\n", irmode);
    sleep(10);

    QCamSetIRMode(QCAM_IR_MODE_OFF);
    irmode = QCamGetIRMode();
    printf("##### set off, get irmode = %d\n", irmode); 
    sleep(10);
#endif 

#define TEST_OSD_API
#ifdef TEST_OSD_API
    EMGCY_LOG("@@@@@@@@@ test osd...\n");
	QCamVideoInputOSD osd_attr;
	memset(&osd_attr, 0, sizeof(QCamVideoInputOSD));
	osd_attr.pic_enable = 0;
	osd_attr.time_enable = 1;
    osd_attr.time_x = 2136;
	osd_attr.time_y = 0;
	
	ret = QCamVideoInput_SetOSD(0, &osd_attr);
	if (ret < 0) {
		printf("QCamVideoInput_SetOSD ERROR !\n");
		return -1;
	}

	memset(&osd_attr, 0, sizeof(QCamVideoInputOSD));
	osd_attr.pic_enable = 0;
	osd_attr.time_enable = 1;
	osd_attr.time_x = 1128; //640-19*8 = 488    
	osd_attr.time_y = 0;

	ret = QCamVideoInput_SetOSD(1, &osd_attr);
	if (ret < 0) {
		printf("QCamVideoInput_SetOSD ERROR !\n");
		return -1;
	}
    
	memset(&osd_attr, 0, sizeof(QCamVideoInputOSD));
	osd_attr.pic_enable = 0;
	osd_attr.time_enable = 1;
	osd_attr.time_x = 488; //640-19*8 = 488
	osd_attr.time_y = 0;

	ret = QCamVideoInput_SetOSD(2, &osd_attr);
	if (ret < 0) {
		printf("QCamVideoInput_SetOSD ERROR !\n");
		return -1;
	}
#endif 


#ifdef MD_API_TEST
    QCamSetMotionSense(4);
    QCamInitMotionDetect2(video_motion_cb);
    QCamStopFullScreenDetect();
    sleep(10);
    QCamUninitMotionDetect();
    
    sleep(3);
    QCamSetMotionSense(2);
    QCamInitMotionDetect2(video_motion_cb);
    QCamStopFullScreenDetect();
    sleep(10);
    QCamStopFullScreenDetect();
    sleep(3);
#endif 
    

    sleep(5);

#if 0
    while(1) 
    {    
        sleep(1);
        system("free >> /tmp/mmcblk0p1/free.txt");
    }
#endif 
    
exit:
    QCamVideoInput_Uninit();
    for (index = 0; index < MAX_FS; index ++) {
        g_video_stream[index].started = 0;
        pthread_join(g_video_stream[index].thread_id, NULL);
        sll_destroy_list(&g_video_stream[index].buf);
        EMGCY_LOG("free channel %d buf!\n", index);
    }
    
	return ret;
}
