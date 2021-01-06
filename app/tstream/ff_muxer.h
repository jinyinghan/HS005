#ifndef __RECORD__
#define __RECORD__
#include <stdio.h>
#include <stdint.h>
#include <time.h>


#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "linklist.h"
#include "libavutil/log.h"
#include "libavutil/timestamp.h"
//#define SET_RECORD_WITH_AUDIO
//#define MAX_VID_NODE 50
#define MAX_H265_NODE				30
#define MAX_VID_NODE 				80
#define MIN_VID_NODE 				30
#define MD_VID_NODE  				15
#define MD_AUD_NODE  				15

#define MAX_AUD_NODE 				100

#if (defined S210)||(defined S211)||(defined S212)||(defined S213)		// FOR T10
#define MAIN_RES				QCAM_VIDEO_RES_720P
#define MAIN_WIDTH  			1280
#define MAIN_HEIGHT 			720
#define H264_FRAME_RATE 		15
#define MAIN_BITRATE  	 		(768)
#define H264_GOP				2

#elif (defined S202)||(defined S203)

#define MAIN_RES				QCAM_VIDEO_RES_1080P
#define MAIN_WIDTH				1920
#define MAIN_HEIGHT 			1080
#define H264_FRAME_RATE 		15
#define MAIN_BITRATE   			(896)
#define H264_GOP				2

#elif (defined DG201)

#define MAIN_RES				QCAM_VIDEO_RES_1080P
#define MAIN_WIDTH				1920
#define MAIN_HEIGHT 			1080
#define H264_FRAME_RATE 		15
#define MAIN_BITRATE   			(768)
#define H264_GOP				2

#elif 	(defined S301)								// FOR T20
#define MAIN_RES				QCAM_VIDEO_RES_1080P
#define MAIN_WIDTH  			1920
#define MAIN_HEIGHT 			1080
#define H264_FRAME_RATE 		15
#define MAIN_BITRATE   			(1024)
#define H264_GOP				2
#endif
#ifndef SECOND_BITRATE
#define SECOND_BITRATE  	 	256
#endif

#define RECORD_INDEX   	  		"/tmp/rindex"		// "./CameraVideo" //
#define RECORD_INDEX_TMP  		"/tmp/rindex_tmp"

#define RECORD_DURATION 		(59)		/*in units of secord*/

typedef struct frame_packet_t{
	uint8_t *data;
    int   size;
    int   stream_index;
    int   flags;
}MY_AVPacket;

enum{
	FRAME_VIDEO_TYPE =1,
	FRAME_AUDIO_TYPE,
	FRAME_NONE_TYPE
};

enum record_channel_number{
RECORD_CHANNEL_ONE=0,
RECORD_CHANNEL_TWO

};


#if 0
#define H264_EXTRA_DATA_SIZE (34)
static unsigned char h264_extra_data[H264_EXTRA_DATA_SIZE] = 
	{0x00,0x00,0x00,0x01,0x67,0x64,0x00,0x1f,0xac,0x3b,0x50,0x28,0x02,0xdd,0x08,0x00,
	 0x00,0x03,0x00,0x08,0x00,0x00,0x03,0x00,0xf4,0x20,0x00,0x00,0x00,0x01,0x68,0xee,
	 0x3c,0x80};
#else
#define H264_EXTRA_DATA_SIZE (33)
static unsigned char h264_extra_data[H264_EXTRA_DATA_SIZE] = 
	{0x00,0x00,0x00,0x01,0x67,0x64,0x00,0x16,0xac,0x3b,0x50,0x50,0x17,0xFC,0xB0,0x80,
	 0x00,0x00,0x03,0x00,0x80,0x00,0x00,0x0f,0x42,0x00,0x00,0x00,0x01,0x68,0xee,0x3c,
	 0x80};


#endif

#define H265_EXTRA_DATA_SIZE (85)
static unsigned char h265_extra_data[H265_EXTRA_DATA_SIZE] = 
	 {0x00,0x00,0x00,0x01,0x40,0x01,0x0c,0x01,0xff,0xff,0x01,0x60,0x00,0x00,0x03,0x00,
	  0x90,0x00,0x00,0x03,0x00,0x00,0x03,0x00,0x96,0xba,0x02,0x40,0x00,0x00,0x00,0x01,
	  0x42,0x01,0x01,0x01,0x60,0x00,0x00,0x03,0x00,0x90,0x00,0x00,0x03,0x00,0x00,0x03,
	  0x00,0x96,0xa0,0x01,0x40,0x20,0x07,0x81,0x65,0xba,0x92,0x4c,0xa2,0x01,0x00,0x00,
	  0x03,0x00,0x01,0x00,0x00,0x03,0x00,0x0f,0x08,0x00,0x00,0x00,0x01,0x44,0x01,0xc0,
	  0x72,0xce,0x70,0x22,0x40};
	  

#define AAC_EXTRA_DATA_SIZE (2)
static unsigned char aac_extra_data[AAC_EXTRA_DATA_SIZE] = 
	{0x14,0x08};

struct mp4_Info {
	AVFormatContext *oc;
	AVStream *audio_st;
	AVStream *video_st;
	AVBitStreamFilterContext* aacbsfc;
	
	uint64_t vid_usec;
	time_t last_vid_pts;
	uint64_t aud_usec;
	time_t last_aud_pts;

	//int64_t vid_usec;
	//int64_t last_vid_pts;
	//int64_t aud_usec;
	//int64_t last_aud_pts;
	
	pthread_mutex_t mutex;
};

int write_video_frame(struct mp4_Info *info,int keyframe,uint8_t *buf, int len ,const struct timeval *tv,enum record_channel_number record_channel);
int write_audio_frame(struct mp4_Info *info,uint8_t *buf, int len,const struct timeval *tv,enum record_channel_number record_channel);
int SdRecord_Start(int keyframe , struct timeval tv,uint8_t *buf , int len);
//int SdRecord_Stop();
int SdRecord_Stop(struct mp4_Info *info);
int capacity_check(char *mount_point);
void Record_Init();
void Record_Uninit();
int record_demuxer_init(char *in_filename, unsigned char *video_codec_id);
int get_record_frame(AVPacket *pkt, int *delay , int frame_type);
void reset_record_frame(AVPacket *pkt);
void record_demuxer_uninit();

extern int online;
extern int sd_exit_flag;
int md_record_init(const char *filename);

enum mRecord_type{
TYPE_SD_RECORD=0,
TYPE_CLOUD_RECORD,
};

#endif
