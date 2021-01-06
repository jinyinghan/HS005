#ifndef __YSX_VIDEO_H__
#define __YSX_VIDEO_H__

#include "common.h"
#include <errno.h>
#include <ysx_sys.h>
#include "sample-common.h"

#define TAG "ysx-Encoder-video"

#define YUV_CHN 2
#define MAX_CHN 3
#define JPG_CHN 3//FS_CHN_NUM
#define YSX_VERSION "v-0.0.4"

#define MAIN_OSD_WIDTH 		29//24
#define MAIN_OSD_HEIGHT 	58//48

#define SECOND_OSD_WIDTH 	16
#define SECOND_OSD_HEIGHT 	32

#define THIRD_OSD_WIDTH 	8
#define THIRD_OSD_HEIGHT 	16

#define YSX_MAX_QP  45 //45
#define YSX_MIN_QP  15 //15

#if 0
#define IR_LED     61
#define IR_LED_OFF 1
#define IR_LED_ON  0
#define IRCUT_P    25 
#define IRCUT_N    26
#define ADC_CHN    0
#endif 

// GPIO PIN RELATED
#if 0
#define IR_LED     90
#define IRCUT_P    72 
#define IRCUT_N    73
#else
#define IR_LED     14
#define IRCUT_P    59
#define IRCUT_N    60

#endif
#define IR_LED_OFF 0
#define IR_LED_ON  1
#define ADC_CHN    0

//#define SENSOR_FPS_DAY   		15
#define SENSOR_FPS_DAY   		20

#define SENSOR_FPS_NIGHT 		12

//#define SENSOR_FPS_NIGHT 		25


//#define YUV_DEBUG   /*save yvv data to /tmp/sample_pic.nv12*/
struct encoder_channel {
	bool alive;
    int enable;
    int enc_grp;             //Encoder group
    int enc_id;            // Encoder channel
    QCamVideoInputChannel ChnCfg;
};


struct ysx_osd {
    int channel;
    int grp;
    pthread_t osd_tid;
    unsigned int enable;
	int font_h;
	int font_w;
    IMPRgnHandle handle;
};


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

enum {
    CURRENT_MODE_NIGHT    = 0x00,
    CURRENT_MODE_DAY      = 0x01,
};

extern uint8_t _loop;
extern int fps_right;

enum{
FPS_50HZ_RIGHT = 0 ,
FPS_60HZ_RIGHT  = 1 ,
};

int QCamSetMainsFrequency(int mode);


int QCamVideoInput_Init();
int QCamVideoInput_AddChannel(QCamVideoInputChannel);
void *ysx_get_stream();
int QCamVideoInput_Start();
int QCamVideoInput_Uninit();
int ysx_ircut_set(QCAM_IR_MODE mode);
int channel_attr_set(int width,int height,QCamVideoInputChannel cn,IMPEncoderChnAttr *enc_chn_attr);
int ysx_jpeg_init();
int system_osd_init();
int ysx_read_config(const char *symbol, int *value);
void * Thread_IR_Auto(void *arg);
void * Thread_FILL_Auto(void *arg);

void * ysx_stream(void *arg);

#define VID_LOG(fmt,args...) do{\
			printf("[%s:%s:%d ] ",__FILE__,__FUNCTION__,__LINE__);\
			printf(fmt, ##args);\
		}while(0)


#endif /*__YSX_VIDEO_H__*/


