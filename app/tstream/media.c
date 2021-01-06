
#include "common_env.h"
#include "common_func.h"

#include "sdrecord.h"


#include "audio_codec.h"

#include "media.h"
#include "ysx_data_buf.h"
#include "rtc_test.h"
#include <fcntl.h> 
#include "PPCS_API.h"
#include "isvpush.h"
#include "vava_header.h"
#include "ppcs_protocal.h"
#include "qcam_audio_output.h"


extern G_StructRecordInfo g_recordinfo_struct[2];
struct adpcm_state enc_state,dec_state;

//缓存锁
static pthread_mutex_t list_lock_vid ;      // 直播流缓存 
static pthread_mutex_t list_lock_aud ;      // 音频流
static pthread_mutex_t list_lock_vrec_vid ;
static pthread_mutex_t list_lock_vrec_aud ;
static pthread_mutex_t list_lock_md_vid ;
static pthread_mutex_t list_lock_md_aud ;



typedef struct _SpeakerStruct
{
        int sid;
        int iIndex;
}SpeakerStruct;

static int pcm_buf_size , aac_buf_size = 0;
static uint8_t aac_buf[768] = {0}, pcm_buffer[2048] = {0};

static int md_write_video_status = 0;
static int md_write_audio_status = 0;
long long start_timestamp = 0;
static int switch_vquality = 1;
static int audio_cloud_num = 0;


int MediaErrHandling(int ret);
int get_online_num(void);
void *thread_ReceiveAudio(void *arg);


#if  (defined AD_BITRATE)
#define BITRATE_RANK				0x3
#define BITRATE_MIN					0x200
#define BITRATE_STEP				((MAIN_BITRATE-BITRATE_MIN)/BITRATE_RANK)
#define BITRATE_SWITCH_DELAY		0x1388

static int ismaster = -1;
static int avIndex_master = -1;

#ifdef S301
#define MAIN_LEVEL 					3
#define SECOND_LEVEL				1
static int main_bitrate[MAIN_LEVEL] = {1024, 768, 512};
static int second_bitrate[SECOND_LEVEL] = {128};
#else
#define MAIN_LEVEL 					3
#define SECOND_LEVEL				3
#define THIRD_LEVEL					3
static int main_bitrate[MAIN_LEVEL] = {1024*1.5, 1024, 768};
static int second_bitrate[SECOND_LEVEL] = {768, 512, 256};
static int third_bitrate[THIRD_LEVEL] = {512, 256, 128};
#endif


static int Set_Bitrate_Up(void)
{
        g_media_struct.bitrate_level = 0;

        QCamVideoInput_SetBitrate(MAIN_CHN, main_bitrate[g_media_struct.bitrate_level], 0);
        QCamVideoInput_SetBitrate(SECOND_CHN, second_bitrate[g_media_struct.bitrate_level], 0);
        //QCamVideoInput_SetBitrate(THIRD_CHN, third_bitrate[g_media_struct.bitrate_level], 0);

        LOG("Set Main-Bitrate %d\n", main_bitrate[g_media_struct.bitrate_level]);
        LOG("Set Second-Bitrate %d\n", second_bitrate[g_media_struct.bitrate_level]);
        //LOG("Set Third-Bitrate %d\n", third_bitrate[g_media_struct.bitrate_level]);
        return 0;
}

static int Set_Resolution_Down(int chn)
{
        int ret = 0;
#if 0
        if (chn == SECOND_CHN) {
                switch_vquality = 1;
                SetYSXCtl(YSX_VQUALITY, 4);	
                g_media_struct.bitrate_level = 0;
                pthread_mutex_lock(&video_live_write);
                stop_buffer_data(video_live_buffer);
                clean_buffer_data(video_live_buffer);
                start_buffer_data(video_live_buffer);
                pthread_mutex_unlock(&video_live_write);			
        }
        else if(chn == THIRD_CHN){
                switch_vquality = 1;
                SetYSXCtl(YSX_VQUALITY, 5);	
                g_media_struct.bitrate_level = 0;
                pthread_mutex_lock(&video_live_write);
                stop_buffer_data(video_live_buffer);
                clean_buffer_data(video_live_buffer);
                start_buffer_data(video_live_buffer);
                pthread_mutex_unlock(&video_live_write);	
        }else
                return -1;
#endif
        if (chn == SECOND_CHN) {
                switch_vquality = 1;
                SetYSXCtl(YSX_VQUALITY, 5);	
                g_media_struct.bitrate_level = 0;

        }

        else
                return -1;
        return ret;
}

static int Get_Cur_Bitrate(int chn)
{
#if 0
        if(chn == MAIN_CHN)
                return main_bitrate[g_media_struct.bitrate_level];
        else if(chn == SECOND_CHN)
                return second_bitrate[g_media_struct.bitrate_level];
        else if(chn == THIRD_CHN)
                return third_bitrate[g_media_struct.bitrate_level];
#endif
        printf("now  bitrate :%d %d %d\n",main_bitrate[g_media_struct.bitrate_level],second_bitrate[g_media_struct.bitrate_level],g_media_struct.bitrate_level);
        if(chn == MAIN_CHN)
                return main_bitrate[g_media_struct.bitrate_level];
        else if(chn == SECOND_CHN)
                return second_bitrate[g_media_struct.bitrate_level];
}

static int Set_Bitrate_Down(int chn)
{	
#if 0
        int i_ret = 0;

        if(chn == MAIN_CHN){
                g_media_struct.bitrate_level ++;
                if(g_media_struct.bitrate_level >= MAIN_LEVEL-1)
                        g_media_struct.bitrate_level = MAIN_LEVEL-1;

                LOG("Set Main-Bitrate %d\n", main_bitrate[g_media_struct.bitrate_level]);
                i_ret = QCamVideoInput_SetBitrate(MAIN_CHN, main_bitrate[g_media_struct.bitrate_level], 0);
                return i_ret;	
        }
        else if(chn == SECOND_CHN){
                g_media_struct.bitrate_level ++;
                if(g_media_struct.bitrate_level >= SECOND_LEVEL-1)
                        g_media_struct.bitrate_level = SECOND_LEVEL-1;

                LOG("Set Second-Bitrate %d\n", second_bitrate[g_media_struct.bitrate_level]);
                i_ret = QCamVideoInput_SetBitrate(SECOND_CHN, second_bitrate[g_media_struct.bitrate_level], 0);
                return i_ret;	
        }
        else if(chn == THIRD_CHN){
                g_media_struct.bitrate_level ++;
                if(g_media_struct.bitrate_level >= THIRD_LEVEL-1)
                        g_media_struct.bitrate_level = THIRD_LEVEL-1;

                LOG("Set Third-Bitrate %d\n", third_bitrate[g_media_struct.bitrate_level]);
                i_ret = QCamVideoInput_SetBitrate(THIRD_CHN, third_bitrate[g_media_struct.bitrate_level], 0);
                return i_ret;	
        }	
#endif

        int i_ret = 0;

        if(chn == MAIN_CHN){
                g_media_struct.bitrate_level ++;
                if(g_media_struct.bitrate_level >= MAIN_LEVEL-1)
                        g_media_struct.bitrate_level = MAIN_LEVEL-1;

                LOG("Set Main-Bitrate %d\n", main_bitrate[g_media_struct.bitrate_level]);
                i_ret = QCamVideoInput_SetBitrate(MAIN_CHN, main_bitrate[g_media_struct.bitrate_level], 0);
                return i_ret;	
        }
        else if(chn == SECOND_CHN){
                g_media_struct.bitrate_level ++;
                if(g_media_struct.bitrate_level >= SECOND_LEVEL-1)
                        g_media_struct.bitrate_level = SECOND_LEVEL-1;

                LOG("Set Second-Bitrate %d\n", second_bitrate[g_media_struct.bitrate_level]);
                i_ret = QCamVideoInput_SetBitrate(SECOND_CHN, second_bitrate[g_media_struct.bitrate_level], 0);
                return i_ret;	
        }



        return i_ret;
}

static int Bitrate_Adaptive()
{
        int i_ret = -1;
#if 0
        if(GetYSXCtl(YSX_VQUALITY) < 3){
                if(Get_Cur_Bitrate(MAIN_CHN) == main_bitrate[MAIN_LEVEL-1])
                        i_ret = Set_Resolution_Down(SECOND_CHN);
                else
                        i_ret = Set_Bitrate_Down(MAIN_CHN);
                return i_ret;
        }
        else if(GetYSXCtl(YSX_VQUALITY) == 4){
                if(Get_Cur_Bitrate(SECOND_CHN) == second_bitrate[SECOND_LEVEL-1])
                        i_ret = Set_Resolution_Down(THIRD_CHN);
                else
                        i_ret = Set_Bitrate_Down(SECOND_CHN);
                return i_ret;	
        }
        else if(GetYSXCtl(YSX_VQUALITY) > 4){
                if(Get_Cur_Bitrate(THIRD_CHN) != third_bitrate[THIRD_LEVEL-1])
                        i_ret = Set_Bitrate_Down(THIRD_CHN);
                return i_ret;
        }
#endif
        if(GetYSXCtl(YSX_VQUALITY) < 3){
                if(Get_Cur_Bitrate(MAIN_CHN) == main_bitrate[MAIN_LEVEL-1])
                        i_ret = Set_Resolution_Down(SECOND_CHN);
                else
                        i_ret = Set_Bitrate_Down(MAIN_CHN);
                return i_ret;
        }else if(GetYSXCtl(YSX_VQUALITY) > 3){
                printf("vga +++++++++++++++++++++++++++++++++++change \n");
                if(Get_Cur_Bitrate(SECOND_CHN) != second_bitrate[SECOND_LEVEL-1])
                        i_ret = Set_Bitrate_Down(SECOND_CHN);
                return i_ret;
        }
        return i_ret;
}

static int AdjustBitRate(int index, unsigned int time, int framesize, int err_code)
{
        static unsigned int iLastAdjustTime=0;
        static unsigned int iDownTime = 0;
        int i_ret = -1;
        float fusage = 0;

        if ((time-iLastAdjustTime) > BITRATE_SWITCH_DELAY){
                fusage = avResendBufUsageRate(index);
                iLastAdjustTime = time;
                if(err_code == 2) {
                        i_ret = Bitrate_Adaptive();
                        return i_ret;
                }
                if ((time - iDownTime) > BITRATE_SWITCH_DELAY){
                        iDownTime = time;
                        if (fusage > 0.6){
                                i_ret = Bitrate_Adaptive();
                                return i_ret;
                        }	
                }
        }
        return i_ret;
}

void set_master_flag(int enable)
{// 0: enable, -1: disable
        //printf("set master ++++++++++\n");
        ismaster = enable;
}

int get_master_flag()
{
        //printf("get master ++++++++++\n");
        return ismaster;
}

void set_master_index(int enable)
{//enable >= 0
        avIndex_master = enable;
}

int get_master_index()
{
        return avIndex_master;
}

#endif

unsigned int getTimeStamp()
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

void media_cmd_open_video(int iMediaNo ,video_info_t *info)
{
    LOG("media_cmd_open_video Enter index %d\n", iMediaNo);


    pthread_mutex_lock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
    g_media_struct.g_media_info[iMediaNo].is_VA = 1;
    g_media_struct.g_media_info[iMediaNo].video_framenum = 0;
    pthread_mutex_unlock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);

    pthread_mutex_lock(&list_lock_vid);
    stop_buffer_data(video_live_buffer);
    clean_buffer_data(video_live_buffer);
    start_buffer_data(video_live_buffer);
    pthread_mutex_unlock(&list_lock_vid);

	info->videoframerate = 15;
	info->videonum = get_online_num();

    if(GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_2K) {
		g_media_struct.resolution = YSX_RESOLUTION_HIGH;
        QCamVideoInput_SetIFrame(MAIN_CHN);
		info->videocodec = VAVA_VIDEO_CODEC_H264;
		info->videobitrate = MEDIA_MAIN_BITRATE;
        LOG("set I frame to main\n");
    }
	else if(GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_HIGH) {
		g_media_struct.resolution = YSX_RESOLUTION_MID;
        QCamVideoInput_SetIFrame(SECOND_CHN);
		info->videocodec = VAVA_VIDEO_CODEC_H264;
		info->videobitrate = MEDIA_SECOND_BITRATE;
        LOG("set I frame to second\n");
	}
	else if(GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_RENEWAL)
	{
		g_media_struct.resolution = YSX_RESOLUTION_LOW;
        QCamVideoInput_SetIFrame(THIRD_CHN);
		info->videocodec = VAVA_VIDEO_CODEC_H264;
		info->videobitrate = MEDIA_THIRD_BITRATE;
        LOG("set I frame to third\n");	
	}
	else //auto 自动模式 分辨率不不变化 由media_auto_video_process接管
	{
		
	}
	
	//第一帧要发送 空帧
	ppcs_proto_send_meta_video_frame(VAVA_REALTIME_VIDEO_TAG,1,g_media_struct.g_media_info[iMediaNo].sessionid);
	
	return ;
}


void media_cmd_stop_video(int iMediaNo)
{
    LOG("media_cmd_stop_video Enter index %d\n", iMediaNo);

    pthread_mutex_lock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
    g_media_struct.g_media_info[iMediaNo].is_VA = 0;
    pthread_mutex_unlock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
    LOG("IOTYPE_USER_IPCAM_STOP Enter\n");
	
	return ;
}

void send_video_frame(const void *data, const int len, const struct timeval tv, const int keyframe, int frame_num)
{
	int i;
	int ret;
	unsigned char iNumView = 0;
	
	for(i=0; i<MAX_USER_NUMBER; i++)
	{
        if (g_media_struct.g_media_info[i].is_VA == 0) {
            continue;
        }

		iNumView++;//实时在线用户
		
		//第一帧需要记录下当前 buf 吐出的帧序 media_cmd_open_video中发送了空帧
		if(g_media_struct.g_media_info[i].video_framenum == 0)
		{
			g_media_struct.g_media_info[i].prep_framenum = frame_num;
			g_media_struct.g_media_info[i].video_framenum = 1;
		}
		else
		{
			//如果 连续的情况
			if(g_media_struct.g_media_info[i].prep_framenum+1 == frame_num)
			{
				g_media_struct.g_media_info[i].prep_framenum = frame_num;
			}
			else if(keyframe)//帧序不连续 但是是keyframe 强行同步帧序
			{
				g_media_struct.g_media_info[i].prep_framenum = frame_num;
			}
			else//如果 出现P帧不连续的情况 又不是keyfame 需要进行丢掉P帧的动作
			{
				LOG("#######  @@@@@  #####\n");
				continue;
			}
		}
		
		ret = ppcs_proto_send_video(VAVA_REALTIME_VIDEO_TAG,1,g_media_struct.g_media_info[i].sessionid, data, len, tv, keyframe,g_media_struct.g_media_info[i].video_framenum++);
		if(ret != 0)//如果buff不够 则直接把帧序设置为不连续 这样应该只会发送I帧
		{
			g_media_struct.g_media_info[i].prep_framenum--;
			g_media_struct.g_media_info[i].video_framenum--;
		}
	}


	g_media_struct.g_isVideoNum = iNumView;
	
}

void aud_process(void *pcm_buf, int pcm_len,const struct timeval *tv)
{
    int len;
    int left;

    static unsigned int has_size = 0;
    if( (pcm_buf_size - has_size ) > pcm_len ) {
        memcpy(pcm_buffer + has_size , pcm_buf , pcm_len );
        has_size += pcm_len ;
    }
	else
    {
        memcpy(pcm_buffer + has_size , pcm_buf , (pcm_buf_size - has_size ) );
        left = pcm_len - (pcm_buf_size-has_size);

        len = MY_Audio_PCM2AAC(pcm_buffer, pcm_buf_size, aac_buf, aac_buf_size);
        if(write_audio_frame(aac_buf, len) != 0)
            SdRecord_Stop();


        pthread_mutex_lock(&list_lock_aud);
        write_buffer_data(audio_live_buffer, aac_buf, len, 1, *tv, 0);
        pthread_mutex_unlock(&list_lock_aud);


        has_size = 0;
        memcpy(pcm_buffer + has_size , pcm_buf + pcm_len - left , left );
        has_size += left;

    }
    return ;
}


void media_cmd_open_audio(int iMediaNo ,audio_info_t *info)
{
    LOG("media_cmd_open_audio Enter index %d\n", iMediaNo);

	
	info->audiocodec  =  VAVA_AUDIO_CODEC_AAC;
	info->audiorate = 16000;
	info->audiobitper = 16;
	info->audiochannel = 1;
	info->audioframerate  = 16;
	
    pthread_mutex_lock(&list_lock_aud);
    stop_buffer_data(audio_live_buffer);
    clean_buffer_data(audio_live_buffer);
    start_buffer_data(audio_live_buffer);
    pthread_mutex_unlock(&list_lock_aud);
	
    pthread_mutex_lock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
    g_media_struct.g_media_info[iMediaNo].is_AA = 1;
	g_media_struct.g_media_info[iMediaNo].audio_framenum = 0;
    pthread_mutex_unlock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
	
	return ;
}

void media_cmd_stop_audio(int iMediaNo)
{
	LOG("media_cmd_stop_audio Enter index %d\n", iMediaNo);
	
	pthread_mutex_lock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
	g_media_struct.g_media_info[iMediaNo].is_AA = 0;
	pthread_mutex_unlock(&g_media_struct.g_media_info[iMediaNo].va_Mutex);
	
	return ;
}



void send_audio_frame(void *aud_data, int aud_size, const struct timeval tv)
{
	int i;
	
	for(i=0; i<MAX_USER_NUMBER; i++)
	{
        if (g_media_struct.g_media_info[i].is_AA == 0) {
            continue;
        }
		
		ppcs_proto_send_audio(VAVA_REALTIME_AUDIO_TAG,1,g_media_struct.g_media_info[i].sessionid, aud_data, aud_size, tv, g_media_struct.g_media_info[i].audio_framenum++);
	}

}

void media_cmd_start_speak_audio(int iMediaNo)
{
	int *sid;
	LOG( "media_cmd_start_speak_audio Enter\n");
	
	pthread_mutex_lock(&g_media_struct.sa_Mutex);
	if((g_media_struct.is_SA == 0) && (g_media_struct.is_playing == 0)) {
		g_media_struct.avIndex_s = g_media_struct.g_media_info[iMediaNo].sessionid;
		//g_media_struct.g_media_info[iMediaNo].bEnableSpeaker = 1;
		g_media_struct.is_SA = 1;
		g_media_struct.is_playing = 1;
	
		sid = (int *)malloc(sizeof(sid));
		if(!sid) {
			pthread_mutex_unlock(&g_media_struct.sa_Mutex);
			return;
		}
		*sid = g_media_struct.g_media_info[iMediaNo].sessionid;

	
		pthread_t Thread_ID;
		int ret;
		if((ret = pthread_create(&Thread_ID, NULL, &thread_ReceiveAudio, (void *)sid))<0) {
			pthread_mutex_unlock(&g_media_struct.sa_Mutex);
			LOG("pthread_create ret=%d\n", ret);
			free(sid);
			return;
		}
		pthread_detach(Thread_ID);
	}
	
	pthread_mutex_unlock(&g_media_struct.sa_Mutex);
	
	return ;
	
}

void media_cmd_stop_speak_audio(int iMediaNo)
{
	LOG("media_cmd_stop_speak_audio Enter\n");
	
	pthread_mutex_lock(&g_media_struct.sa_Mutex);
	g_media_struct.is_SA = 0;
	pthread_mutex_unlock(&g_media_struct.sa_Mutex);

	return ;
}


void media_cmd_switch_record_quality(VAVA_VIDEO_QUALITY quality)
{
	if(GetYSXCtl(YSX_RECORD_QUALITY) != quality)//当前设备 录像分辨率不一样
	{
		SetYSXCtl(YSX_RECORD_QUALITY, (int)quality);
		pthread_mutex_lock(&list_lock_vrec_vid);
		stop_buffer_data(video_record_buffer);
		clean_buffer_data(video_record_buffer);
		start_buffer_data(video_record_buffer);
		pthread_mutex_unlock(&list_lock_vrec_vid);				
	}
	//强制I帧
    if(GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_2K) {
        QCamVideoInput_SetIFrame(MAIN_CHN);
    }
	else if(GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_HIGH) {
        QCamVideoInput_SetIFrame(SECOND_CHN);
	}
	else if(GetYSXCtl(YSX_VQUALITY) == VAVA_VIDEO_QUALITY_RENEWAL)
	{
        QCamVideoInput_SetIFrame(THIRD_CHN);
	}
	else //auto 自动模式 分辨率不不变化 由media_auto_video_process接管
	{
		
	}
}

#define AUDIO_BUF_SIZE 2048
void *thread_ReceiveAudio(void *arg)
{
	int session_id;
    int  *sid = (int *)arg;
	int ret;

    unsigned int frmNo = 0;
    unsigned int servType;

    if (!sid)
    {
    	goto function_err;
    }
	session_id =  *sid;
    LOG("thread_ReceiveAudio SID %d \n", *sid);
    free(sid);

    unsigned char *buf = (unsigned char *)malloc(AUDIO_BUF_SIZE);
    if(buf==NULL){
        LOG("malloc Err with %s\n",strerror(errno));
        goto function_err;
    }

    aac_decoder_start();


	
    while (g_media_struct.is_SA) {
		ret = ppcs_proto_recv_audio(session_id,1,buf,AUDIO_BUF_SIZE);
        if(ret > 0) {
            ret = aac_decode(buf+sizeof(VAVA_MSG_HEADER_AV_STREAM), ret);
            if(ret)
                QCamAudioOutputPlay(aac_decoder_buf,ret);
        }
		/*
        int ret = avCheckAudioBuf(avIndex);
        if (ret < 2) { // determined by audio frame rate
            ysx_setTimer(0, 10);
//            usleep(10000);
            continue;
        }

        ret = avRecvAudioData(avIndex, buf, (AUDIO_BUF_SIZE), (char *)&frameInfo, sizeof(ysx_tutk_FRAMEINFO_t), &frmNo);
        if(ret > 0) {
            ret = aac_decode(buf, ret);
            if(ret)
                QCamAudioOutputPlay_ysx(aac_decoder_buf,ret);
        }
        */
        
    }

    if(buf)
        free(buf);
    aac_decoder_stop();

    g_media_struct.is_playing = 0;
    LOG("thread_ReceiveAudio out 1\n");
    return (void *)0;
function_err:
    g_media_struct.is_playing = 0;
    LOG("thread_ReceiveAudio out 2\n");
    return (void *)-1;
}


void media_cmd_switch_quality(VAVA_VIDEO_QUALITY q)
{
	if(q == VAVA_VIDEO_QUALITY_AUTO)
		return ;

	if(GetYSXCtl(YSX_VQUALITY)==VAVA_VIDEO_QUALITY_2K )
	{
		g_media_struct.resolution = YSX_RESOLUTION_HIGH;
		QCamVideoInput_SetIFrame(MAIN_CHN);
	}
	else if(GetYSXCtl(YSX_VQUALITY)==VAVA_VIDEO_QUALITY_HIGH)
	{
		g_media_struct.resolution = YSX_RESOLUTION_MID;
		QCamVideoInput_SetIFrame(SECOND_CHN);
	}
	else
	{
		g_media_struct.resolution = YSX_RESOLUTION_LOW;
		QCamVideoInput_SetIFrame(THIRD_CHN);
	}
		
	if(GetYSXCtl(YSX_VQUALITY) != q) {
		switch_vquality = 1;
		SetYSXCtl(YSX_VQUALITY,q);

		pthread_mutex_lock(&list_lock_vid);
		stop_buffer_data(video_live_buffer);
		clean_buffer_data(video_live_buffer);
		start_buffer_data(video_live_buffer);
		pthread_mutex_unlock(&list_lock_vid);
	}
	return ;
}

void start_md_record()
{

    //   audio_cloud_num = 0;
    //return ;
    LOG("@@@@@@@@@@MD start md message!\n");
   
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start_timestamp = tv.tv_sec*1000 + tv.tv_usec/1000;
    md_write_video_status = 1;
    md_write_audio_status = 1;

}

void Md2Snapshot(char *file_name)
{
	char *pic_buff = NULL;
    int len = 400*1024;

    if(NULL == file_name){
            LOG("malloc buffer Err with %s\n",strerror(errno));
            return;
    }

    pic_buff = (char *)mem_malloc(len);
		
    if(NULL == pic_buff){
            LOG("malloc buffer Err with %s\n",strerror(errno));
            return;
    }

    if(QCamVideoInput_CatchJpeg(pic_buff,&len) == 0)
    {
            FILE *fp = fopen(file_name,"wb+");
            fwrite(pic_buff,1,len,fp);
            fclose(fp);
    }
    mem_free(pic_buff);
    return;
}

static void video_h264fhd_cb(const struct timeval *tv, const void *data, const int len, const int keyframe)
{
    static int frame_num = 0;
    if(data == NULL || len <= 0){
        LOG("video_h264fhd_cb 264 callback error !\n");
        return ;
    }

	frame_num++;

	//SD卡 录像
	if(GetYSXCtl(YSX_RECORD_QUALITY) == VAVA_VIDEO_QUALITY_2K)// VAVA_VIDEO_QUALITY
	{
		pthread_mutex_lock(&list_lock_vrec_vid);
		write_buffer_data(video_record_buffer, (char *)data, len, keyframe, *tv, frame_num);
		pthread_mutex_unlock(&list_lock_vrec_vid);
	}
	
	//直播视频流
    if(g_media_struct.resolution == YSX_RESOLUTION_HIGH){
		//if(keyframe)
		//	printf("fhd frame!\n");
        pthread_mutex_lock(&list_lock_vid);
        write_buffer_data(video_live_buffer, (char *)data, len, keyframe, *tv, frame_num);
        pthread_mutex_unlock(&list_lock_vid);
    }

    return;
}

static void video_h264hd_cb( const struct timeval *tv, const void *data, const int len, const int keyframe)
{
	static int frame_num = 0;

	frame_num++;
	
	//SD卡 录像
	if(GetYSXCtl(YSX_RECORD_QUALITY) == VAVA_VIDEO_QUALITY_HIGH)// VAVA_VIDEO_QUALITY
	{
		pthread_mutex_lock(&list_lock_vrec_vid);
		write_buffer_data(video_record_buffer, (char *)data, len, keyframe, *tv, frame_num);
		pthread_mutex_unlock(&list_lock_vrec_vid);
	}
	
	//直播视频流
    if(g_media_struct.resolution == YSX_RESOLUTION_MID){
		//if(keyframe)
		//	printf("hd key frame!\n");
        pthread_mutex_lock(&list_lock_vid);
        write_buffer_data(video_live_buffer, (char *)data, len, keyframe, *tv, frame_num);
        pthread_mutex_unlock(&list_lock_vid);
    }

    return;
}

static void video_h264vga_cb(const struct timeval *tv, const void *data, const int len, const int keyframe)
{

	static int frame_num = 0;

	frame_num++;
	//SD卡 录像
	if(GetYSXCtl(YSX_RECORD_QUALITY) == VAVA_VIDEO_QUALITY_RENEWAL)// VAVA_VIDEO_QUALITY
	{
		pthread_mutex_lock(&list_lock_vrec_vid);
		write_buffer_data(video_record_buffer, (char *)data, len, keyframe, *tv, frame_num);
		pthread_mutex_unlock(&list_lock_vrec_vid);
	}

	//直播视频流
    if(g_media_struct.resolution == YSX_RESOLUTION_LOW){
		//if(keyframe)
		//	printf("vga frame!\n");
        pthread_mutex_lock(&list_lock_vid);
        write_buffer_data(video_live_buffer, (char *)data, len, keyframe, *tv, frame_num);
        pthread_mutex_unlock(&list_lock_vid);
    }
	return ;
}

void aud_pcm_cb(const struct timeval *tv, const void *pcm_buf, const int pcm_len, const void *spk_buf)
{
    if(pcm_buf == NULL || pcm_len <= 0){
        LOG("aud_pcm_cb callback error !\n");
        return ;
    }
    /*for mp4 record*/
    aud_process((void *)pcm_buf,pcm_len,tv);

    return;
}
#define LICENSE_PATH   "/system_rw/perdetec/license.txt"
int getLicenseFile(){
        if(access(LICENSE_PATH,F_OK) == 0)
                return 1;
        else
                return 0;
}
int Video_Init()
{
    int ret=0;
    QCamVideoInputChannel vid_chn[3];

    vid_chn[MAIN_CHN].res           = QCAM_VIDEO_RES_1296P;//QCAM_VIDEO_RES_500W;
    vid_chn[MAIN_CHN].bitrate       = MEDIA_MAIN_BITRATE;
    vid_chn[MAIN_CHN].cb 			= video_h264fhd_cb;
    vid_chn[MAIN_CHN].channelId		= MAIN_CHN;
    vid_chn[MAIN_CHN].fps			= 15;
    vid_chn[MAIN_CHN].gop 			= 2;	
    vid_chn[MAIN_CHN].vbr 			= 1; 	 /*choose CBR mode*/
    vid_chn[MAIN_CHN].encoding      = 0;	//H265:1  H264:0	

	vid_chn[SECOND_CHN].res         = QCAM_VIDEO_RES_720P; //H264 , AWS  ECHO　RTSPS(TLS) 
    vid_chn[SECOND_CHN].bitrate     = MEDIA_SECOND_BITRATE;
    vid_chn[SECOND_CHN].cb 			= video_h264hd_cb;
    vid_chn[SECOND_CHN].channelId   = SECOND_CHN;
    vid_chn[SECOND_CHN].fps 		= 15;
    vid_chn[SECOND_CHN].gop 		= 2;
    vid_chn[SECOND_CHN].vbr 		= 1; 	 /*choose CBR mode*/
    vid_chn[SECOND_CHN].encoding    = 0;	//H264


 
	vid_chn[THIRD_CHN].res 		 = QCAM_VIDEO_RES_360P;
    vid_chn[THIRD_CHN].bitrate 	 = MEDIA_THIRD_BITRATE;//128;
    vid_chn[THIRD_CHN].cb 			 = video_h264vga_cb;
    vid_chn[THIRD_CHN].channelId    = THIRD_CHN;
    vid_chn[THIRD_CHN].fps 		 = 15;
    vid_chn[THIRD_CHN].gop 		 = 2;
    vid_chn[THIRD_CHN].vbr 		 = 1; 	 /*choose CBR mode*/
    vid_chn[THIRD_CHN].encoding     = 0;	//H264


    ret = QCamVideoInput_Init(); 
    if(ret < 0){
            LOG("QCamVideoInput_Init ERROR !\n");
            return -1;
    }

    /*step 2, add channel*/
    ret = QCamVideoInput_AddChannel(vid_chn[MAIN_CHN]);
    if(ret < 0){
            LOG("QCamVideoInput_AddChannel 0 ERROR !\n");
            return -1;
    }

    ret = QCamVideoInput_AddChannel(vid_chn[SECOND_CHN]);
    if(ret < 0){
            LOG("QCamVideoInput_AddChannel 1 ERROR !\n");
            return -1;
    }
	
    ret = QCamVideoInput_AddChannel(vid_chn[THIRD_CHN]);
    if (ret < 0) {
    	LOG("QCamVideoInput_AddChannel 2 ERROR !\n");
       	return -1;
     }
    
    /*step 3, start stream */
    ret = QCamVideoInput_Start();
    if(ret < 0){
            LOG("QCamVideoInput_Start ERROR !\n");
            return -1;
    }

    QCamVideoInputOSD osd_attr;
    memset(&osd_attr, 0, sizeof(QCamVideoInputOSD));
    osd_attr.pic_enable = 0;
    osd_attr.time_enable = 1;
    osd_attr.time_x = 1724; //1440;	//2560-(19+1)*24   2304-(19+1)*29
    osd_attr.time_y = 0; 

    ret = QCamVideoInput_SetOSD(MAIN_CHN, &osd_attr);
    if (ret < 0) {
            LOG("QCamVideoInput_SetOSD ERROR !\n");
            return -1;
    }

    
	memset(&osd_attr, 0, sizeof(QCamVideoInputOSD));
	osd_attr.pic_enable = 0;
	osd_attr.time_enable = 1;
	osd_attr.time_x = 960; 		//1280-(19+1)*16    
	osd_attr.time_y = 0;

	ret = QCamVideoInput_SetOSD(SECOND_CHN, &osd_attr);
	if (ret < 0) {
	LOG("QCamVideoInput_SetOSD ERROR !\n");
	return -1;
	}
       
       
    memset(&osd_attr, 0, sizeof(QCamVideoInputOSD));
    osd_attr.pic_enable = 0;
    osd_attr.time_enable = 1;
    osd_attr.time_x = 480; 		//640-(19+1)*8 
    osd_attr.time_y = 0;

    ret = QCamVideoInput_SetOSD(THIRD_CHN, &osd_attr);
    if (ret < 0) {
            LOG("QCamVideoInput_SetOSD ERROR !\n");
            return -1;
    }

    return ret;
}

int Audio_Init()
{
    int ret;
    QCamAudioOutputAttribute spk_attr;
    QCamAudioInputAttr attr;

    spk_attr.sampleBit = 16;
    spk_attr.sampleRate = AUDIO_SAMPLERATE_YSX;
    spk_attr.volume = g_enviro_struct.spk_vol;
    ret = QCamAudioOutputOpen(&spk_attr);
    if(ret < 0 )
    {
            LOG("QCamAudioOutputOpen Failed\n");
            return -1;
    }


    attr.sampleBit = 16;
    attr.sampleRate = AUDIO_SAMPLERATE_YSX;
    attr.volume = -1;
    attr.cb = aud_pcm_cb;
    ret = QCamAudioInputOpen(&attr);
    if(ret < 0)
    {
            LOG("QCamAudioInputOpen Failed\n");
            return -1;
    }


    if( aac_decoder_init() < 0 )
            return -1;

    if((audio_encoder_start(attr.sampleRate,&aac_buf_size,&pcm_buf_size)) < 0)
            return -1;

    LOG("################aac_size = %d \n",aac_buf_size);
    LOG("################pcm_size = %d \n",pcm_buf_size);
    if(aac_buf_size == 0 || pcm_buf_size == 0){
            LOG("aac pcm buf_size error\n");
            return -1;
    }

    memset(aac_buf,0,sizeof(aac_buf));
    memset(pcm_buffer,0,sizeof(pcm_buffer));

    memset(&enc_state,0,sizeof(struct adpcm_state));
    memset(&dec_state,0,sizeof(struct adpcm_state));

    ret = QCamAudioInputStart();
    if(ret < 0)
    {
            LOG("QCamAudioInputStart Failed\n");
            return -1;
    }

    return 0;
}


extern int64_t samples_count[2];
extern int64_t next_pts[2] ;

enum{
        TYPE_INVALID = -1,
        TYPE_VIDEO,
        TYPE_AUDIO,
};

void *thd_record_video_pool(void *arg)
{
 	
	prctl(PR_SET_NAME,"thd_record_pool");
	char *tmp_data = NULL;
	int tmp_size = 0;
	int tmp_keyframe = 0;
	struct timeval tmp_tv;
	int index = 0;
	
	clean_buffer_data(video_record_buffer);
    while(g_enviro_struct._loop) {	
		pthread_mutex_lock(&list_lock_vrec_vid);
		read_buffer_data(video_record_buffer, &tmp_data, &tmp_size, &tmp_keyframe, &tmp_tv, &index);
		pthread_mutex_unlock(&list_lock_vrec_vid);

		if(tmp_size) {
			if(tmp_data) {
				if(SdRecord_Start(tmp_keyframe, tmp_tv, tmp_data, tmp_size) == 0)
				{
					if(write_video_frame(tmp_keyframe, tmp_data, tmp_size, &tmp_tv) != 0)
						SdRecord_Stop();
				}
			}
		}
		else
			usleep(50*1000);
			
    }
    
    LOG("thd_record_video_pool exit\n");
    pthread_exit(0);
}




void *thd_vid_pool(void *arg)
{
    prctl(PR_SET_NAME,"thd_vid0_pool");

    int ret;
    char *tmp_data = NULL;
    int tmp_size = 0;
    int tmp_keyframe = 0;
    struct timeval tmp_tv;
    int tmp_frame_num = 0;
	char *swap_data = NULL;

	swap_data = malloc(MAX_KEYFRAME_LENGTH);
	if(!swap_data)
		exit(0);//never happen

    clean_buffer_data(video_live_buffer);

    while(g_enviro_struct._loop) {
		tmp_size = 0; //read buff can't init this 
		tmp_data = NULL;
	
		if( (GetYSXCtl(YSX_C_EN) == 0) || (get_online_num() == 0) ) {
            ysx_setTimer(0, 50);
            continue;
		}

        pthread_mutex_lock(&list_lock_vid);
        read_buffer_data(video_live_buffer, &tmp_data, &tmp_size, &tmp_keyframe, &tmp_tv, &tmp_frame_num);
		
		if(tmp_size <= (MAX_KEYFRAME_LENGTH-sizeof(VAVA_MSG_HEADER_AV_STREAM)) )//注意点  留出 msg header的空间
			memcpy(&swap_data[sizeof(VAVA_MSG_HEADER_AV_STREAM)],tmp_data,tmp_size);
		else
		{
			LOG("####  NEVER HAPPEN !!\n");
			tmp_data = NULL;
		}
        pthread_mutex_unlock(&list_lock_vid);

		//ysx_h264_udp_send("192.168.199.56", swap_data, tmp_size);

		//LOG("ret=%d, size=%d\n", ret, tmp_size);
        if(tmp_size) {
            if(tmp_data) {
                if(switch_vquality) {
                    if(tmp_keyframe) {
                        send_video_frame(swap_data, tmp_size, tmp_tv, tmp_keyframe, tmp_frame_num);
                        switch_vquality = 0;
                    }
                } else{
                    send_video_frame(swap_data, tmp_size, tmp_tv, tmp_keyframe, tmp_frame_num);
                }

            }
        } else
            ysx_setTimer(0, 50);

    }

	free(swap_data);
    LOG("thd_vid0_pool exit\n");
    pthread_exit(0);
}



void *thd_aud_pool(void *arg)
{
    prctl(PR_SET_NAME,"thd_aud_pool");

    char *tmp_data = NULL;
    int tmp_size = 0;
    int tmp_keyframe = 0;
    struct timeval tmp_tv;
    int tmp_frame_num = 0;

	char *swap_data = NULL;

	swap_data = malloc(AUDIO_LIVE_BUFFER_SIZE);
	if(!swap_data)
		exit(0);

    clean_buffer_data(audio_live_buffer);

    while(g_enviro_struct._loop) {
        if((GetYSXCtl(YSX_C_EN) == 1) &&
            (GetYSXCtl(YSX_MIC_EN) == 1) &&
            (get_online_num() > 0)) {

        } else {

            ysx_setTimer(0, 50);
            continue;
        }

        pthread_mutex_lock(&list_lock_aud);
        read_buffer_data(audio_live_buffer, &tmp_data, &tmp_size, &tmp_keyframe, &tmp_tv, &tmp_frame_num);
		if(tmp_size<= (AUDIO_LIVE_BUFFER_SIZE-sizeof(VAVA_MSG_HEADER_AV_STREAM)) )
			//memcpy(swap_data,tmp_data,tmp_size);
			memcpy(&swap_data[sizeof(VAVA_MSG_HEADER_AV_STREAM)],tmp_data,tmp_size);
		else
		{
			LOG("####  NEVER HAPPEN !!\n");
			tmp_data = NULL;
		}
        pthread_mutex_unlock(&list_lock_aud);

        //LOG("%d\n", tmp_size);
        if(tmp_size) {
            if(tmp_data) {
                send_audio_frame(swap_data,tmp_size,tmp_tv);
            }
        } else
            ysx_setTimer(0, 50);
    }

	free(swap_data);
    LOG("thd_aud_pool exit\n");
    pthread_exit(0);
}


int thread_stream_init()
{
    int ret;

    pthread_mutex_init(&list_lock_aud,NULL);
    pthread_mutex_init(&list_lock_vid,NULL);

    pthread_mutex_init(&list_lock_vrec_vid,NULL);
    pthread_mutex_init(&list_lock_vrec_aud,NULL);

    pthread_mutex_init(&list_lock_md_vid,NULL);
    pthread_mutex_init(&list_lock_md_aud,NULL);

	video_record_buffer = init_av_buffer(VIDEO_RECORD_BUFFER_SIZE, USED_VIDEO_RECORD_BUF_NUM, MAX_VIDEO_RECORD_BUF_NUM);
	if(!video_record_buffer)
	{
		LOG("init_av_buffer video_record_buffer ERROR!\n");
		return -1;
	}

    video_live_buffer = init_av_buffer(VIDEO_LIVE_BUFFER_SIZE, USED_VIDEO_LIVE_BUF_NUM, MAX_VIDEO_LIVE_BUF_NUM);
    if(!video_live_buffer) {
        LOG("init_av_buffer video_live_buffer ERROR!\n");
        return -1;
    }

    audio_live_buffer = init_av_buffer(AUDIO_LIVE_BUFFER_SIZE, USED_AUDIO_LIVE_BUF_NUM, MAX_AUDIO_LIVE_BUF_NUM);
    if(!audio_live_buffer) {
        LOG("init_av_buffer video_live_buffer ERROR!\n");
        return -1;
    }

	 

    pthread_t thd_vid,thd_record_aud,thd_record_vid,thd_aud,thd_vid_cloud,thd_aud_cloud;
    if((ret = pthread_create(&thd_vid, NULL, &thd_vid_pool, NULL)) < 0)
    {
        LOG("pthread_create for thd_vid_pool error ret=%d\n", ret);
        return -1;
    }
    pthread_detach(thd_vid);


    if((ret = pthread_create(&thd_aud, NULL, &thd_aud_pool, NULL)) < 0)
    {
        LOG("pthread_create for thd_aud_pool erro ret=%d\n", ret);
        return -1;
    }
    pthread_detach(thd_aud);
	
	
    if((ret = pthread_create(&thd_record_vid, NULL, &thd_record_video_pool, NULL)) < 0)
    {
        LOG("pthread_create for thd_vid_pool erro ret=%d\n", ret);
        return -1;
    }
    pthread_detach(thd_record_vid);
	
	
    return 0;	
}




int init_media()
{
    /*set cur video for video.c****/
    int i=0;
	printf("enter init_media\n");
    for(i=0; i<10; i++){
            g_media_struct.g_media_info[i].sessionid = -1;
            g_media_struct.g_media_info[i].is_VA = 0;
            g_media_struct.g_media_info[i].is_AA = 0;
            pthread_mutex_init(&g_media_struct.g_media_info[i].va_Mutex,NULL);
            pthread_mutex_init(&g_media_struct.g_media_info[i].aa_Mutex,NULL);
			

			pthread_mutex_init(&g_media_struct.g_media_info[i].pb_info.pb_mutex,NULL);
			stop_sd_playback(&g_media_struct.g_media_info[i].pb_info);
    }

    pthread_mutex_init(&g_media_struct.sa_Mutex,NULL);
    pthread_mutex_init(&g_media_struct.pa_Mutex,NULL);	/*playback lock init*/
    g_media_struct.g_isVideoNum = 0;
    g_media_struct.playch = -1;

    g_media_struct.now_pavIndex = -1;
    g_media_struct.bChange = 0;
    g_media_struct.bPlayStatus = PLAY_STOP;

	//如果是auto 模式  默认设置最高分辨率
	if(GetYSXCtl(YSX_VQUALITY)==VAVA_VIDEO_QUALITY_AUTO || GetYSXCtl(YSX_VQUALITY)==VAVA_VIDEO_QUALITY_2K )
		g_media_struct.resolution = YSX_RESOLUTION_HIGH;
	else if(GetYSXCtl(YSX_VQUALITY)==VAVA_VIDEO_QUALITY_HIGH)
		g_media_struct.resolution = YSX_RESOLUTION_MID;
	else
		g_media_struct.resolution = YSX_RESOLUTION_LOW;
		

    if(thread_stream_init() != 0){
        EMGCY_LOG("thread_stream_init Init error \n");
        return -1;
    }

   // Record_Init();

    if(Audio_Init() != 0){
            EMGCY_LOG("Audio Init error \n");
            return -1;
    }
    if(Video_Init() != 0){
            EMGCY_LOG("Video Init error \n");
            return -1;
    }	
    //if(!getLicenseFile())
    YSX_IVS_Init();	

    QCamVideoInput_SetInversion(GetYSXCtl(YSX_INVERSION));

    set_ir_mode(GetYSXCtl(YSX_IR_MODE));

	printf("exit init_media\n");
    return 0;
}

static void DeInit_Stream_Buffer()
{
    deinit_av_buffer(video_live_buffer);
    deinit_av_buffer(audio_live_buffer);
    deinit_av_buffer(video_record_buffer);	
}

static void media_lock_release()
{
    pthread_mutex_destroy(&list_lock_aud);
    pthread_mutex_destroy(&list_lock_vid);

    pthread_mutex_destroy(&list_lock_vrec_vid);
    pthread_mutex_destroy(&list_lock_vrec_aud);

    pthread_mutex_destroy(&list_lock_md_vid);
    pthread_mutex_destroy(&list_lock_md_aud);
	return ;
	
}

void Media_Close()
{
        int i=0;
        g_media_struct.is_SA = 0;
        for(i=0; i<10; i++){
                g_media_struct.g_media_info[i].sessionid = -1;
                g_media_struct.g_media_info[i].is_VA = 0;
                g_media_struct.g_media_info[i].is_AA = 0;
                pthread_mutex_destroy(&g_media_struct.g_media_info[i].va_Mutex);
                pthread_mutex_destroy(&g_media_struct.g_media_info[i].aa_Mutex);
        }

        pthread_mutex_destroy(&g_media_struct.sa_Mutex);
        pthread_mutex_destroy(&g_media_struct.pa_Mutex);	
        g_media_struct.g_isVideoNum = 0;
        g_media_struct.playch = -1;
        g_media_struct.now_pavIndex = -1;
        g_media_struct.bChange = 0;
        g_media_struct.bPlayStatus = PLAY_STOP;

       // Record_Uninit();
        g_enviro_struct._loop = 0;
	   
        QCamVideoInput_Uninit();		/*video*/
        QCamAudioInputStop();			/*mic*/
        QCamAudioInputClose();	
        //	QCamAudioOutputClose(); 		/*speaker*/
        //	sleep(1);

        DeInit_Stream_Buffer();
        media_lock_release();

        if(aac_decoder_buf)
                free(aac_decoder_buf);
        aac_decoder_buf = NULL;	

        LOG("End Media close !\n");
}


