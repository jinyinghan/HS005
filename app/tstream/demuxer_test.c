/*************************************************************************
	> File Name: demuxer_test.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Wed 28 Nov 2018 05:38:52 PM CST
 ************************************************************************/

#include <stdio.h>
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "log.h"


enum{
	FRAME_VIDEO_TYPE =1,
	FRAME_AUDIO_TYPE,
	FRAME_NONE_TYPE
};

static AVFormatContext *ifmt_ctx = NULL ;
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

	/* initialize libavcodec, and register all codecs and formats */
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
			printf("###Video codec_tag=%d###\n", ifmt_ctx->streams[i]->codec->codec_tag);
		}else if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			audioindex=i;
			printf("###Video codec_tag=%d###\n", ifmt_ctx->streams[i]->codec->codec_tag);
		}else{
			break;
		}
	}
	VDuration = ifmt_ctx->streams[videoindex]->duration;
	ADuration = ifmt_ctx->streams[audioindex]->duration;//以微秒为单位
	audio_dur = video_dur = 0;
	AVStream * st = NULL;
	LOG("VDuration %u , ADuration %u\n", 
			VDuration, 
			ADuration);
//	st = &ifmt_ctx -> streams[videoindex];
//	LOG("video duration: %u\n", st -> duration * av_q2d(st -> time_base));

			
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	
	return ret;
}

int get_record_frame(AVPacket *pkt, int *delay , int frame_type)
{
	static unsigned int pre_data_size = 0;
	static unsigned int pre_dts = 0;
	static unsigned int pre_seek = 0;
	if(!ifmt_ctx)
		return FRAME_NONE_TYPE;
	int result = FRAME_NONE_TYPE;

	av_init_packet(pkt);

//    av_log(NULL, AV_LOG_ERROR, "%d\n", __LINE__);


	if(video_dur >= VDuration && audio_dur >= ADuration)
	{
		LOG("#######read over!\n");
		result = FRAME_NONE_TYPE;
		pkt->data = NULL;
		pkt->size = 0;
		*delay = 0 ;
		return FRAME_NONE_TYPE;
	}

//    av_log(NULL, AV_LOG_ERROR, "%d\n", __LINE__);

	if(frame_type == FRAME_AUDIO_TYPE && audio_dur < ADuration)
	{
		result = av_seek_frame(ifmt_ctx,audioindex,audio_dur,AVSEEK_FLAG_ANY);
		if(result < 0){
			LOG("audio av_seek_frame Err %d\n",result);
			return FRAME_NONE_TYPE;
		}
        
 //       av_log(NULL, AV_LOG_ERROR, "%d\n", __LINE__);
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
//		LOG("get video frame %d %d\n",video_dur, VDuration);
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

		if (pre_data_size == pkt -> size && pre_dts == pre_dts) {
			EMGCY_LOG("seek error!!!\n");
			EMGCY_LOG("pre seek: %d\n", pre_seek);
			EMGCY_LOG("cur seek: %d\n", video_dur);
			pkt -> size = 0;
		}		

		result = FRAME_VIDEO_TYPE;
		video_dur += pkt->duration;
		*delay = pkt->duration * 1000 / 90000 ;
		pre_data_size = pkt -> size;
		pre_dts = pkt -> dts;
		pre_seek = video_dur;
//		printf("video : video_dur = %d, %d , dur %d, size =%d\n",video_dur, VDuration,pkt->duration,pkt->size);
	}
//    av_log(NULL, AV_LOG_ERROR, "%d\n", __LINE__);

#define FF_TIMESTAMP_DBG
#ifdef FF_TIMESTAMP_DBG
	int stream_index = -1;
	if (FRAME_VIDEO_TYPE == frame_type) {
		stream_index = videoindex;
	}
	else if (FRAME_AUDIO_TYPE == frame_type) {
		stream_index = audioindex;
	}

	if (1) {	
		if (stream_index == 0) {
			AVRational * time_base = &ifmt_ctx -> streams[stream_index]->time_base;
			printf("stream[%d] size=%06d pts=%09lld dts=%09lld  %06d isKeyFrame:%d time:%0.5g\n",
					stream_index,
					pkt -> size,
					pkt -> pts,
					pkt -> dts,
					ifmt_ctx -> streams[stream_index]->r_frame_rate.num,
					pkt -> flags,
					av_q2d(*time_base) * pkt -> dts);
		}
	}
#endif 

	if (*delay > 1000) {
		*delay = 1000;
	}
//      av_log(NULL, AV_LOG_ERROR, "get frame type: %d, data size: %d\n", frame_type, pkt -> size);

	return result;
}


int get_record_frame2(AVPacket *pkt, int *delay , int frame_type)
{
	int ret = 0;
	av_init_packet(pkt);
	ret = av_read_frame(ifmt_ctx, pkt);
	if (ret < 0) {
		ret = FRAME_NONE_TYPE;
		goto exit;	
	}

	if (videoindex == pkt -> stream_index) {
		ret = FRAME_VIDEO_TYPE;
		frame_type = FRAME_VIDEO_TYPE;
		*delay = pkt->duration * 1000 / 90000 ;
	}
	else if (audioindex == pkt -> stream_index) {
		ret = FRAME_AUDIO_TYPE;
		frame_type = FRAME_AUDIO_TYPE;
		*delay = pkt->duration * 1000 / 16000;
	}
	
//#define FF_TIMESTAMP_DBG
#ifdef FF_TIMESTAMP_DBG
	int stream_index = -1;
	if (FRAME_VIDEO_TYPE == frame_type) {
		stream_index = videoindex;
	}
	else if (FRAME_AUDIO_TYPE == frame_type) {
		stream_index = audioindex;
	}

	if (1) {	
		if (stream_index == 0) {
			AVRational * time_base = &ifmt_ctx -> streams[stream_index]->time_base;
			printf("stream[%d] size=%06d pts=%09lld dts=%09lld  %06d isKeyFrame:%d time:%0.5g delay: %d\n",
					stream_index,
					pkt -> size,
					pkt -> pts,
					pkt -> dts,
					ifmt_ctx -> streams[stream_index]->r_frame_rate.num,
					pkt -> flags,
					av_q2d(*time_base) * pkt -> dts,
					*delay);
		}
	}
#endif 

exit:
	return ret;
}



void reset_record_frame(AVPacket *pkt)
{
	if(pkt)
	{
		av_packet_unref(pkt);
	}
}

void record_demuxer_uninit()
{
	reset_record_frame(NULL);

	if(ifmt_ctx);
	{
		avformat_close_input(&ifmt_ctx);
		ifmt_ctx = NULL;
	}
	return;
}


int main(int argc, char ** argv)
{
	int ret = 0;
	if (argc != 2) {
		printf("argument error!\n");
		return -1;
	}
	char filename[128] = {0};
	AVPacket pkt = {0};
	unsigned int delay = 0;
	strcpy(filename, argv[1]);
	LOG("file name: '%s'\n", filename);

	ret = record_demuxer_init(filename);
	if (ret < 0) {
		LOG("demuxer init failed!\n");
		return -1;
	}
	FILE * fp = fopen("record.h265", "wb");
	FILE * afp = fopen("record.aac", "wb");
	while(1)
	{	
		ret = get_record_frame2(&pkt, &delay, FRAME_VIDEO_TYPE);
		if (ret == FRAME_VIDEO_TYPE) {
			fwrite(pkt.data, sizeof(char), pkt.size, fp);
			fflush(fp);
			usleep(delay*1000);
		}
		else if (ret == FRAME_AUDIO_TYPE) {
			fwrite(pkt.data, sizeof(char), pkt.size, afp);
			fflush(afp);
		}
		reset_record_frame(&pkt);
	}	
	return ret;
}


