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

#define LOG printf

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

enum {
	FRAME_VIDEO_TYPE    = 0,
	FRAME_AUDIO_TYPE    = 1,
	FRAME_NONE_TYPE     = 2,
};

int get_record_frame(AVPacket *pkt, int *delay , int frame_type)
{
	if(!ifmt_ctx)
		return FRAME_NONE_TYPE;
	int result = FRAME_NONE_TYPE;

	av_init_packet(pkt);

	if(video_dur >= VDuration && audio_dur >= ADuration)
	{
		LOG("#######read over!\n");
		result = FRAME_NONE_TYPE;
		pkt->data = NULL;
		pkt->size = 0;
		*delay = 0 ;
		return FRAME_NONE_TYPE;
	}


	if(frame_type == FRAME_AUDIO_TYPE && audio_dur < ADuration)
	{
		result = av_seek_frame(ifmt_ctx,audioindex,audio_dur,AVSEEK_FLAG_ANY);
		if(result < 0){
			LOG("audio av_seek_frame Err %d\n",result);
			return FRAME_NONE_TYPE;
		}
		result = av_read_frame(ifmt_ctx, pkt);
		if(result < 0){
			LOG("audio av_read_frame Err %d\n",result);	
			return FRAME_NONE_TYPE;
		}
		result = FRAME_AUDIO_TYPE;
		audio_dur += pkt->duration;
		*delay = pkt->duration * 1000 / 16000;
//		printf("audio : audio_dur = %d, %08d\n",audio_dur, ADuration);
	}

	else if(frame_type == FRAME_VIDEO_TYPE && video_dur < VDuration)
	{
//		LOG("get video frame %d %d\n",video_dur,VDuration);
		result = av_seek_frame(ifmt_ctx,videoindex,video_dur,AVSEEK_FLAG_ANY);
		if(result < 0){
			LOG("video av_seek_frame Err %d , %d , %d\n",result,video_dur, VDuration);
			return FRAME_NONE_TYPE;
		}
		result = av_read_frame(ifmt_ctx, pkt);
		if(result < 0){
			LOG("video av_read_frame Err %d\n",result);	
			return FRAME_NONE_TYPE;
		}

		result = FRAME_VIDEO_TYPE;
		video_dur += pkt->duration;
		*delay = pkt->duration * 1000 / 90000 ;
//		printf("video : video_dur = %d, %d , dur %d, size =%d\n",video_dur, VDuration,pkt->duration,pkt->size);
	}

	if(*delay > 1000)
		*delay = 1000;

	return result;
}




const char * file_path = "/tmp/mmcblk0p1/YsxCam/2018-11-08/23/2018_11_08_23_16_00.mp4";
int main(int argc, char ** argv)
{
    int ret = 0;
    
    AVPacket pkt;
    av_register_all();
//    av_packet_init(&pkt);
    av_packet_unref(&pkt);
//    record_demuxer_init(file_path);
   
//    get_record_frame(&pkt, 50 * 1000, FRAME_VIDEO_TYPE);
//    av_packet_unref(&pkt);
    
    
    return 0;
}


