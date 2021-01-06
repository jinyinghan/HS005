#ifndef __RECORD__
#define __RECORD__
#include <stdio.h>
#include <stdint.h>
#include <time.h>


#include "mp4v2/mp4v2.h"
#include "linklist.h"

//#define MAX_VID_NODE 50
#define MAX_VID_NODE 				80
#define MIN_VID_NODE 				30
#define MD_VID_NODE  				15
#define MD_AUD_NODE  				10
#define RECORD_NODE					20

#define MAX_AUD_NODE 				100
#define MAIN_CHN   					0
#define SECOND_CHN 					1

#if (defined S210)||(defined S211)||(defined S212)||(defined S213)		// FOR T10
#define MAIN_RES				QCAM_VIDEO_RES_720P
#define MAIN_WIDTH  			1920
#define MAIN_HEIGHT 			1080
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

#else									// FOR T20
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

#define RECORD_INDEX_END_MARK  "[END]"

#define RECORD_DURATION 		(59)		/*in units of secord*/

typedef struct frame_packet_t{
	uint8_t *data;
    int   size;
    int   stream_index;
    int   flags;
	int   starttime;
}MY_AVPacket;

enum{
	FRAME_VIDEO_TYPE =1,
	FRAME_AUDIO_TYPE,
	FRAME_NONE_TYPE
};

typedef enum {
    READDIR_MIN = -1,
    READDIR_ERR,
    READDIR_EMPTY = 2,
    READDIR_UNEMPTY,
    READDIR_MAX,
}ysx_readdir_type_e;


int write_video_frame(int keyframe,uint8_t *buf, int len ,const struct timeval *tv);
int write_audio_frame(uint8_t *buf, int len);
int SdRecord_Start(int keyframe , struct timeval tv,uint8_t *buf , int len);
int SdRecord_Stop();
int capacity_check(char *mount_point);


extern int online ;
extern int sd_exit_flag;

int md_record_init(const char *filename);
int md_write_video_frame(int keyframe, uint8_t * buf, int len, const struct timeval *tv);
int md_write_audio_frame(uint8_t *buf, int len,const struct timeval *tv);
int md_record_close();


#endif
