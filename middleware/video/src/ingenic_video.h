    /*************************************************************************
	> File Name: ingenic_video.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月20日 星期四 14时27分27秒
 ************************************************************************/

#ifndef _INGENIC_VIDEO_H_
#define _INGENIC_VIDEO_H_

/*
????
gpio_opt -n 72 -m 1 -v 0
gpio_opt -n 73 -m 1 -v 1

ҹ??
gpio_opt -n 72 -m 1 -v 1
gpio_opt -n 73 -m 1 -v 0
*/
#define EFFECT_VALUE            00
#if 0
#define PIN_IRCUT_P             72
#define PIN_IRCUT_N             73
#endif 

#define PIN_IRCUT_P             73
#define PIN_IRCUT_N             72


#define PIN_IRLIGHT             90
#define PIN_WHITE_LIGHT         61


#define FPS_DAY_DEFAULT     15
#define FPS_NIGHT_DEFAULT   10


enum {
    CURRENT_IS_NIGHT    = 0x00,
    CURRENT_IS_DAY      = 0x01,
};


#define EMERGENCY_DEBUG
#if defined EMERGENCY_DEBUG
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
#else 
EMGCY_LOG(fmt...)
#endif 

enum IRCUT_MODE_SELECT {
    IRCUT_MODE_ON       = 0x00,
    IRCUT_MODE_OFF      = 0x01,
    IRCUT_MODE_AUTO     = 0x02,
    IRCUT_MODE_NUMS     = 0x03
};



int set_ircut_mode(int mode);
int system_module_bind(void);
//void * get_video_stream(void * arg);


#endif 
