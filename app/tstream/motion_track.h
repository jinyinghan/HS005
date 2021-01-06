/*************************************************************************
	> File Name: motion_track_drawrect.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月14日 星期五 16时12分38秒
 ************************************************************************/

#ifndef _MOTION_DRAW_ERCT_H_
#define _MOTION_DRAW_RECT_H_


#define EMERGENCY_DEBUG
#ifdef  EMERGENCY_DEBUG
#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"

#define EMGCY_LOG(fmt...)   \
	        do {\
				printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0)  
#endif 

#define VIDEO_STREAM_NUM    2
#define IVS_GROUP_ID        0
#define IVS_CHANNEL_ID      1


enum {
    VIDEO_QUALITY_TYPE_MAIN     = 0x01,
    VIDEO_QUALITY_TYPE_SECOND   = 0x04,
    VIDEO_QUALITY_TYPE_THREE    = 0x05,
    VIDEO_QUALITY_TYPE_NUMS     = 0x06,
};


enum {
    VIDEO_STREAM_MAIN           = 0x00,
    VIDEO_STREAM_SECOND         = 0x01, 
    VIDEO_STREAM_THREE          = 0x02,
    VIDEO_STREAM_NUMS           = 0x03,
};

struct osd_rect{
	uint8_t enable;
	uint16_t x0;
	uint16_t y0;
	uint16_t x1;
	uint16_t y1;
	uint16_t linewidth;  // pixel
};

void * motion_drawRect_thread(void *arg);

#endif
