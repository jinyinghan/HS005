/*******************************************************************************
* write at SZ, 2018-10-11 11:08:16, by echo
*******************************************************************************/
#include "ysx_data_buf.h"
#include "ppcs_protocal.h"

#if (!defined YSX_MEDIA_H)
#define YSX_MEDIA_H

#define MAX_KEYFRAME_LENGTH  (256*1024)

#define VIDEO_LIVE_BUFFER_SIZE 		512*1024
#define USED_VIDEO_LIVE_BUF_NUM 	15
#define MAX_VIDEO_LIVE_BUF_NUM		30

#define VIDEO_RECORD_BUFFER_SIZE 	512*1024
#define USED_VIDEO_RECORD_BUF_NUM 	15
#define MAX_VIDEO_RECORD_BUF_NUM	30

#define VIDEO_MD_BUFFER_SIZE 		256*1024
#define MAX_VIDEO_MD_BUF_NUM		30

#define AUDIO_LIVE_BUFFER_SIZE 		128*1024
#define USED_AUDIO_LIVE_BUF_NUM 	10
#define MAX_AUDIO_LIVE_BUF_NUM		20


#define MEDIA_MAIN_BITRATE    (1024*1.5)
#define MEDIA_SECOND_BITRATE  (768)
#define MEDIA_THIRD_BITRATE   (192)



extern struct mp4_Info fulltime_mp4;
#ifdef YSX_CLOUD
extern struct mp4_Info md_mp4;
#endif

extern av_buffer *video_record_buffer;
extern av_buffer *video_live_buffer;
extern av_buffer *audio_live_buffer;

void media_cmd_open_video(int iMediaNo,video_info_t *info);
void media_cmd_stop_video(int iMediaNo);

void media_cmd_open_audio(int iMediaNo,audio_info_t *info);
void media_cmd_stop_audio(int iMediaNo);


void Media_Close();

#endif
