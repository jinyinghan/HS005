

//此文件 实现了 《Sunvalley云存储推流SDK接口文档V2.0.0》 的内容
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <stdbool.h>

#include "json/json.h"
#include "vava_svpush.h"
#include "isvpush.h"
#include "media.h"
#include "log.h"


#define MAX_CHANNEL_NUM			20
#define TOKEN_SERVER_URL		"http://10.30.0.200:2118/connection/token/get"
#define CHECK_RESULT(ret) if(ret < 0) {printf("assert %s\n",__LINE__);assert(0); return (void *)0;}
#define TEST_DEVICE_SN		"P020101000101200707400001"
bool g_brunning = false;

int vava_event_callback(long lCID, long eventType, long wparam, long lparam)
{
        //CAutoLock lock(m_crisec);

        /*std::map<long, CPushTask*>::iterator it = m_mconnect_task_list.find(lCID);
        if(it == m_mconnect_task_list.end())
        {
            printf("Invalid connect task id:%ld eventType:%ld, wparam:%ld, lparam:%ld\n", lCID, eventType, wparam, lparam);
            //assert(0);
            return -1;
        }*/

        switch(eventType)
        {
            case E_SV_EVENT_TYPE_RTMP_INVALID_URL:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_DNS_RESOVLE_FAILED:
            case E_SV_EVENT_TYPE_RTMP_SOCKET_CONNECT_FAILED:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_HANDSHAKE_FAILED:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_APP_FAILED:
            case E_SV_EVENT_TYPE_RTMP_PUBLISH_STREAM_FAILED:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_SET_TIMEOUT_FAILED:
            case E_SV_EVENT_TYPE_RTMP_INVALID_PARAMETER:
            {
                printf("lcid:%ld connect error, errorcode:%d\n", lCID, eventType);
                //ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE:
            {
                printf("lcid:%ld connection close\n", lCID);
                //ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_CONNECT_SUCCESS:
            {
                printf("lcid:%ld connection success\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_AUDIO_ADTS_DATA_ERROR:
            case E_SV_EVENT_TYPE_RTMP_VIDEO_START_CODE_ERROR:
            case E_SV_EVENT_TYPE_RTMP_RECV_AUDIO_DATA_ERROR:
            case E_SV_EVENT_TYPE_RTMP_SEND_VIDEO_NO_SPS_PPS:
            case E_SV_EVENT_TYPE_RTMP_SEND_ERROR_SPS_PPS:
            case E_SV_EVENT_TYPE_RTMP_SEND_ERROR_SPS:
            case E_SV_EVENT_TYPE_RTMP_SEND_ERROR_PPS:
            {
                printf("lcid:%ld connection error, errorcode:%d\n", lCID, eventType);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH:
            {
                printf("lcid:%ld connection resume media push\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH:
            {
                printf("lcid:%ld connection pause media push\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR:
            {
                printf("lcid:%ld connection error, push thread exit\n", lCID);
                g_brunning = false;
                //ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_OPEN:
            {
                g_brunning = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_CLOSE:
            {
                g_brunning = false;
                return 0;
            }
            default:
            {
                 printf("lcid:%ld connection unknown error,  errorcode:%ld\n", lCID, eventType);
                return 0;
            }

        }
}


void *vava_test_thread(void *arg);

void vava_test_init(void)
{
	int ret;
	 pthread_t vava_thread;
    // create thread to login because without WAN still can work on LAN
    if(ret = pthread_create(&vava_thread, NULL, &vava_test_thread, NULL))
    {
        LOG("vava_test_thread fail, ret=[%d]\n", ret);
        return ;
    }
    pthread_detach(vava_thread);
	return ;
}

#define VAVA_TEST

//extern  void start_vava_md_record(void);

void *vava_test_thread(void *arg)
{
	char ver[256];
	struct timeval tv,start_tv;
	int ret = 0;
	long lcid = -1;
	char* pkeystring = "vavakey";
	char* pvideodata = NULL;
	int video_data_len = 0;
	char* paudiodata = NULL;
	int audio_data_len = 0;
	char *video_buf = NULL;
	char *audio_buf = NULL;
	int64_t pts = 0;
	int keyflag = 0;
	int encflag = 0;
	static int vava_md_write_video_status=0;//控制KEYFAME
	static int vava_md_write_audio_status=0;//控制KEYFAME
	unsigned long long untsamp;
	// get sdk version
    SVPush_API_Version(ver, 256);
    printf(ver);
    printf("\n sunvalley cloud storage push sdk example\n");
	
#ifdef VAVA_TEST
	//sdk initialize
    ret = SVPush_API_Initialize(1024*1024, MAX_CHANNEL_NUM);
    CHECK_RESULT(ret);
    ret = SVPush_API_Init_log((E_SV_LOG_LEVEL)E_LOG_LEVEL_INFO, SV_LOG_OUTPUT_MODE_CONSOLE|SV_LOG_OUTPUT_MODE_FILE , "/tmp/log/");
    CHECK_RESULT(ret);
    ret = SVPush_API_Set_Token_Server_Interface(TOKEN_SERVER_URL);
    CHECK_RESULT(ret);

	lcid = SVPush_API_Connect_By_DeviceSN("live", NULL, TEST_DEVICE_SN, vava_event_callback);
	if(lcid < 0)
	{
		printf("push sdk connect media server failed, lcid:%ld, TEST_DEVICE_SN:%s\n", lcid, TEST_DEVICE_SN);
		assert(0); // 偶尔会失败
		return (void *)0;;
	}
#endif

	//CHECK_RESULT(ret);
	//g_brunning = true;
	video_buf = malloc(MAX_KEYFRAME_LENGTH);
	if(video_buf == NULL)
	{
		printf("malloc err\n"); exit(0);
	}

    do
    {
		gettimeofday(&start_tv, NULL);
		printf("start push t:%llu~~ \n",start_tv.tv_sec);

		gettimeofday(&tv, NULL);
		untsamp = ((unsigned long long)tv.tv_sec /*+3600*16*/) * 1000 + tv.tv_usec / 1000;
#ifdef VAVA_TEST
		ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_VIDEO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_KEY_FRAME, (long)(pkeystring));
		//CHECK_RESULT(ret);
		ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_AUDIO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_ALL_FRAME, (long)(pkeystring));
		//CHECK_RESULT(ret);
		ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_STREAM_START, E_IPC_TIGGER_TYPE_FACE, (long)(&untsamp));
#endif
		//send av frame 15s
		//start_vava_md_record();
		vava_md_write_video_status = 1;
	
		while( tv.tv_sec - start_tv.tv_sec < 15)
		{
		    char *tmp_data = NULL;
		    int tmp_size = 0;
		    int tmp_keyframe = 0;
		    struct timeval tmp_tv;
		    unsigned char tmp_frame_num = 0;

			// handle video !!
			//tmp_data = md_read_buffer(video_buf,&tmp_size, &tmp_keyframe, &tmp_tv,&tmp_frame_num);

			//printf("tmp_data %p tmp_size %d tmp_keyframe %d\n",tmp_data, tmp_size,tmp_keyframe);
			
            if(tmp_size) {
                if(tmp_data) {
                    if((vava_md_write_video_status == 1)&&(tmp_keyframe)){
                        vava_md_write_video_status = 2;
                    }
					if(vava_md_write_video_status == 2)
					{
						
#ifdef VAVA_TEST
						SVPush_API_Send_Packet(lcid, E_SV_MEDIA_TYPE_H264, video_buf,tmp_size, tv.tv_sec*1000+tv.tv_usec/1000, tmp_keyframe, 0);
#endif
						if(tmp_keyframe)
						{
							printf("vava_test_thread it's key frame !\n");
						}
					}
                }
            }

			// handle audio !!			


			gettimeofday(&tv, NULL);
			//printf("s %d\n",tv.tv_sec);
			
			usleep(30*1000);
		}



		
		gettimeofday(&tv, NULL);
		untsamp = ((unsigned long long)tv.tv_sec /*+3600*16*/) * 1000 + tv.tv_usec / 1000;
		// segment end alarm time 10000ms
		vava_md_write_video_status = 0;
#ifdef VAVA_TEST
		ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_STREAM_END, 10000, (long)(&untsamp));
		CHECK_RESULT(ret);
#endif		
		printf("end push and sleep 30s ~~ \n");
    	sleep(30);
#if 0    
        ret = recdemux.readframe(&iph, pbuf, max_buf_len);
        if(ret < 0)
        {
            printf("ret:%d = recdemux.readframe(&iph, pbuf, max_buf_len) failed\n", ret);
            break;
        }
		encflag = 0;
        ret = SVPush_API_Send_Packet(lcid, (E_SV_MEDIA_TYPE)iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag, 0);
        if(ret < 0)
        {
            printf("ret:%d = SVPush_API_Send_Packet(lcid:%ld, media_type:%d, pbuf:%p, iph.size:%d, iph.pts:" PRId64 ", iph.keyflag:%d, 0)\n", ret, lcid, iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag);
            break;
        }
        /*if(iph.codec_id == codec_id_h264 || codec_id_h265 == iph.codec_id)
        {
            // read h264 data from file(annexb format)
            // ...
            //send h264 data packet
            ret = SVPush_API_Send_Packet(lcid, iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag, 0);
        }
		
		// read aac packet data from file
		// ...
		//send aac data packet(with adts header)
		keyflag = 1;
       	ret = SVPush_API_Send_Packet(lcid, E_SV_MEDIA_TYPE_AAC, paudiodata, audio_data_len, pts, keyflag, encflag);*/
		usleep(30000);
#endif
    } while (1);

	free(video_buf);
    SVPush_API_UnInitialize();

    return (void *)0;
}




