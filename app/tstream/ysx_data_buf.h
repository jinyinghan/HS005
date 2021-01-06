#ifndef _DATA_BUF_H_
#define _DATA_BUF_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"log.h"
#include<stdint.h>
#define MAX_NODE_NUM 30

typedef struct av_node{
	char *data;
	int size;
	int keyframe;
	int read_mark;				// 1:已经被读取,0: 未被读取
	struct timeval tv;
	//unsigned char index;
	int index;
}av_node;

typedef struct av_buffer{
	char *buffer_start;	
	int buffer_offset;			// 当前缓存数据总大小
	int buffer_sum_size;
	int max_node_num;
	int min_node_num;
	int write_pos;				// 下一帧序号
	int read_pos;				
	int polling_once;
	int running;				
	int Pframe_pos;				// GOP序列中最后一个P帧的帧序号	
	int wait_Iframe;
	struct av_node node[MAX_NODE_NUM];	
}av_buffer;

typedef enum
{
 	AV_BUFFER_NULL =  0,
	AV_BUFFER_FULL =  1,
	AV_BUFFER_ERR  = -1,
}AV_BUFFER_STATUS;

av_buffer *init_av_buffer(int buf_size,int min_node_num, int max_node_num);
void deinit_av_buffer(av_buffer *buffer);
int write_buffer_data(av_buffer *buffer, char *data, int size, int keyframe, struct timeval tv, int index);
int read_buffer_data(av_buffer *buffer, char **data, int *size, int *keyframe, struct timeval *tv, int * index);
void read_write_pos_sync(av_buffer *buffer);
void start_buffer_data(av_buffer *buffer);
void stop_buffer_data(av_buffer *buffer);
void clean_buffer_data(av_buffer *buffer);

#endif



