/*************************************************************************
	> File Name: ff_dmuxer.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Thu 08 Nov 2018 11:30:01 PM CST
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "linklist.h"

static AVFormatContext * ifmt_ctx = NULL;
static int videoindex = -1 , audioindex=-1;
static uint32_t VDuration =0 ,ADuration = 0;
static uint32_t audio_dur = 0, video_dur = 0;


int record_demuxer_init(char *in_filename)
{
	int ret = 0;
	int i;

	if(NULL == in_filename){
		LOG("file name is NULL !\n");
		return -1;
	}

	av_register_all();

	//Input
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		LOG( "Could not open input file.\n"); ifmt_ctx = NULL;
		return -1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		LOG( "Failed to retrieve input stream information\n");
		avformat_free_context(ifmt_ctx);ifmt_ctx = NULL;
		return -1;
	}

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
		}else if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			audioindex=i;
		}else{
			break;
		}
	}
	VDuration = ifmt_ctx->streams[videoindex]->duration;
	ADuration = ifmt_ctx->streams[audioindex]->duration;
	audio_dur = video_dur = 0;

	LOG("VDuration %u , ADuration %u\n",VDuration,ADuration);
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	
	return ret;
}


int main(int argc, char ** argv)
{
    int ret = 0;
    return 0;
}


