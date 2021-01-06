#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>

#include "ff_muxer.h"
#include "media.h"
#include "include/common_env.h"
#include "include/common_func.h"

//static const char confBUF[2] = {0x15,0x88};   //for 8K
//static const char confBUF[2] = {0x14,0x8};  	//for 16k


static int sd_flag = 0;
struct tm curr_time={0};
static void *md_record_handle = NULL;
extern G_StructRecordInfo g_recordinfo_struct[2];

//#define REAL_FPS ((QCamGetModeStatus()==QCAM_FILL_DAY) ?  (15) : (10))

static int frame_sd_write_v  = 0;
static int frame_sd_write_a  = 0;

 int64_t samples_count[2] = {0};
 int64_t next_pts[2] = {0};
#define YSX_SAMPELRAE   16000
#define NUM_PER_SAMPLE   1024;

pthread_mutex_t CheckerMutex;
pthread_cond_t CheckerCond;

int write_flag[2] = {0};


unsigned int bPause[2]={0}; 
#ifdef USE_NEW_FFMPEG_RECORD
int create_fps[2] = {0};
#endif
int get_real_fps(){

	//int real_fps=real_fps=((QCamGetModeStatus()==QCAM_MODE_DAY) ?  (15) : (10));
	int real_fps = 0;
	real_fps = qcam_get_fps();
	if(real_fps == -1 || real_fps == 0){
		
		return 15;
	}
	return real_fps;  
}
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
 {
	   AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
 
	   printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
			  av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
			  av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
			  av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
			  pkt->stream_index);
}

void* vrec_checker(void* arg)
{
	
	//int check_count = 0;
	
	for(;;)
	{
		bPause[0]=1;
		pthread_mutex_lock(&CheckerMutex);
		pthread_cond_wait(&CheckerCond, &CheckerMutex);
		pthread_mutex_unlock(&CheckerMutex);
		bPause[0]=0;
		if (!g_enviro_struct._loop)
			break;

		del_oldest_record(g_enviro_struct.t_sdpath);
	
	}
	

	pthread_mutex_destroy(&CheckerMutex);
	pthread_cond_destroy(&CheckerCond);
	
	return NULL;
}


void Record_Init()
{
	av_log_set_level(AV_LOG_DEBUG);
	
	memset(&fulltime_mp4,0,sizeof(struct mp4_Info));
	fulltime_mp4.oc = NULL;
	pthread_mutex_init(&fulltime_mp4.mutex,NULL);	
#ifdef YSX_CLOUD	
	memset(&md_mp4,0,sizeof(struct mp4_Info));
	md_mp4.oc = NULL;
	pthread_mutex_init(&md_mp4.mutex,NULL);
#endif	

		int ret;
		#if 0
	    pthread_mutex_init(&CheckerMutex, NULL);
		pthread_cond_init(&CheckerCond, NULL);
		pthread_t checker_thread;
        if((ret = pthread_create(&checker_thread, NULL, &vrec_checker, NULL)) < 0)
        {
                LOG("perdetect_thread create fail, ret=[%d]\n", ret);
                return -1;
        }
        pthread_detach(checker_thread);
	  	#endif




	return ;
}

#define STR_SIZE sizeof("rwxrwxrwx")
static int Check_Record_FilePerm(char *filename)
{
	struct stat file_stat;
	struct tm tb;
	char file_name[128] = {'\0'};
	char file_path[32] = {'\0'};

	strcpy(file_path, g_enviro_struct.t_sdpath);
	const char fmt[] = "%Y_%m_%d_%H_%M_%S";  
	if (strptime(filename, fmt, &tb)) {
		sprintf(file_name, "%s/YsxCam/%04d-%02d-%02d/%02d/%s",
							file_path, 1900+tb.tm_year, tb.tm_mon+1, 
							tb.tm_mday, tb.tm_hour, filename);
	}
	stat(file_name, &file_stat);

	static char str[STR_SIZE];
    snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
        (file_stat.st_mode & S_IRUSR) ? 'r' : '-', (file_stat.st_mode & S_IWUSR) ? 'w' : '-',
        (file_stat.st_mode & S_IXUSR) ? 'x' : '-',
       
        (file_stat.st_mode& S_IRGRP) ? 'r' : '-', (file_stat.st_mode & S_IWGRP) ? 'w' : '-',
        (file_stat.st_mode & S_IXGRP) ? 'x' : '-',
    
        (file_stat.st_mode & S_IROTH) ? 'r' : '-', (file_stat.st_mode & S_IWOTH) ? 'w' : '-',
        (file_stat.st_mode & S_IXOTH) ? 'x' : '-');

	if(strncmp("r-xr-xr-x", str, STR_SIZE) == 0)
		return 1;
	else
		return 0;
	
}

/*按时间顺序索引文件*/
static int last_min=0;
int CreateRecordIndex(char *basePath, FILE *fIndex)
{
    struct dirent **namelist,*ptr;
	struct stat s_buf;
    int n;
	char nameTemp[128];

    n = scandir(basePath, &namelist, NULL, alphasort);
    if (n < 0)
	{
		LOG("scan dir error:%s",strerror(errno));
		return -1;
	}

	int i;
	for(i=0;i<n;i++)
	{
		ptr = namelist[i];
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)	 ///current dir OR parrent dir
				continue;

		memset(nameTemp,0,sizeof(nameTemp));
		strcpy(nameTemp,basePath);
		strcat(nameTemp,"/");
		strcat(nameTemp,ptr->d_name);

		stat(nameTemp,&s_buf);
		if(S_ISDIR(s_buf.st_mode))
		{
//			printf("### %s\n",ptr->d_name);
			CreateRecordIndex(nameTemp,fIndex);
			//rmdir(nameTemp);
		}
		else if(S_ISREG(s_buf.st_mode))
		{
			//if(Check_Record_FilePerm(ptr->d_name))
			{
				RIndex rtmp;
				struct tm tb;

				const char fmt[] = "%Y_%m_%d_%H_%M_%S"; 				//"2000_02_01_00_09_33.mp4";
				if (strptime((const char *)ptr->d_name, fmt, &tb))
				{
					if (tb.tm_min != last_min)
					{
						rtmp.iy = 1900+tb.tm_year;
						rtmp.im = tb.tm_mon+1;
						rtmp.id = tb.tm_mday;
						rtmp.ih = tb.tm_hour;
						rtmp.imi = tb.tm_min;
						rtmp.timelen = 60- tb.tm_sec;
						fwrite(&rtmp, sizeof(RIndex), 1, fIndex);
						last_min = tb.tm_min;
					}
				}
			}
		}

		free(ptr);
	}
    free(namelist);
	return 0;
}

static int err_cnt = 0;
struct timeval lst_vid = {0,0};

int sync_record_index()
{
	LOG("Creat New record index\n");
	err_cnt = 0;
	char tmp[128];

	FILE *fp = fopen(RECORD_INDEX,"wb");
	if(NULL == fp){
		LOG("#Create file %s error#\n",RECORD_INDEX);
		return -1;
	}

	CLEAR(tmp);
	sprintf(tmp,"%s/YsxCam/",g_enviro_struct.t_sdpath);
	CreateRecordIndex(tmp,fp) ;
	fclose(fp);

	return 0;
}

#define MAX_NALUS_SZIE 5124
int write_video_frame(struct mp4_Info *info, int keyframe, uint8_t *buf, int len, const struct timeval *tv,enum record_channel_number record_channel)
{
	int ret = 0;
	pthread_mutex_lock(&info->mutex);
	if(info->oc)
	{
		#if 0
        if (buf[0] == 0x0 && buf[1] == 0x0 && buf[2] == 0x0 && buf[3] == 0x1 &&  buf[4] == 0x40) {
            AVCodecContext * c;      
            c = info->video_st->codec;
            if (c->extradata_size == 0) {           
                int nalus_num = 0;
                char * ptrBuf = NULL;
                int num = 0;
                ptrBuf = buf;
                if (ptrBuf[4] == 0x40) {               
                    for(num = 0; num < MAX_NALUS_SZIE; num++) {
                        if( num >= len ) {
                            printf("not get nalus\n");
                            break;
                        }
                    
                        if( ptrBuf[num]  == 0x4e && ptrBuf[num-1]  == 0x1  && ptrBuf[num-2]  == 0x0 ) {
                            nalus_num = num - 4;                   
                            int i = 0;
                            for( i = 0; i < c->extradata_size;i++){
                                printf("%x ",c->extradata[i]); 
                            }
                            printf("[%d]\n",c->extradata_size);
                            nalus_num = num - 4;
                            break;
                        }
                    }                   
                }
            }
            else {
//                av_log(NULL, AV_LOG_ERROR, "extra data size: %d\n", c->extradata_size);
            }
        }
    	#endif
		AVPacket pkt={0};
		av_init_packet(&pkt);
		pkt.data = buf;
		pkt.size = len;
		
		if(keyframe)
			pkt.flags |= AV_PKT_FLAG_KEY;
	
		pkt.stream_index = info->video_st->index;
		#if 1
		time_t tmp;
		
		uint64_t now = (tv->tv_sec*1000000+tv->tv_usec);
		uint64_t t_diff = abs(now-info->vid_usec);
		#else
		int64_t tmp;
		
		int64_t now = (tv->tv_sec*1000000+tv->tv_usec);
		int64_t t_diff = abs(now-info->vid_usec);
		


		#endif
	
		
		
		#ifdef USE_NEW_FFMPEG_RECORD
				
		pkt.dts=pkt.pts = next_pts[record_channel]++;
	
		av_packet_rescale_ts(&pkt, (AVRational){1,create_fps[record_channel] < get_real_fps() ? 10:get_real_fps()/*15*/}, (AVRational){1, 90000});

		if(access("/tmp/lzf_ok_video",F_OK) == 0)
		log_packet(info->oc, &pkt);
		#else
		#if 1
		if (info->vid_usec <= 0) 
			tmp = 90000*1/15;//REAL_FPS;
		else if(t_diff>=(1000000 * 1))
			tmp = 90000*1/10;
		else
			tmp = 90*t_diff/1000 ;//   90000 / 100000 = 90 / 1000
		#endif
		//tmp = 90000*1/15;
		info->last_vid_pts += tmp; 
		pkt.pts =pkt.dts = info->last_vid_pts;
		//pkt.pts = pkt.dts;
		
		info->vid_usec = now;
		//printf("video  %lld %lld~~~~~~~~~~~~~~~~~~~~~\n",pkt.pts,pkt.dts);
		//printf("video  %ld ~~~~~~~~~~~~~~~~~~~~~\n",pkt.dts);
		#endif
	
	    static int num=0;
		if(num++ % 100 == 0 && record_channel == RECORD_CHANNEL_ONE){
	
		int real=get_real_fps();
		//printf("=============================  now fps %d %ld \n",real,pkt.dts);
		//printf("%ld \n",pkt.pts);
		}
		
		if ((ret=av_interleaved_write_frame(info->oc, &pkt)) != 0) {
			LOG("Error while writing video frame %d %s %ld %ld\n",ret,av_err2str(ret),pkt.dts,pkt.pts);//
			ret = -1;
		}else{
			if(RECORD_CHANNEL_ONE == record_channel)
				frame_sd_write_v++;
		}


		
        av_free_packet(&pkt);	
	}
	
	pthread_mutex_unlock(&info->mutex);
	return ret;
}

int write_audio_frame(struct mp4_Info *info,uint8_t *buf, int len,const struct timeval *tv,enum record_channel_number record_channel)
{
	int ret = 0;
    int rc = 0;
	pthread_mutex_lock(&info->mutex);
	//write_flag[1] = 1;
	if(info->oc)
	{
		AVPacket pkt={0};
		av_init_packet(&pkt);
		pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = info->audio_st->index;

#ifdef ADD_AUDIO_FILTER
        LOG("audio filter start!\n");
        rc = av_bitstream_filter_filter(info->aacbsfc, info->audio_st->codec, NULL, 
                                    &pkt.data, &pkt.size, buf, len, 0);   
        if (rc < 0) {
            LOG("audio bitstream filter failed!\n");
            ret = -1;
        }
#else 
        pkt.data = buf + 7;
        pkt.size = len - 7;
#endif 

#ifdef AUDIO_FILTER_DBG
        static int first = 1;
        static unsigned int frame_count = 0;
        char pre_name[128] = {0};
        char after_name[128] = {0};
        FILE * pre_fp = NULL;
        FILE * after_fp = NULL;

        snprintf(pre_name, sizeof(pre_name) - 1, "./adts/pre_adts-%d.aac", frame_count);
        snprintf(after_name, sizeof(after_name) - 1, "./adts/after_adts-%d.aac", frame_count);
        pre_fp = fopen(pre_name, "wb");
        after_fp = fopen(after_name, "wb");

        fwrite(buf, sizeof(char), len, pre_fp);
        fflush(pre_fp);
        fclose(pre_fp);
        
        fwrite(pkt.data, sizeof(char), pkt.size, after_fp);
        fflush(after_fp); 
        fclose(after_fp);  
        frame_count ++;
#endif  
		//uint64_t tmp;
		#ifdef USE_NEW_FFMPEG_RECORD


	    pkt.pts=av_rescale_q(samples_count[record_channel], (AVRational){1, 16000}, (AVRational){1, 16000});
		pkt.dts=pkt.pts;
		
		samples_count[record_channel] += NUM_PER_SAMPLE;
		
		av_packet_rescale_ts(&pkt,(AVRational){1, 16000},(AVRational){1, 16000});
		#else 

		#if 1
		time_t tmp;	
		uint64_t now = (tv->tv_sec*1000000+tv->tv_usec);
		uint64_t t_diff = abs(now -info->aud_usec);

		#else
		int64_t tmp;
		
		int64_t now = (tv->tv_sec*1000000+tv->tv_usec);
		int64_t t_diff = abs(now-info->vid_usec);
		#endif
		
	
		if(info->aud_usec <= 0 || t_diff>=(1000000 * 1))
			tmp = 16000*1/10;
		else
			tmp = 16*t_diff/1000 ;//   90000 / 100000 = 90 / 1000
		#endif

	
		
		info->last_aud_pts += tmp; 
		pkt.pts = pkt.dts = info->last_aud_pts;
		//pkt.pts = pkt.dts;
		
		
		info->aud_usec = now;
		

	
		
		if(access("/tmp/lzf_ok_audio",F_OK) == 0)
		log_packet(info->oc, &pkt);
		//printf("audio %d %d ~~~~~~~~~~~~~~~~~~~~~\n",pkt.pts,pkt.dts);
		if ((ret=av_interleaved_write_frame(info->oc, &pkt)) != 0) {
			LOG("Error while writing audio frame %d %s %ld %ld \n",ret,av_err2str(ret),pkt.dts,pkt.pts);
			ret = -1;
		}else{
			if(RECORD_CHANNEL_ONE == record_channel)
			frame_sd_write_a++;
		}
        
	}
	
	pthread_mutex_unlock(&info->mutex);
	return ret;
}

static AVStream *add_mdaudio_stream(AVFormatContext *oc, enum AVCodecID codec_id)
{  
    AVStream *st = avformat_new_stream(oc, NULL);
    if (!st) {
		av_log(NULL,AV_LOG_INFO,"audio new stream error !\n");
        return NULL;
    }
		
	AVCodecContext *codec_ctx = st->codec;//需要使用avcodec_free_context释放

	codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;    	
    codec_ctx->codec_id = codec_id;

    /* put sample parameters */
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    codec_ctx->bit_rate = 16000;
    codec_ctx->sample_rate = 16000;
    codec_ctx->channels = 1;
	codec_ctx->bits_per_coded_sample = 16;
	codec_ctx->frame_size = 16;
	codec_ctx->channel_layout = AV_CH_LAYOUT_MONO;

	st->id = oc->nb_streams -1;
	st->time_base = (AVRational){1, 16000};

	st->codec->extradata = aac_extra_data;
	st->codec->extradata_size = AAC_EXTRA_DATA_SIZE;	

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	avcodec_parameters_from_context(st->codecpar, codec_ctx);
    if (st -> codecpar -> extradata) {
        av_log(NULL, AV_LOG_ERROR, "audio extra data is not null!\n");
    }
	
    av_log(NULL, AV_LOG_ERROR, "add mdaudio stream ok\n");
	return st;
}  

static AVStream *add_audio_stream(AVFormatContext *oc, enum AVCodecID codec_id,enum mRecord_type record_type)
{  
    AVStream *st = avformat_new_stream(oc, NULL);
    if (!st) {
		av_log(NULL,AV_LOG_INFO,"audio new stream error !\n");
        return NULL;
    }
		
	AVCodecContext *codec_ctx = st->codec;//需要使用avcodec_free_context释放

	codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;    	
    codec_ctx->codec_id = codec_id;

    /* put sample parameters */
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    codec_ctx->bit_rate = 16000;
    codec_ctx->sample_rate = 16000;
    codec_ctx->channels = 1;
	codec_ctx->bits_per_coded_sample = 16;
	codec_ctx->frame_size = 16;
	codec_ctx->channel_layout = AV_CH_LAYOUT_MONO;

	st->id = oc->nb_streams -1;
	st->time_base = (AVRational){1, 16000};

	st->codec->extradata = aac_extra_data;
	st->codec->extradata_size = AAC_EXTRA_DATA_SIZE;	

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	avcodec_parameters_from_context(st->codecpar, codec_ctx);
    if (st -> codecpar -> extradata) {
        av_log(NULL, AV_LOG_ERROR, "audio extra data is not null!\n");
    }
    av_log(NULL, AV_LOG_ERROR, "add audio stream ok\n");
	return st;
}   

/* add a video output stream */
static AVStream *add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id, enum mRecord_type record_type)
{
	#ifdef USE_NEW_FFMPEG_RECORD
	create_fps[0] = get_real_fps();
	#endif
	
    AVStream *st = avformat_new_stream(oc, NULL);
    if (!st) {
		av_log(NULL,AV_LOG_INFO,"video new stream error !\n");
        return NULL;
    }
	
	AVCodecContext *codec_ctx = st->codec;

	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->codec_id = codec_id;

	 if((record_type==TYPE_SD_RECORD)&&(g_enviro_struct.storage_resolution==YSX_RESOLUTION_1080P)){
    codec_ctx ->width = 1920;    
    codec_ctx ->height = 1080;
    codec_ctx->pix_fmt = AV_PIX_FMT_NV12;
	#ifdef USE_NEW_FFMPEG_RECORD
	codec_ctx->time_base = (AVRational){1,create_fps[0]/*15*/};
	#else 
	codec_ctx->time_base = (AVRational){1,15};
	#endif
	codec_ctx->bit_rate = 1024000;
	codec_ctx->codec_tag = 0;	 

	st->id = oc->nb_streams -1;
	st->time_base = (AVRational){1, 90000};
	st->start_time = 0;
	}else if((record_type==TYPE_SD_RECORD)&&(g_enviro_struct.storage_resolution==YSX_RESOLUTION_360P)){
	 codec_ctx ->width = 640;     
    codec_ctx ->height = 360;
    codec_ctx->pix_fmt = AV_PIX_FMT_NV12;
	#ifdef USE_NEW_FFMPEG_RECORD
	codec_ctx->time_base = (AVRational){1,create_fps[0]/*15*/};
	#else 
	codec_ctx->time_base = (AVRational){1,15};
	
	#endif
	codec_ctx->bit_rate = 128000;
	codec_ctx->codec_tag = 0;	 

	st->id = oc->nb_streams -1;
	st->time_base = (AVRational){1, 90000};
	st->start_time = 0;
	}
	if(AV_CODEC_ID_H264==codec_id){
	
		codec_ctx->qmin=10;
		codec_ctx->qmax=51;
	
	}

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    avcodec_parameters_from_context(st->codecpar, codec_ctx);
    if (st -> codecpar -> extradata) {
        av_log(NULL, AV_LOG_ERROR, "extra data is not null!\n");
    }
	av_log(NULL, AV_LOG_ERROR, "add video stream ok \n");
    return st;
}

static AVStream *add_mdvideo_stream(AVFormatContext *oc, enum AVCodecID codec_id)
{
	#ifdef USE_NEW_FFMPEG_RECORD
	create_fps[1] = get_real_fps();
	#endif
    AVStream *st = avformat_new_stream(oc, NULL);
    if (!st) {
		av_log(NULL,AV_LOG_INFO,"audio new stream error !\n");
        return NULL;
    }
	
	AVCodecContext *codec_ctx = st->codec;

	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->codec_id = codec_id;

    codec_ctx ->width = 640;      
    codec_ctx ->height = 360;
    codec_ctx->pix_fmt = AV_PIX_FMT_NV12;
	#ifdef  USE_NEW_FFMPEG_RECORD
	codec_ctx->time_base = (AVRational){1,create_fps[1]/*15*/};
	#else 
	codec_ctx->time_base = (AVRational){1,15};
	#endif
	
	codec_ctx->bit_rate = 128000;
	codec_ctx->codec_tag = 0;	

	//codec_ctx->qmin=10;
	//codec_ctx->qmax=51;

	st->id = oc->nb_streams -1;
	st->time_base = (AVRational){1, 90000};
	st->start_time = 0;

	st->codec->extradata = h264_extra_data;
	st->codec->extradata_size = H264_EXTRA_DATA_SIZE;	

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    avcodec_parameters_from_context(st->codecpar, codec_ctx);
    if (st -> codecpar -> extradata) {
        av_log(NULL, AV_LOG_ERROR, "extra data is not null!\n");
    }

	av_log(NULL, AV_LOG_ERROR, "add mdvideo stream ok \n");
    return st;
}

int mp4_mdrecord_init(struct mp4_Info *info ,const char *filename)
{
	int ret = -1;
	AVOutputFormat *fmt;
 
	av_register_all();
	//av_log_set_level(AV_LOG_INFO);

	ret = avformat_alloc_output_context2(&info->oc, NULL, NULL, filename);
	if (ret < 0 ||  !info->oc) {	 
		printf("Could not deduce output format from file extension: using MPEG.\n");		
		avformat_alloc_output_context2(&info->oc, NULL, "mpeg", filename);	
	}
	if (!info->oc)	
		return -1;	

	fmt= info->oc->oformat;
	fmt->video_codec = AV_CODEC_ID_H264;
#ifdef SET_RECORD_WITH_AUDIO
	fmt->audio_codec = AV_CODEC_ID_AAC;
#endif 

    info->video_st = add_mdvideo_stream(info->oc, fmt->video_codec);
#ifdef SET_RECORD_WITH_AUDIO
    info->audio_st = add_mdaudio_stream(info->oc, fmt->audio_codec);
#endif 

    av_log(NULL, AV_LOG_ERROR, "total stream num: %d\n", info -> oc -> nb_streams);
    
	if(info->video_st == NULL && info->audio_st == NULL){
		LOG("video_st alloc failed \n");
		return -1;
	}

	av_dump_format(info->oc, 0, filename, 1);

	if (!(fmt->flags & AVFMT_NOFILE)) 
	{	  
		if (avio_open(&info->oc->pb, filename, AVIO_FLAG_WRITE) < 0) 
		{		 
			LOG("Could not open '%s'\n", filename); 	
			return -1;		
		}	 
	}
	
	if(avformat_write_header(info->oc,NULL) < 0){
		LOG("write header error\n");
		return -1;		
	}
#ifdef SET_RECORD_WITH_AUDIO
#ifdef ADD_AUDIO_FILTER
	info->aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");	
	if(!info->aacbsfc)
	{
		LOG("bitstream filter init error\n");
		return -1;		
	}
#endif     
#endif 
	
	info->last_aud_pts = 0;
	info->last_vid_pts = 0;
	info->vid_usec = 0;
	info->aud_usec = 0;
	printf("Record init ok! \n");
	return 0;
}

int mp4_record_init(struct mp4_Info *info ,const char *filename,enum mRecord_type record_type)
{
	int ret = -1;
	AVOutputFormat *fmt;
 
	av_register_all();
	//av_log_set_level(AV_LOG_INFO);

	ret = avformat_alloc_output_context2(&info->oc, NULL, NULL, filename);
	if (ret < 0 ||  !info->oc) {	 
		printf("Could not deduce output format from file extension: using MPEG.\n");		
		avformat_alloc_output_context2(&info->oc, NULL, "mpeg", filename);	
	}
	if (!info->oc)	
		return -1;	

	fmt= info->oc->oformat;
	//if(g_recordinfo_struct[g_enviro_struct.storage_resolution].playload) 
	//	fmt->video_codec = AV_CODEC_ID_HEVC;
	//else
		fmt->video_codec = AV_CODEC_ID_H264;
	
#ifdef SET_RECORD_WITH_AUDIO
	fmt->audio_codec = AV_CODEC_ID_AAC;
#endif 

    info->video_st = add_video_stream(info->oc, fmt->video_codec,record_type);
#ifdef SET_RECORD_WITH_AUDIO
    info->audio_st = add_audio_stream(info->oc, fmt->audio_codec,record_type);
#endif 

    av_log(NULL, AV_LOG_ERROR, "total stream num: %d\n", info -> oc -> nb_streams);

    
	if(info->video_st == NULL && info->audio_st == NULL){
		LOG("video_st alloc failed \n");
		return -1;
	}

	av_dump_format(info->oc, 0, filename, 1);

	if (!(fmt->flags & AVFMT_NOFILE)) 
	{	  
		if (avio_open(&info->oc->pb, filename, AVIO_FLAG_WRITE) < 0) 
		{		 
			LOG("Could not open '%s'\n", filename); 	
			return -1;		
		}	 
	}
	
	if(avformat_write_header(info->oc,NULL) < 0){
		LOG("write header error\n");
		return -1;		
	}
#ifdef SET_RECORD_WITH_AUDIO
#ifdef ADD_AUDIO_FILTER
	info->aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");	
	if(!info->aacbsfc)
	{
		LOG("bitstream filter init error\n");
		return -1;		
	}
#endif     
#endif 
	
	info->last_aud_pts = 0;
	info->last_vid_pts = 0;
	info->vid_usec = 0;
	info->aud_usec = 0;

	frame_sd_write_v  = 0;
	frame_sd_write_a  = 0;
	printf("Record init ok! \n");
	return 0;
}


int mp4_record_close(struct mp4_Info *info,enum record_channel_number record_channel)
{
	LOG("close mp4 record ...\n");
	
	if (info->oc){
		av_write_trailer(info->oc);
		if (info ->video_st->codec->extradata) {
			info ->video_st->codec->extradata = NULL;
			info ->video_st->codec->extradata_size = 0;
		}

		if (info ->audio_st->codec->extradata) {
			info ->audio_st->codec->extradata = NULL;
			info ->audio_st->codec->extradata_size = 0;
		}
		
		
#ifdef ADD_AUDIO_FILTER
        if (info -> aacbsfc) {
            av_bitstream_filter_close(info -> aacbsfc);
            av_log(NULL, AV_LOG_ERROR, "close audio filter...\n");
        }
#endif         
        av_log(NULL, AV_LOG_ERROR, "close stream codec ok!\n");
	    /* close output */
    	if (info->oc && !(info->oc->oformat->flags & AVFMT_NOFILE))
        	avio_closep(&info->oc->pb);
		if(info->oc)
		avformat_free_context(info->oc);
		
		info->oc = NULL;
		av_log(NULL, AV_LOG_ERROR, "free context ok!\n");
	}
	if(record_channel == RECORD_CHANNEL_ONE){
	printf("write video audio num %d %d\n",frame_sd_write_v,frame_sd_write_a);
	LOG("write video audio num %d %d\n",frame_sd_write_v,frame_sd_write_a);
	frame_sd_write_v = 0;
	frame_sd_write_a = 0;
	}
	
 samples_count[record_channel] = 0;
 next_pts[record_channel] = 0;
 
	return 0;
}

int SdRecord_Stop(struct mp4_Info *info)
{
	LOG("SD record stop #\n");
	pthread_mutex_lock(&info->mutex);
	if(info->oc)
		mp4_record_close(info,RECORD_CHANNEL_ONE);
	sd_flag = 0;
	pthread_mutex_unlock(&info->mutex);
	return 0;
}

static int create_multi_dir(const char *path)
{
    int i, len;

    len = strlen(path);
    char dir_path[len+1];
    dir_path[len] = '\0';

    strncpy(dir_path, path, len);

    for (i=0; i<len; i++)
    {
        if (dir_path[i] == '/' && i > 0)
        {
            dir_path[i]='\0';
            if(access(dir_path, F_OK) != 0)
            {
                    if (mkdir(dir_path, 0755) < 0)
                    {
                            LOG("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
                            return -1;
                    }
            }
            dir_path[i]='/';
        }
    }

    return 0;
}

void dump_index(RIndex temp)
{
	printf("iy = %d ",temp.iy);
	printf("im = %d ",temp.im);
	printf("id = %d ",temp.id);
	printf("ih = %d ",temp.ih);
	printf("imi = %d ",temp.imi);
	printf("timelen = %d ",temp.timelen);
	printf("\n");
	return;
}


int gen_record_name(char *sdpath,char *filename)
{
	time_t timep;
	char dir_path[128];
	struct tm *p;

	if(NULL == filename){
		LOG("file name is NULL\n");
		return -1;
	}

	time(&timep);
	struct tm *t= localtime(&timep); 
	memcpy(&curr_time,t,sizeof(struct tm));
	p = t;

	CLEAR(dir_path);
	sprintf(dir_path,"%s/YsxCam/%04d-%02d-%02d/%02d/",sdpath,(1900+p->tm_year) , (1+p->tm_mon), p->tm_mday,p->tm_hour);

	create_multi_dir(dir_path);

	/*2016_12_14_10_22_49.mp4*/
	sprintf(filename,"%s%04d_%02d_%02d_%02d_%02d_00.mp4",dir_path,(1900+p->tm_year),
					(1+p->tm_mon), p->tm_mday, p->tm_hour,p->tm_min);
	return 0;
}

int remove_tmp_file(char *path)
{
	return 0 ;
	char sys_cmd[256];
	CLEAR(sys_cmd);
	snprintf(sys_cmd,sizeof(sys_cmd),"rm %stmp-* 2>/dev/null",path);
//	LOG("delet tmp --%s--\n",sys_cmd);
	AMCSystemCmd(sys_cmd);

	return 0;
}

int save_record_index(char *tmpfilename)
{
	FILE *fp = NULL;
	char filename[128];
	char *start;

	if(strlen(tmpfilename) == 0)
		return 0;

	//chmod(tmpfilename, S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

	/*sync record*/
	struct tm *p = &curr_time;
	RIndex tmp;
	tmp.iy  = (1900+p->tm_year);
	tmp.im  = (1+p->tm_mon);
	tmp.id  = p->tm_mday;
	tmp.ih  = p->tm_hour;
	tmp.imi = p->tm_min;
	tmp.timelen = 60-p->tm_sec;


	CLEAR(filename);
	sprintf(filename,"%s",RECORD_INDEX);
	fp = fopen(filename,"ab+");
	if(fp){
		fwrite(&tmp,sizeof(RIndex),1,fp);
		fclose(fp);
	}
	return 0;
}

/*
return:
	2 --- empty
	0 -- dir error
	>2 -- not empty
*/
int empty_dir(char *dirpath)
{
	DIR *dirp;
	int num=0;

	dirp = opendir(dirpath);
	while(dirp) {
		if (readdir(dirp) != NULL)
			++num;
		else
		{
			closedir(dirp);
			break;
		}
	}
	printf("num:%d\n",num);
	return num;
}
void del_oldest_dir(const char *path, const RIndex cur_info)
{
 int ret;
 FILE *fp ;
 RIndex tm_info;
 char buf[128];
 fp = fopen(RECORD_INDEX,"rb");
    if(NULL == fp)
    {
        printf("Failed to open %s\n",RECORD_INDEX);
        return;
    }
 	ret = fread(&tm_info, sizeof(RIndex), 1, fp);
    if(ret <= 0){
        printf("Empty index file\n");
 	   fclose(fp);
        return ;
    }
    fclose(fp);

 //ext: 2019-12-04   �����л�
 if(cur_info.iy != tm_info.iy || cur_info.im != tm_info.im || cur_info.id != tm_info.id)
 {
  int i;
  for(i=0;i<24;i++)//ɾ��24Сʱ���ļ���
  {
   snprintf(buf,sizeof(buf),"%s/YsxCam/%04d-%02d-%02d/%02d",
     path,cur_info.iy,cur_info.im,cur_info.id,i);
   rmdir(buf);//ɾ�����ļ���
  }  
  snprintf(buf,sizeof(buf),"%s/YsxCam/%04d-%02d-%02d",
    path,cur_info.iy,cur_info.im,cur_info.id);
  //rm_dir(buf);//rm 2019-12-04
  rmdir(buf);//ɾ�����ļ���
 }
 
 return ;
}

int del_oldest_record(const char *path)
{
	int ret;
	char index_name[64],buf[128],index_tmp[64];
	FILE *fp , *fp_tmp;
	RIndex tm_info;
	LOG("##### DEL RECORD FILE ##### \n");

	CLEAR(index_name);
	snprintf(index_name,sizeof(index_name),"%s",RECORD_INDEX);
	fp = fopen(RECORD_INDEX,"rb");
	if(NULL == fp)
	{
		LOG("Failed to open %s\n",index_name);
		return -1;
	}

	fseek(fp,0,SEEK_SET);
	ret = fread(&tm_info, sizeof(RIndex), 1, fp);
	if(ret <= 0){
		LOG("Empty index file\n");
		fclose(fp);
		return -1;
	}
//	dump_index(tm_info);

	CLEAR(index_tmp);
	snprintf(index_tmp,sizeof(index_tmp),"%s",RECORD_INDEX_TMP);
	fp_tmp = fopen(index_tmp,"wb");
	if(NULL == fp_tmp)
	{
		LOG("Failed to open %s\n",index_tmp);
		fclose(fp);
		return -1;
	}

	char temp[512] = {'\0'};
	if(NULL == temp)
	{
		
		LOG("malloc for temp error %s\n",strerror(errno));
		fclose(fp);
		fclose(fp_tmp);
		return -1;
	}

	while(1){
		ret = fread(temp,1,512,fp);
		if(ret <= 0 )
			break;
		else
			fwrite(temp,ret,1,fp_tmp);
	}

	fclose(fp);
	fclose(fp_tmp);

	remove(index_name);
	rename(index_tmp,index_name);

	CLEAR(buf);
	snprintf(buf,sizeof(buf),"%s/YsxCam/%04d-%02d-%02d/%02d/%04d_%02d_%02d_%02d_%02d_00.mp4",
			path,tm_info.iy,tm_info.im,tm_info.id,tm_info.ih,tm_info.iy,tm_info.im,tm_info.id,tm_info.ih,tm_info.imi);

	//chmod(buf, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);

	LOG("remove file %s\n",buf);
	if(remove(buf) != 0)
		perror("remove file\n");
	#if 0
	char *start = rindex(buf,'/');
	int len = strlen(start)-1;
	memset(start+1,0,len);
	remove_tmp_file(buf);	/*先删除临时文件*/
	if(empty_dir(buf) == 2)
		rmdir(buf);

	start = rindex(buf,'/');
	len = strlen(start)-1;
	memset(start+1,0,len);
	if(empty_dir(buf) == 2)
		rmdir(buf);
	#endif
	//del_oldest_dir(path,tm_info);
	return 0;
}

int readonly_check()
{
	char buffer[256];
	CLEAR(buffer);

	FILE *read_fp = popen("mount | grep mmc | grep \"(ro\"","r");
	if (read_fp != NULL)
	{
		fread(buffer, sizeof(char), 50, read_fp);
		if (strlen(buffer) != 0)
		{
			pclose(read_fp);
			LOG("## SD Readonly ## , Disable mp4 record\n");
			LOG("## SD Readonly ## , Disable mp4 record\n");
			LOG("## SD Readonly ## , Disable mp4 record\n");

			remove("/tmp/sd_ok");
			return 1;
		}
	}
	pclose(read_fp);
	return 0;
}

static int vrec_storage_is_full(const char *szRecordPath,unsigned int dwReversedSize)
{


	#if 0
	unsigned long avalible_size=0;
	struct statfs s;
	if (statfs(szRecordPath, &s) != 0) {
		LOG("stafs failed: %s\n",strerror(errno));
		LOG("statfs failed for path->[%s]\n", szRecordPath);
		return -1;
	}

		
	avalible_size = (s.f_bavail >> 10) * (s.f_bsize >> 10);

	if(avalible_size <= 20){
	 
		LOG("the SDcard is full ,we will not to record ,please format it \n");
		return -1;
	}
	
	if (avalible_size <= dwReversedSize)
	{
		LOG("No Space For SD record !\n");
		return 1;
	}
	#endif
	struct statfs diskInfo; //系统stat的结构体
	static int check_cnt = 0;
	
	
		if(readonly_check())
			return -1;

		if (statfs(szRecordPath, &diskInfo) == -1){   //获取分区的状态
			LOG("stafs failed: %s\n",strerror(errno));
			LOG("statfs failed for path->[%s]\n",szRecordPath);
			return -1;
		}
	

	    unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
	    unsigned long long totalsize = diskInfo.f_blocks * blocksize >> 20;
	    unsigned long long freesize  = diskInfo.f_bfree  * blocksize >> 20; //剩余空间的大小

		g_enviro_struct.t_sdtotal = totalsize;
		g_enviro_struct.t_sdfree  = freesize;
	if(freesize <= 20)
			return -1;
	if (freesize  <= dwReversedSize)
	{
		LOG("No Space For SD record !\n");
		return 1;
	}

	return 0;

}



int capacity_check(char *mount_point)
{
	struct statfs diskInfo; //系统stat的结构体
	static int check_cnt = 0;
	//int tmp_size = 0;
	while(1)
	{
		if(readonly_check())
			return -1;

		if (statfs(mount_point, &diskInfo) == -1){   //获取分区的状态
			LOG("stafs failed: %s\n",strerror(errno));
			LOG("statfs failed for path->[%s]\n", mount_point);
			return -1;
		}
		if((check_cnt++) > 10)
		{
			LOG("No Space For SD record !\n");
			check_cnt = 0;
			return -1;
		}

	    unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
	    unsigned long long totalsize = diskInfo.f_blocks * blocksize >> 20;
	    unsigned long long freesize  = diskInfo.f_bfree  * blocksize >> 20; //剩余空间的大小

		g_enviro_struct.t_sdtotal = totalsize;
		g_enviro_struct.t_sdfree  = freesize;
		
		if(freesize > 512/*g_enviro_struct.t_sdfree != 0 && g_enviro_struct.t_sdtotal / g_enviro_struct.t_sdfree < 10*/)
		{
			check_cnt = 0;
			break;
		}

	    LOG("Total_size = %llu MB , free_size = %llu MB \n",   totalsize , freesize);
		del_oldest_record(mount_point);
		
	}

	return 0;
}

int get_sd_path(char *path, int path_len)
{
	if((path == NULL) || (path_len < SD_PATH_LEN)) {
		pr_error("in %d\n", path_len);
		return -1;
	}
	FILE *fp = fopen("/tmp/sd_ok", "r");
	//if(!fp) return -1;
	CLEAR(path);
	int ret=fread(path, 1, SD_PATH_LEN, fp);
	path[ret-1]='\0';
	fclose(fp);
	return 0;
}
#define YSX_SD_RECORD_FREE_SPACE  1024
int SdRecord_Start(int keyframe, struct timeval tv, uint8_t *buf, int len)
{
	static char file[128]={'\0'};
	static char iframe_cnt = 0;
	static uint64_t lst_rcd_time = 0;
	struct mp4_Info *info = &fulltime_mp4;

	pthread_mutex_lock(&info->mutex);
	uint64_t current_time = tv.tv_sec;
	struct tm *diff_tm;  
    diff_tm = gmtime(&lst_rcd_time);  //减小等待I帧累计的时差
	
	/*not sync time*/
	if( access("/tmp/sync_time",X_OK) != 0 ){
		pthread_mutex_unlock(&info->mutex);
		return -1;
	}
	/*sd exist check*/
	if(access("/tmp/sd_ok",F_OK) == 0 ){
		if(sd_flag == 0 || access("/tmp/index_sync",F_OK) == 0){
			remove("/tmp/index_sync");
			get_sd_path(g_enviro_struct.t_sdpath, sizeof(g_enviro_struct.t_sdpath));
			if(info->oc)
				mp4_record_close(info,RECORD_CHANNEL_ONE);
			sync_record_index();
		}

		if((current_time%60 == 0) && sd_flag != 2){
			sd_flag  = 2;
		}
		if(sd_flag != 2)//
			sd_flag = 1;
	}
	else{
		sd_flag = 0;
		if(info->oc)
			mp4_record_close(info,RECORD_CHANNEL_ONE);
		pthread_mutex_unlock(&info->mutex);
		if(access(RECORD_INDEX,F_OK)==0)
			remove(RECORD_INDEX);
		return -1;
	}

	if(sd_flag == 2 && abs( current_time - lst_rcd_time ) > (RECORD_DURATION - diff_tm->tm_sec)){
		if(info->oc){
			iframe_cnt = 0;
			mp4_record_close(info,RECORD_CHANNEL_ONE);
			save_record_index(file);
			g_enviro_struct.record_status = YSX_RECORD_END;
		}
		if(!keyframe){
			pthread_mutex_unlock(&info->mutex);
			if((iframe_cnt%10) == 0){
				//QCamVideoInput_SetIFrame(g_enviro_struct.storage_resolution);
				#if 0
				if(g_enviro_struct.storage_resolution)
					QCamVideoInput_SetIFrame(MAIN_CHN);
				else
					QCamVideoInput_SetIFrame(SECOND_CHN);
				#endif
					
			}
			iframe_cnt ++;
//			LOG("Wait for key frame ..\n");
			return -1;
		}

		lst_rcd_time = current_time;


		if(capacity_check(g_enviro_struct.t_sdpath) == 0){
		#if 0
		if(readonly_check())
			return -1;
		if(vrec_storage_is_full(g_enviro_struct.t_sdpath, YSX_SD_RECORD_FREE_SPACE) == 1){
			 pthread_mutex_lock(&CheckerMutex);	
			 pthread_cond_signal(&CheckerCond);
			 pthread_mutex_unlock(&CheckerMutex);
		}else if(vrec_storage_is_full(g_enviro_struct.t_sdpath, YSX_SD_RECORD_FREE_SPACE) == -1){
		   
			return -1;
		}
		#endif	  
			memset(file,0,sizeof(file));
			gen_record_name(g_enviro_struct.t_sdpath,file);
			LOG("### new record ...%s\n",file);
			mp4_record_init(info,file,TYPE_SD_RECORD);
			g_enviro_struct.record_status = YSX_RECORD_ING;
		}

	}
	pthread_mutex_unlock(&info->mutex);

	return 0;
}

#ifdef YSX_CLOUD
extern 	void start_post_mdrecord();
int MdRecord_Start(struct timeval tv ,int newfile)
{
	static uint64_t lst_rcd_time = 0;
	struct mp4_Info *info = &md_mp4;
	static int md_flag = 0;

	pthread_mutex_lock(&info->mutex);

	uint64_t now = tv.tv_sec;

	/*not sync time*/
	if( access("/tmp/sync_time",X_OK) != 0 )
	{
		pthread_mutex_unlock(&info->mutex);
		return -1;
	}

	if(!md_flag && !info->oc )
	{
		if( newfile == 1)
		{
			md_flag = 1;	
			lst_rcd_time = now;
			LOG("### new md record ...%s\n",FILE_VIDEO);
			mp4_record_init(info,FILE_VIDEO,TYPE_CLOUD_RECORD);
		}else
		{	
			pthread_mutex_unlock(&info->mutex);
			return -1;
		}
	}
	
	if( md_flag && abs( now - lst_rcd_time ) > 5 )
	{
		LOG(".........md record time reach...\n");
		if(info->oc)
		{
			md_flag = 0;	

			mp4_record_close(info,RECORD_CHANNEL_TWO);
			start_post_mdrecord();
			
		}
	}
	pthread_mutex_unlock(&info->mutex);

	return 0;
}

int MdRecord_close()
{
	LOG("close mp4 record ...\n");
#if 0
	if(md_record_handle)
	{
//		mp4_record_close(md_record_handle,0);
        mp4_record_close(&md_mp4);    
		md_record_handle = NULL;
	}
	md_audio_tk = MP4_INVALID_TRACK_ID;
#endif 
    mp4_record_close(&md_mp4,RECORD_CHANNEL_TWO);    
    return 0;
}
#endif 

/****************DEMUXER*************************/
static AVFormatContext *ifmt_ctx = NULL ;
static int videoindex = -1 , audioindex=-1;
static uint32_t VDuration =0 ,ADuration = 0;
static uint32_t audio_dur = 0, video_dur = 0;
int record_demuxer_init(char *in_filename, unsigned char *video_codec_id)
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
		}else if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			audioindex=i;
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

	av_dump_format(ifmt_ctx, 0, in_filename, 0);
	*video_codec_id = ifmt_ctx->streams[videoindex]->codecpar->codec_id;
	
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
	}
	else if(frame_type == FRAME_VIDEO_TYPE && video_dur < VDuration)
	{
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
	}

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
//			printf("stream[%d] size=%06d pts=%09lld dts=%09lld  %06d isKeyFrame:%d time:%0.5g\n",
//					stream_index,
//					pkt -> size,
//					pkt -> pts,
//					pkt -> dts,
//					ifmt_ctx -> streams[stream_index]->r_frame_rate.num,
//					pkt -> flags,
//					av_q2d(*time_base) * pkt -> dts);
		}
	}
#endif 

	if (*delay > 1000) {
		*delay = 1000;
	}

	return result;
}


int get_record_frame2(AVPacket *pkt, int *delay , int frame_type)
{
	int ret = 0;
	av_init_packet(pkt);
	if(video_dur >= VDuration && audio_dur >= ADuration)
	{
		LOG("#######read over!\n");
		pkt->data = NULL;
		pkt->size = 0;
		*delay = 0 ;
		return FRAME_NONE_TYPE;
	}

	
	ret = av_read_frame(ifmt_ctx, pkt);
	if (ret < 0) {
		ret = FRAME_NONE_TYPE;
		goto exit;	
	}

	if (videoindex == pkt -> stream_index) {
		ret = FRAME_VIDEO_TYPE;
		frame_type = FRAME_VIDEO_TYPE;
		*delay = pkt->duration * 1000 / 90000 ;
		video_dur += pkt->duration;
	}
	else if (audioindex == pkt -> stream_index) {
		ret = FRAME_AUDIO_TYPE;
		frame_type = FRAME_AUDIO_TYPE;
		*delay = pkt->duration * 1000 / 16000;
		audio_dur += pkt->duration;
	}
	
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
//			printf("stream[%d] size=%06d pts=%09lld dts=%09lld  %06d isKeyFrame:%d time:%0.5g delay: %d\n",
//					stream_index,
//					pkt -> size,
//					pkt -> pts,
//					pkt -> dts,
//					ifmt_ctx -> streams[stream_index]->r_frame_rate.num,
//					pkt -> flags,
//					av_q2d(*time_base) * pkt -> dts,
//					*delay);
		}
	}
#endif 

exit:
	return ret;
}


void reset_record_frame(AVPacket *pkt)
{
	if(pkt){
		av_packet_unref(pkt);
	}
}

void record_demuxer_uninit()
{
	reset_record_frame(NULL);

	if(ifmt_ctx){
		avformat_close_input(&ifmt_ctx);
		ifmt_ctx = NULL;
	}
	return;
}

static md_record_Mutex;
struct timeval md_lst_vid = {0,0};
struct timeval md_lst_aud = {0,0};
static int md_err_cnt = 0;
static int md_record_started = 0;
int md_record_init(const char *filename)
{
	if(NULL == filename){
		LOG("file name is NULL !\n");
		return -1;
	}
	
    if (mp4_mdrecord_init(&md_mp4, filename) < 0) {
        return -1;
    }
	memset(&md_lst_vid,0,sizeof(struct timeval));
	memset(&md_lst_aud,0,sizeof(struct timeval));
    md_record_started = 1;
	return 0;
}

int md_write_video_frame(int keyframe, uint8_t *buf, int len, const struct timeval *tv)
{
	int ret = 0;
    if (md_record_started) {
        if (write_video_frame(&md_mp4, keyframe, buf, len, tv,RECORD_CHANNEL_TWO) < 0) {
            ret = -1;
        }
    }
    else {
        ret = -1;
    }

	return ret;
}

int md_write_audio_frame(uint8_t *buf, int len,const struct timeval *tv)
{
	int ret = 0;
    if (md_record_started) {
        if (write_audio_frame(&md_mp4, buf, len, tv,RECORD_CHANNEL_TWO) < 0) {
			LOG("write cloud audio failed\n");
            ret = -1;
        }
    }
    else {
        ret = -1;
    }

	return ret;
}

int md_record_close(struct mp4_Info *info)
{
	LOG("close mp4 record ...\n");
    if (md_record_started) {
		pthread_mutex_lock(&info->mutex);
        mp4_record_close(&md_mp4,RECORD_CHANNEL_TWO);
		pthread_mutex_unlock(&info->mutex);
        md_record_started = 0;
    }
	return 0;
}

static void record_lock_release()
{	
	pthread_mutex_destroy(&fulltime_mp4.mutex);	
#ifdef YSX_CLOUD		
	pthread_mutex_destroy(&md_mp4.mutex);
#endif
}

void Record_Uninit()
{
	SdRecord_Stop(&fulltime_mp4);	
#ifdef YSX_CLOUD
	md_record_close(&md_mp4);
#endif	

	record_lock_release();
	memset(&fulltime_mp4,0,sizeof(struct mp4_Info));
	fulltime_mp4.oc = NULL;	
#ifdef YSX_CLOUD	
	memset(&md_mp4,0,sizeof(struct mp4_Info));
	md_mp4.oc = NULL;
#endif

		#if 0
		if(bPause[0] == 1){
			 pthread_mutex_lock(&CheckerMutex);	
			 pthread_cond_signal(&CheckerCond);
			 pthread_mutex_unlock(&CheckerMutex);
			}
		#endif


	
	
	return ;
}
