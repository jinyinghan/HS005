/*************************************************************************
	> File Name: media_buf.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Fri 26 Oct 2018 07:50:49 PM CST
 ************************************************************************/

#ifndef _MEDIA_BUF_H_
#define _MEDIA_BUF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_NODE_NUM 480

/*
example:

#define VIDEO_PRE_BUFFER_SIZE       4500000            //for 1Mbps bitrate 30 second use
#define MAX_VIDEO_PRE_BUF_NUM    (15 * MOTION_TIME_INTERVAL)    //15fps x pre-record seconds
#define USED_VIDEO_PRE_BUF_NUM   (15 * MOTION_TIME_INTERVAL)    //15fps x pre-record seconds
video_pre_buffer = init_av_buffer(VIDEO_PRE_BUFFER_SIZE, 
            USED_VIDEO_PRE_BUF_NUM, MAX_VIDEO_PRE_BUF_NUM);

*/

typedef struct av_node{
    char *data;
    int size;
    int i_frame_flag;
    int read_mark;
    //	int index;
    //	struct av_node *next;
} av_node;

typedef struct av_buffer {
    char * buffer_start;
    int size;
volatile int buffer_ptr;
volatile int write_pos;//0~480
volatile int read_pos;
    int need_buf_num;//450s
    int max_node_num;//480
    int polling_once; //是否被放入过节点
    struct av_node node[MAX_NODE_NUM];	
} av_buffer;



av_buffer *init_av_buffer(int buf_size,int need_num, int max_node_num);
void deinit_av_buffer(av_buffer * buf);
int write_buffer_data(av_buffer *buffer, char *data, int size);
int read_buffer_data(av_buffer *buffer, char **data, int *size);
void read_write_pos_sync(av_buffer *buffer);

#ifdef __cplusplus
}
#endif
#endif
