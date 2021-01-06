/*************************************************************************
	> File Name: test_api.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Sat 27 Oct 2018 11:39:09 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

#include "ysx_audio.h"
#include "list.h"



typedef struct {
    int index;
    int started;
    int is_mic;
    pthread_t thread_id;
    char thread_name[32];
    SINGLY_LINKED_LIST_INFO_ST buf;
} media_audio_stream;

media_audio_stream g_mic;


int set_thread_name(pthread_t thread, const char * name)
{
    if (thread < 0 || NULL == name) {
        fprintf(stderr, "argument error!\n");
        return -1;
    }
    prctl(PR_SET_NAME, name);
    return 0;
}



void media_mic_cb(const struct timeval *tv, const void * pcm_buf, const int pcm_len, const void *spk_buf)
{
    queue_push2(&g_mic.buf, pcm_buf, pcm_len); 
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
    media_audio_stream * mic = (media_audio_stream *)arg;
    if (strlen(mic -> thread_name) > 0) {
        set_thread_name(mic -> thread_id, mic -> thread_name);      
    }
    

    printf("stream index: %d\n", mic -> index);
    
    channel = mic -> index;
    snprintf(stream_name, sizeof(stream_name) -1, "mic-%d.pcm", mic -> index);
  
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
		item_count = sll_list_item_num(&mic -> buf);
		if (item_count <= 0) {
			usleep(50 * 1000);
			continue;
		}

//      fprintf(stderr, "chaennel: %d, item num: %d\n", channel, item_count);
      
		memset(data_buf, '\0', sizeof(data_buf));
		data_len = sll_get_list_item(&mic -> buf, data_buf, 512 * 1024, frame_index);
		if (data_len > 0) {
            fwrite(data_buf, sizeof(char), data_len, fp);
            fflush(fp);  
            frame_index ++;
            if (channel == 0) {
 //               printf("item num: %d, frame index: %d, len: %d\n", item_count, frame_index, data_len);
            }
        }
		else {          
			if (frame_index < mic -> buf.tail_id) {
				frame_index = mic -> buf.head_id;
			}
			else if (frame_index >= mic -> buf.tail_id) {
                usleep(50 * 1000);
            }
        }
   }

}


int main(int argc, char ** argv)
{
    int ret = 0;
    QCamAudioInputAttr attr;


    memset(&g_mic, '\0', sizeof(media_audio_stream));
    g_mic.index = 0;
    g_mic.started = 1;
    g_mic.is_mic = 1;
    snprintf(g_mic.thread_name, sizeof(g_mic.thread_name) - 1, "stream-%02d", 0);
    sll_init_list(&g_mic.buf, 60, 2 * 1024 * 1024);


	attr.sampleBit = 16;
	attr.sampleRate = 16000;
	attr.volume = -1;
	attr.cb = media_mic_cb;
	ret = QCamAudioInputOpen(&attr);
	if (ret < 0) {
		fprintf(stderr, "QCamAudioInputOpen Failed\n");
		return -1;
	}

	ret = QCamAudioInputStart();
	if (ret < 0) {
		fprintf(stderr, "QCamAudioInputStart Failed\n");
		return -1;
	}



   ret = pthread_create(&g_mic.thread_id, NULL, write_stream_proc, (void *)&g_mic);
   if (ret < 0) {
       printf("create thread failed!\n");
       return -1;
   }
   pthread_detach(g_mic.thread_id);

   sleep(5);
  
    
   QCamAudioInputClose();

   g_mic.started = 0;
   pthread_join(g_mic.thread_id, NULL);
   sll_destroy_list(&g_mic.buf);

    

    return 0; 
}
