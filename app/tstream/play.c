#include "include/common_env.h"
#include "include/common_func.h"
#include "ppcs_protocal.h"

#include <time.h>

#include "sdrecord.h"
#include "play.h"
#include "audio_codec.h"

void dump_time(STimeDay tmp)
{
    LOG( "%04d-%02d-%02d %02d:%02d:%02d\n",tmp.year,tmp.month,tmp.day,tmp.hour,tmp.minute,tmp.second);
}

time_t ysx_mktime(RIndex RI)
{
	struct tm tb;
    memset(&tb,0,sizeof(struct tm));
    tb.tm_year = RI.iy-1900;// = 1900+tb.tm_year;
    tb.tm_mon  = RI.im - 1;
    tb.tm_mday = RI.id;
    tb.tm_hour = RI.ih;
    tb.tm_min  = RI.imi;
    tb.tm_sec  = 0;    //60-rtotal[i].timelen;
    return mktime(&tb);
}

time_t TranslateTime(STimeDay sT)
{
    struct tm p;
    memset(&p,0,sizeof(struct tm));

    p.tm_year = sT.year-1900;
    p.tm_mon = sT.month-1;
    p.tm_mday = sT.day;
    p.tm_hour = sT.hour;
    p.tm_min = sT.minute;
    p.tm_sec = 0;//sT.second;

    return mktime(&p);
}

int record_demuxer_init(playback_info_t *pb_info )
{
	uint32_t numSamples;
	int i;

	pb_info->oMp4File = MP4Read(pb_info->pb_file);
	if (!pb_info->oMp4File)
	{
		LOG("Read MP4 %s error....\r\n",pb_info->pb_file); pb_info->oMp4File = NULL;
		return -1;
	}

	MP4TrackId trackId = MP4_INVALID_TRACK_ID;
	uint32_t numTracks = MP4GetNumberOfTracks(pb_info->oMp4File,NULL,0);
	LOG("numTracks:%d\n",numTracks);

	if(numTracks != 2)
	{
		MP4Close(pb_info->oMp4File,0);
		LOG("#@# illegal track numbers !\n");pb_info->oMp4File = NULL;
		return -1;
	}

	for (i = 0; i < numTracks; i++)
	{
		trackId = MP4FindTrackId(pb_info->oMp4File, i,NULL,0);
		const char* trackType = MP4GetTrackType(pb_info->oMp4File, trackId);
		if (MP4_IS_VIDEO_TRACK_TYPE(trackType))
		{
			LOG("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
			pb_info->VTrackId = trackId;

			MP4Duration duration = MP4GetTrackDuration(pb_info->oMp4File, trackId );
			numSamples = MP4GetTrackNumberOfSamples(pb_info->oMp4File, trackId);
			uint32_t timescale = MP4GetTrackTimeScale(pb_info->oMp4File, trackId);
			//oStreamDuration = duration/(timescale/1000);
			pb_info->VTotalNum = numSamples;
			LOG("video duration = %lld , numSamples = %d , timescale= %d\n",duration,numSamples,timescale);
		}
		else if (MP4_IS_AUDIO_TRACK_TYPE(trackType))
		{
			pb_info->ATrackId = trackId;
			LOG("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
			MP4Duration duration = MP4GetTrackDuration(pb_info->oMp4File, trackId );
			numSamples = MP4GetTrackNumberOfSamples(pb_info->oMp4File, trackId);
			uint32_t timescale = MP4GetTrackTimeScale(pb_info->oMp4File, trackId);
			pb_info->ATotalNum = numSamples;
			LOG("audio duration = %lld , numSamples = %d , timescale= %d\n",duration,numSamples,timescale);
			pb_info->samplerate = timescale;

		}
	}

	return 0;
}

int get_file_size(const char * file_name)
{
	int len;
	FILE *fp;
	if(file_name==NULL)
		return 0;
	fp=fopen(file_name,"rb");
	if(fp==NULL)
		return 0;
	fseek(fp,0L,SEEK_END);
	len=ftell(fp);
	fclose(fp);
	return len;
}

//泽宝要求统计所有 h264和aac的sample的大小
int get_mp4_file_size(const char * file_name)
{
    uint32_t numSamples;
    int i;
    int ASize_sum= 0,VSize_sum= 0;

    MP4FileHandle oMp4File = NULL;
    int VTrackId = -1,ATrackId = -1;
    int VTotalNum  =0, ATotalNums = 0;

    unsigned char *VData = NULL,*AData = NULL;
    unsigned int VSize = 0, ASize = 0;
    MP4Timestamp VStartTime = 0,AStartTime = 0;
    MP4Duration VDuration,ADuration ;
    MP4Duration VRenderingOffset,ARenderingOffset;
    bool VIsSyncSample = 0,AIsSyncSample =0;

    oMp4File = MP4Read(file_name);
    if (!oMp4File)
    {
        LOG("Read MP4 %s error....\r\n",file_name);
        oMp4File = NULL;
        return -1;
    }

    MP4TrackId trackId = MP4_INVALID_TRACK_ID;
    uint32_t numTracks = MP4GetNumberOfTracks(oMp4File,NULL,0);
    LOG("\nnumTracks:%d\n",numTracks);

    if(numTracks != 2)
    {
        MP4Close(oMp4File,0);
        LOG("#@# illegal track numbers !\n");
        oMp4File = NULL;
        return -1;
    }

    for (i = 0; i < numTracks; i++)
    {
        trackId = MP4FindTrackId(oMp4File, i,NULL,0);
        const char* trackType = MP4GetTrackType(oMp4File, trackId);
        // "vide"
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType))
        {
            LOG("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
            VTrackId = trackId;

            MP4Duration duration = MP4GetTrackDuration(oMp4File, trackId );
            numSamples = MP4GetTrackNumberOfSamples(oMp4File, trackId);
            uint32_t timescale = MP4GetTrackTimeScale(oMp4File, trackId);
            VTotalNum = numSamples;
            //LOG("video duration = %lld , numSamples = %d , timescale= %d\n",duration,numSamples,timescale);

        }
        else if (MP4_IS_AUDIO_TRACK_TYPE(trackType))
        {
            ATrackId = trackId;
            LOG("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
            MP4Duration duration = MP4GetTrackDuration(oMp4File, trackId );
            numSamples = MP4GetTrackNumberOfSamples(oMp4File, trackId);
            uint32_t timescale = MP4GetTrackTimeScale(oMp4File, trackId);
            ATotalNums = numSamples;
            //LOG("audio duration = %lld , numSamples = %d , timescale= %d\n",duration,numSamples,timescale);
        }
    }
    for (i = 1; i <= VTotalNum; i++)
    {
        MP4ReadSample(oMp4File,VTrackId,i,&VData,&VSize,&VStartTime,&VDuration,&VRenderingOffset,&VIsSyncSample);
        VSize_sum += VSize;
		//printf("V[%3d]:%08d\n",i,VSize);
		if(VData)
        	free(VData);
        VData = NULL;
    }

    for (i = 1; i <= ATotalNums; i++)
    {
        MP4ReadSample(oMp4File,ATrackId,i,&AData,&ASize,&AStartTime,&ADuration,&ARenderingOffset,&AIsSyncSample);
        ASize_sum += ASize;
		//printf("A[%3d]:%08d\n",i,ASize);
		if(AData)
        	free(AData);
        AData = NULL;
    }

    MP4Close(oMp4File,0);

    return (ASize_sum+VSize_sum);
}


int get_mp4_duration(char *in_filename)
{
	MP4FileHandle mp4file;
	uint32_t numSamples;
	int i;
	
	mp4file = MP4Read(in_filename);
	if (!mp4file)
	{
		LOG("Read MP4 %s error....\r\n",in_filename);mp4file = NULL;
		return -1;
	}

	MP4TrackId trackId = MP4_INVALID_TRACK_ID;
	uint32_t numTracks = MP4GetNumberOfTracks(mp4file,NULL,0);
	//LOG("numTracks:%d\n",numTracks);

	if(numTracks != 2)
	{
		MP4Close(mp4file,0);
		LOG("#@# illegal track numbers !\n");mp4file = NULL;
		return -1;
	}

	for (i = 0; i < numTracks; i++)
	{
		trackId = MP4FindTrackId(mp4file, i,NULL,0);
		const char* trackType = MP4GetTrackType(mp4file, trackId);
		if (MP4_IS_VIDEO_TRACK_TYPE(trackType))
		{
			//LOG("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
			MP4Duration duration = MP4GetTrackDuration(mp4file, trackId );
			//LOG("duration = %lld\n",duration);
			return (int)duration;
		}
	}	

	return -1;
	
}

int get_record_frame(playback_info_t *pb_info, MY_AVPacket *pkt , int *delay , int frame_type)
{
	if(!pb_info->oMp4File)
	{
		printf("pb_info->oMp4File is NULL\n");
		return FRAME_NONE_TYPE;
	}
	int result = -1;
	unsigned char *VData = NULL , *AData = NULL;
	unsigned int VSize = 0, ASize = 0;
	MP4Timestamp VStartTime = 0,AStartTime = 0;
	MP4Duration VDuration,ADuration ;
	MP4Duration VRenderingOffset,ARenderingOffset;
	bool VIsSyncSample = 0 ,AIsSyncSample =0;

	memset(pkt,0,sizeof(MY_AVPacket));

	if( (frame_type == FRAME_VIDEO_TYPE && pb_info->video_index > pb_info->VTotalNum) 
		|| (frame_type == FRAME_AUDIO_TYPE && pb_info->audio_index > pb_info->ATotalNum))
	{
		printf("%d @@@@@ %d %d %d %d \n",frame_type,pb_info->video_index,pb_info->VTotalNum,pb_info->audio_index,pb_info->ATotalNum);
		result = FRAME_NONE_TYPE;
		pkt->data = NULL;
		pkt->size = 0;
		*delay = 0 ;
		goto done;
	}

	//audio track
	if(frame_type == FRAME_AUDIO_TYPE && pb_info->audio_index <= pb_info->ATotalNum)
	{
		MP4ReadSample(pb_info->oMp4File,pb_info->ATrackId,pb_info->audio_index,&AData,&ASize,&AStartTime,&ADuration,&ARenderingOffset,&AIsSyncSample);
		//		printf("audio : StartTime = %lld ,pDuration = %lld pIsSyncSample = %d\n",AStartTime,ADuration,AIsSyncSample);
		result = FRAME_AUDIO_TYPE;
		pb_info->audio_index ++ ;
		pkt->data = AData;
		pkt->size = ASize;
		*delay = ADuration * 1000 / pb_info->samplerate;
		pkt->starttime = AStartTime;
		//printf("==A[%3d]:%08d\n",pb_info->audio_index-1,ASize);
		//printf("audio : %d %d StartTime = %lld ,pDuration = %lld delay = %d\n",pb_info->audio_index,ASize,AStartTime,ADuration,*delay);
	}
	//video track
	else if(frame_type == FRAME_VIDEO_TYPE && pb_info->video_index <= pb_info->VTotalNum)
	{
		MP4ReadSample(pb_info->oMp4File,pb_info->VTrackId,pb_info->video_index,&VData,&VSize,&VStartTime,&VDuration,&VRenderingOffset,&VIsSyncSample);
		//printf("video :%d StartTime = %lld ,pDuration = %lld pIsSyncSample = %d\n",VSize, VStartTime,VDuration,VIsSyncSample);

		//printf("VData %02X %02X %02X %02X %02X\n",VData[0],VData[1],VData[2],VData[3],VData[4]);
		
		if(VData && VSize > 4)
		{
			//标准的264帧，前面几个字节就是frame的长度.
			//需要替换为标准的264 nal 头.
			VData[0] = 0x00;
			VData[1] = 0x00;
			VData[2] = 0x00;
			VData[3] = 0x01;
		}

		//H264常见的 帧类型 用0x1F掩码
		/*
			0x67, 0x47, 0x27
			0x68, 0x48, 0x28
			0x65, 0x45, 0x25
			0x61, 0x41, 0x21
		*/
		//if( VData[4] != 0x41 )	// I frame
		if( (VData[4] & 0x1F) != 1 )
		{
			LOG("###I frame### %d\n",pb_info->video_index);
			pkt->flags = 1;
		}else
			pkt->flags = 0;
		result = FRAME_VIDEO_TYPE;
		pb_info->video_index ++ ;
		pkt->data = VData;
		pkt->size = VSize;
		pkt->starttime = AStartTime;
		//printf("==V[%3d]:%08d\n",pb_info->video_index-1,VSize);
		*delay = VDuration * 1000 / 90000 ;
		//		printf("video :type:0x%0x StartTime = %lld ,pDuration = %lld  delay =%d\n",VData[4],VStartTime,VDuration,*delay);
	}

	if(*delay > 1000)
		*delay = 1000;
done:

	return result;
}


void reset_record_frame(MY_AVPacket *pkt)
{
	if(pkt && pkt->data)
	{
		//		mem_free(pkt->data);
		// 这里释放的内存是MP4V2里面申请的内存
		free(pkt -> data);
	}
	return;
}

void record_demuxer_uninit(playback_info_t *pb_info)
{
	if(pb_info->oMp4File);
	{
		pb_info->video_index = 1;
		pb_info->audio_index = 1;
		MP4Close(pb_info->oMp4File,0);
		pb_info->oMp4File = NULL;
	}
	return;
}

int play_ctrl(playback_info_t *pb_info,record_play_ctrl_t rec_play)
{
	pthread_mutex_lock(&pb_info->pb_mutex);
	switch(rec_play.ctrl)
	{
		case VAVA_REPLAY_CTRL_START:
			pb_info->pb_status == PLAY_START;
		break;
		case VAVA_REPLAY_CTRL_PAUSE:
			pb_info->pb_status == PLAY_PAUSE;
		break;
		case VAVA_REPLAY_CTRL_STOP:
			pb_info->pb_status == PLAY_STOP;
		break;
		case VAVA_REPLAY_CTRL_CONTINUE:
			pb_info->pb_status == PLAY_START;
		break;
		case VAVA_REPLAY_CTRL_FAST_FOWRD:
			pb_info->pb_status == PLAY_START;//暂不支持
		break;
		case VAVA_REPLAY_CTRL_FAST_BACKWARD:
			pb_info->pb_status == PLAY_START;//暂不支持
		break;		
	}
	pthread_mutex_unlock(&pb_info->pb_mutex);
	return 0;
}

void *thread_sd_video_playback(void *arg)
{
    
	int stream_type;
	MY_AVPacket pkt;
	int delay = 0;
	int check_buf_delay = 0;

	playback_info_t  *pb_info = (playback_info_t *)arg;
	LOG("thread_sd_video_playback %d\n",pb_info->pb_status);
	
	while( pb_info->_loop )
	{
		pthread_mutex_lock(&pb_info->pb_mutex);
		if( access("/tmp/sd_ok",F_OK)!=0 //sd卡不存在
			||pb_info->pb_status == PLAY_STOP 
			||pb_info->pb_status == PLAY_PAUSE
			||pb_info->pb_status == PLAY_VIDEO_OVER
			||pb_info->pb_status == PLAY_OVER
			||pb_info->pb_status == PLAY_CHANGE)
		{
			pthread_mutex_unlock(&pb_info->pb_mutex);
			usleep(30*1000);
			continue;
		}

		if(ppcs_service_check_buffer(pb_info->SID,2)<0) //ppcs缓存不够
		{
			if(check_buf_delay<20)//最大延时 2S
				check_buf_delay++;
			pthread_mutex_unlock(&pb_info->pb_mutex);
			usleep(check_buf_delay*100*1000);
			continue;
		}
		check_buf_delay=0;

		stream_type = get_record_frame(pb_info, &pkt,&delay,FRAME_VIDEO_TYPE);
		if(stream_type == FRAME_NONE_TYPE ){ //在audio的线程里面设置PLAY_OVER
			LOG("    ####### PLAY_OVER \n");
			
			if(pb_info->pb_status == PLAY_AUDIO_OVER)//两个都结束 才是全部结束
			{
				pb_info->pb_status = PLAY_OVER;
				record_demuxer_uninit(pb_info);
				LOG("    ####### PLAY_OVER %d\n",pb_info->totol_byte);
			}
			else
			{
				pb_info->pb_status = PLAY_VIDEO_OVER;
			}
			pthread_mutex_unlock(&pb_info->pb_mutex);
			usleep(1000*30);
			continue;
		}
		if(stream_type == FRAME_VIDEO_TYPE){
			int ret;
			struct timeval tv;
			if(pb_info->psf.type == VAVA_RECFILE_TRANSPORT_NORMA)
				tv.tv_sec = pkt.starttime / 90000+pb_info->pb_tv.tv_sec;
			else
				tv.tv_sec = pkt.starttime / 90000;
			tv.tv_usec = ((pkt.starttime * 1000 / 90)%1000)*1000;

			//if(pb_info->video_framenum==0)
			//{
			//	ppcs_proto_send_meta_video_frame(VAVA_RECORD_VIDEO_TAG,2,pb_info->SID);//发送空帧
			//	LOG("send empty frame\n");
			//}
							
			//pb_info->video_framenum++;
				
			if(pkt.flags)
			{
				pb_info->wait_keyframe = 0;
			}
			if(pb_info->wait_keyframe == 0)//已经等到key frame
			{
				char *swap_data = NULL;
				swap_data = malloc(pkt.size+sizeof(VAVA_MSG_HEADER_AV_STREAM));
				if(swap_data==NULL)
				{
					LOG("OOPS NEVER ~~~\n");
					usleep(100*1000);
					continue;
				}
				memcpy(&swap_data[sizeof(VAVA_MSG_HEADER_AV_STREAM)],pkt.data,pkt.size);
				pb_info->totol_byte += pkt.size;
				ret = ppcs_proto_send_video(VAVA_RECORD_VIDEO_TAG, 2, pb_info->SID, swap_data, pkt.size, tv, pkt.flags ,pb_info->video_framenum);
				free(swap_data);
				pb_info->video_framenum++;
			}
			if(ret != 0)//没有往ppcs发送的帧序已经不连续了
			{
				pb_info->wait_keyframe = 1;
			}
		}
		
		reset_record_frame(&pkt);//释放内存
		pthread_mutex_unlock(&pb_info->pb_mutex);
		if(pb_info->psf.type == VAVA_RECFILE_TRANSPORT_NORMA)//下载模式不延时
			usleep(delay*1000); // 0 0 1 ;两个个video frame , 1个audio frame
		else
			usleep(delay*10);			
	}
 
    LOG("video thread play exit\n");
    pthread_exit((void *)0);
}

void *thread_sd_audio_playback(void *arg)
{
	

	int delay;
	MY_AVPacket pkt;
	int stream_type;
    int pb_samplerate;
	int check_buf_delay;

	playback_info_t  *pb_info = (playback_info_t *)arg;
	LOG("thread_sd_audio_playback %d\n",pb_info->pb_status);
	
	while( pb_info->_loop )
	{
		pthread_mutex_lock(&pb_info->pb_mutex);
		if( access("/tmp/sd_ok",F_OK) != 0 //sd卡不存在
			||pb_info->pb_status == PLAY_STOP 
			||pb_info->pb_status == PLAY_PAUSE
			||pb_info->pb_status == PLAY_AUDIO_OVER
			||pb_info->pb_status == PLAY_OVER )
		{
			pthread_mutex_unlock(&pb_info->pb_mutex);
			usleep(30*1000);
			continue;
		}
		else if(pb_info->pb_status == PLAY_CHANGE)//播放内容发生变化
		{
			record_demuxer_uninit(pb_info);
			
            if(record_demuxer_init(pb_info) < 0){
                LOG("audio record_demuxer_init error    !\n");
                pthread_mutex_unlock(&pb_info->pb_mutex);
                break;
            }
			pb_info->pb_status = PLAY_START;
			pthread_mutex_unlock(&pb_info->pb_mutex);
			continue;
		}else{}

		if(ppcs_service_check_buffer(pb_info->SID,2)<0) //ppcs缓存不够
		{
			if(check_buf_delay<20)//最大延时 2S
				check_buf_delay++;
			pthread_mutex_unlock(&pb_info->pb_mutex);
			usleep(check_buf_delay*100*1000);
			continue;
		}
		check_buf_delay=0;


		if(pb_info->video_framenum <= 0)  //空帧没有发送
		{
			pthread_mutex_unlock(&pb_info->pb_mutex);
			continue;
		}
		

		stream_type = get_record_frame(pb_info, &pkt,&delay,FRAME_AUDIO_TYPE);
		if(stream_type == FRAME_NONE_TYPE ) {  // 在audio的线程里面设置PLAY_OVER
			
			if(pb_info->pb_status == PLAY_VIDEO_OVER)//两个都结束 才是全部结束
			{
				pb_info->pb_status = PLAY_OVER;
				record_demuxer_uninit(pb_info);
				LOG("    ####### PLAY_OVER %d\n",pb_info->totol_byte);
			}
			else
			{
				pb_info->pb_status = PLAY_AUDIO_OVER;
			}
			pthread_mutex_unlock(&pb_info->pb_mutex);
			usleep(1000*30);
			continue;
		}
		if(stream_type == FRAME_AUDIO_TYPE) {
			struct timeval tv;
			if(pb_info->psf.type == VAVA_RECFILE_TRANSPORT_NORMA)
				tv.tv_sec = pkt.starttime / pb_info->samplerate + pb_info->pb_tv.tv_sec;
			else
				tv.tv_sec = pkt.starttime / pb_info->samplerate ;
			tv.tv_usec = ( (pkt.starttime * 1000 / pb_info->samplerate)%1000 )*1000; 

			char *swap_data = NULL;		
			swap_data = malloc(pkt.size+sizeof(VAVA_MSG_HEADER_AV_STREAM));
			if(swap_data==NULL)
			{
				LOG("OOPS NEVER ~~~\n");
				usleep(100*1000);
				continue;
			}
			memcpy(&swap_data[sizeof(VAVA_MSG_HEADER_AV_STREAM)],pkt.data,pkt.size);
			pb_info->totol_byte += pkt.size;
			ppcs_proto_send_audio(VAVA_RECORD_AUDIO_TAG,2,pb_info->SID, swap_data, pkt.size, tv, pb_info->audio_framenum++);
			free(swap_data);			
		}
		
		reset_record_frame(&pkt);//释放内存
		pthread_mutex_unlock(&pb_info->pb_mutex);
		if(pb_info->psf.type == VAVA_RECFILE_TRANSPORT_NORMA)//下载模式不延时
			usleep(delay*1000); // 0 0 1 ;两个个video frame , 1个audio frame
		else
			usleep(delay*10);
	}

	stop_sd_playback(pb_info);
	
	LOG("audio thread play exit\n");
    pthread_exit((void *)0);
}


void *thread_sd_download_playback(void *arg)
{
	pthread_exit((void *)0);
}

playback_info_t *get_pb_info(int iGNo)
{
	return &g_media_struct.g_media_info[iGNo].pb_info;
}

//把 YYYYMMDD  HHMMSS_type 转成 tv, 这个没有usec，就直接 time_t就可以了
time_t get_tv_from_filename(char * dirname, char *filename)
{
	char year_str[5]={0};
	char mm_str[3]={0};
	char dd_str[3]={0};
	char hh_str[3]={0};
	char min_str[3]={0};
	char ss_str[3]={0};
	
	struct tm tm_;
	int year, month, day, hour, minute,second;

	year_str[0]=dirname[0];year_str[1]=dirname[1];year_str[2]=dirname[2];year_str[3]=dirname[3];year_str[4]=0;
	mm_str[0]=dirname[4];mm_str[1]=dirname[5];mm_str[2]=0;
	dd_str[0]=dirname[6];dd_str[1]=dirname[7];dd_str[2]=0;
	hh_str[0]=filename[0];hh_str[1]=filename[1];hh_str[2]=0;
	min_str[0]=filename[2];min_str[1]=filename[3];min_str[2]=0;
	ss_str[0]=filename[4];ss_str[1]=filename[5];ss_str[2]=0;
	year = atoi(year_str);
	month = atoi(mm_str);
	day = atoi(dd_str);
	hour = atoi(hh_str);
	minute = atoi(min_str);
	second = atoi(ss_str);
	
	tm_.tm_year  = year-1900;  
	tm_.tm_mon	 = month-1;  
	tm_.tm_mday  = day;  
	tm_.tm_hour  = hour;  
	tm_.tm_min	 = minute;	
	tm_.tm_sec	 = second;	
	tm_.tm_isdst = 0;  
  
	time_t t_ = mktime(&tm_); //已经减了8个时区	
	return t_; //秒时间	
}


int start_sd_playback(int iGNo)
{
	int ret;
	
	playback_info_t *pb_info = get_pb_info(iGNo);

	LOG("start_sd_playback \n");

	// 如果已经存在了 则走 PLAY_CHANGE 这种做法架构边的复杂 但是playback av线程可以一直存在
	if(pb_info->SID >= 0)
	{
		pthread_mutex_lock(&pb_info->pb_mutex);
		pb_info->pb_status = PLAY_CHANGE;
		memset(pb_info->pb_file, 0, sizeof(pb_info->pb_file));
		
		if(pb_info->pb_mode == 0)//时间段 回放模式
		{
			LOG("pb by time\n");
		}
		else if(pb_info->pb_mode == 1)//单个文件 回放模式
		{
			char hour[3]={0};
			hour[0]=pb_info->psf.filename[0];hour[1]=pb_info->psf.filename[1];hour[2]=0;
			snprintf(pb_info->pb_file,sizeof(pb_info->pb_file)-1,
				"%s/VAVACAM/%s/%02s/%s.mp4",g_enviro_struct.t_sdpath,pb_info->psf.dirname,hour,pb_info->psf.filename);
			pb_info->pb_tv.tv_sec = get_tv_from_filename(pb_info->psf.dirname,pb_info->psf.filename);
			LOG("pb by single file %s\n", pb_info->pb_file);
		}
		else
		{
			LOG("unkown mode\n");
			pthread_mutex_unlock(&pb_info->pb_mutex);
			return VAVA_ERR_CODE_RECPLAY_FAIL; 
		}
		//pb_info->VTrackId = -1;
		//pb_info->ATrackId = -1;
		//pb_info->VTotalNum = 0;
		//pb_info->ATotalNum = 0;
		pb_info->video_index = 1;
		pb_info->audio_index = 1;
		pb_info->audio_framenum = 0;
		pb_info->video_framenum = 0;
		pb_info->wait_keyframe = 1;
		pb_info->totol_byte = 0;

		pb_info->token = random()%1000000;
		pb_info->file_size = get_mp4_file_size(pb_info->pb_file);		
		
		pthread_mutex_unlock(&pb_info->pb_mutex);
		return VAVA_ERR_CODE_SUCCESS;
	}

	pb_info->SID = g_media_struct.g_media_info[iGNo].sessionid;
	
	
	if(pb_info->_loop)
	{
		LOG("pb NEVER HAPPEN\n");
		return VAVA_ERR_CODE_RECPLAY_FAIL;
	}

	//回放模式
	if(pb_info->pb_mode == 0)//时间段 回放模式
	{
		LOG("pb by time\n");
	}
	else if(pb_info->pb_mode == 1)//单个文件 回放模式
	{
		char hour[3]={0};
		hour[0]=pb_info->psf.filename[0];hour[1]=pb_info->psf.filename[1];hour[2]=0;	
		snprintf(pb_info->pb_file,sizeof(pb_info->pb_file)-1,
			"%s/VAVACAM/%s/%02s/%s.mp4",g_enviro_struct.t_sdpath,pb_info->psf.dirname,hour,pb_info->psf.filename);
		pb_info->pb_tv.tv_sec = get_tv_from_filename(pb_info->psf.dirname,pb_info->psf.filename);
		LOG("pb by single file %s\n", pb_info->pb_file);
	}
	else
	{
		LOG("unkown mode\n");
		return VAVA_ERR_CODE_RECPLAY_FAIL; 
	}

	if(access(pb_info->pb_file,F_OK)!=0)
	{
		LOG("file not exist\n");
		return VAVA_ERR_CODE_RECFILE_NOTFOUND;
	}
	
	pthread_mutex_lock(&pb_info->pb_mutex);
	//对第一个文件进行解码
    if(record_demuxer_init(pb_info) < 0){
        LOG("record_demuxer_init error  !\n");
		pthread_mutex_unlock(&pb_info->pb_mutex);
        return VAVA_ERR_CODE_RECPLAY_FAIL;
    }

	pb_info->pb_status = PLAY_STOP;
	LOG("       ######## PLAY_STOP\n");
	pb_info->_loop = 1;//设置 播放进程为RUNNING
	
	//pb_info->VTrackId = -1;
	//pb_info->ATrackId = -1;
	//pb_info->VTotalNum = 0;
	//pb_info->ATotalNum = 0;
	pb_info->video_index = 1;
	pb_info->audio_index = 1;
	pb_info->audio_framenum = 0;
	pb_info->video_framenum = 0;
	pb_info->wait_keyframe = 1;
	pb_info->totol_byte = 0;

	pthread_mutex_unlock(&pb_info->pb_mutex);

	//启动音视频回放线程
	pthread_t thread_video;
	if((ret = pthread_create(&thread_video, NULL, &thread_sd_video_playback, pb_info)) < 0) {
		LOG("pthread_create ret=%d\n", ret);
		pb_info->_loop = 0;//回收线程资源
		return VAVA_ERR_CODE_RECPLAY_FAIL;
	}
	pthread_detach(thread_video);
	LOG("       ######## thread_sd_video_playback\n");

	pthread_t thread_audio;
	if((ret = pthread_create(&thread_audio, NULL, &thread_sd_audio_playback, pb_info)) < 0) {
		LOG("pthread_create ret=%d\n", ret);
		pb_info->_loop = 0;//回收线程资源
		return VAVA_ERR_CODE_RECPLAY_FAIL;
	}
	pthread_detach(thread_audio);
	LOG("       ######## thread_sd_audio_playback\n");
	
	pthread_t thread_download;
	if((ret = pthread_create(&thread_download, NULL, &thread_sd_download_playback, pb_info)) < 0) {
		LOG("pthread_create ret=%d\n", ret);
		pb_info->_loop = 0;//回收线程资源
		return VAVA_ERR_CODE_RECPLAY_FAIL;
	}
	pthread_detach(thread_download);
	LOG("       ######## thread_sd_download_playback\n");

	pb_info->token = random()%1000000;
	pb_info->file_size = get_mp4_file_size(pb_info->pb_file);

	LOG("       ######## PLAY_START file size %d\n",pb_info->file_size);
	pb_info->pb_status = PLAY_START;
	
	
	return VAVA_ERR_CODE_SUCCESS;
}

int stop_sd_playback(playback_info_t *pb_info)
{
	pb_info->_loop = 0;//设置 播放进程为STOP退出
	pb_info->SID = -1;
	//pthread_join is better
	LOG("sid %d pb stop\n",pb_info->SID);
	return 0;
}

