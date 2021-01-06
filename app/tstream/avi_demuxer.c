#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/statfs.h>

#include "include/common_env.h"
#include "include/common_func.h"
#include "sdrecord.h"

int videoindex=-1,audioindex=-1;  
AVFormatContext *ifmt_ctx = NULL ;	

static int record_demuxer_init1(char *in_filename)
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
		printf( "Could not open input file.\n");  
		return -1;
	}  
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {  
		printf( "Failed to retrieve input stream information\n");  
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
	
	av_dump_format(ifmt_ctx, 0, in_filename, 0);  
	printf("videoindex = %d , audioindex = %d\n",videoindex,audioindex);
	return ret;
}

//static int get_record_frame(AVFormatContext *ifmt_ctx)
//{
//	 AVPacket pkt; 
//	if(av_read_frame(ifmt_ctx, &pkt) < 0)	
//		return -1;

//	return 0;
//}

int main(int argc , char **argv)
{
	AVPacket pkt; 
	int ret;

	if(record_demuxer_init1(argv[1]) != 0 )
	{
		return -1;
	}

	FILE *fp_264,*fp_aac;

	fp_264 = fopen("demuxer_stream.264","w");
	fp_aac = fopen("demuxer_stream.aac","w");

	struct timeval tv1,tv2,tv3,tv4;
	memset(&tv1,0,sizeof(struct timeval));
	memset(&tv2,0,sizeof(struct timeval));
	memset(&tv3,0,sizeof(struct timeval));
	memset(&tv4,0,sizeof(struct timeval));
	while(1)
	{
		ret = av_read_frame(ifmt_ctx, &pkt);
		

		if(ret < 0)	{
			printf("ret = %d\n",ret);
			break;
		}
		printf("stream index = %d , pts = %ld , duration=%lu\n",pkt.stream_index,pkt.pts,pkt.duration);
		if(pkt.stream_index == videoindex)
		{			
//			LOG("Video PTS:%ld , DTS:%d\n",pkt.pts,pkt.dts);	
			if( pkt.flags & AV_PKT_FLAG_KEY )
			{
				gettimeofday(&tv2,NULL);
				int delta = (tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec);
//				LOG("Video Interval time :%d\n",delta);
				memcpy(&tv1,&tv2,sizeof(struct timeval));
			}
			fwrite(pkt.data ,1,pkt.size , fp_264);
		}
		else if(pkt.stream_index == audioindex)
		{
//			LOG("frame size %ld\n",pkt.size);
//			
//			LOG("Audio PTS:%ld , DTS:%d\n",pkt.pts,pkt.dts);			
			fwrite(pkt.data ,1,pkt.size , fp_aac);
		}	
		else 
			continue;

		av_free_packet(&pkt); 
	}
	
	fclose(fp_264);
	fclose(fp_aac);
	av_close_input_file(ifmt_ctx);

}

