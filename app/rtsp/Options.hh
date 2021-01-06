/*
 * Ingenic IMP RTSPServer Option variables, modifiable from the command line.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef _OPTIONS_HH
#define _OPTIONS_HH

#include <RTSPServer.hh>
#include "qcam_video_input.h"
typedef enum RES_TYPE {
	RES_D1 = 0,
	RES_VGA,
	RES_720P,
	RES_HD1,
	RES_CIF,
	RES_QVGA,
} RES_TYPE_t;

typedef enum 
{
	SENSOR_OV9712=0,
	SENSOR_OV9732,
	SENSOR_OV9750,
	SENSOR_JXH42,
	SENSOR_SC1035,
	SENSOR_SC1135,
	SENSOR_SC1045,
	SENSOR_SC1145,
	SENSOR_AR0130,
	SENSOR_JXH61,
	SENSOR_GC1024,
	SENSOR_GC1064,
	SENSOR_GC2023,
	SENSOR_BF3115,
	SENSOR_IMX225,
	SENSOR_OV2710,
	SENSOR_IMX322,
	SENSOR_SC2135,
	SENSOR_SP1409,
	SENSOR_JXH62,
	SENSOR_BG0806,
	SENSOR_OV4689,
	SENSOR_JXF22,
	SENSOR_IMX323,
	SENSOR_IMX291

}sensor_type_t;

extern portNumBits rtspServerPortNum;
extern UserAuthenticationDatabase* authDB;
extern char* streamDescription;
extern int videoBitrate;

extern QCAM_VIDEO_RESOLUTION resolution;
extern int IRMode;
extern int gconf_Inversion;
extern int gconf_FPS_Num;
extern int gconf_FPS_Den;
extern int gconf_EncSaveFPSNum;
extern int gconf_EncSaveFPSDen;
extern int gconf_Main_VideoWidth;
extern int gconf_Main_VideoHeight;
extern int gconf_Main_VideoWidth_ori;
extern int gconf_Main_VideoHeight_ori;
extern int gconf_Second_VideoWidth;
extern int gconf_Second_VideoHeight;
extern int gconf_Second_EncWidth;
extern int gconf_Second_EncHeight;
extern char gconf_snapPath[];
extern bool gconf_cli;
extern sensor_type_t gconf_sensor_type;
extern bool gconf_use_internal_move;
extern int gconf_isp_tuning_thread;

extern void checkArgs(UsageEnvironment& env, int argc, char** argv);
extern int option_save_buf(int fd, void *virAddr, uint32_t size);

#define AFMT_NONE	0
#define AFMT_PCM_RAW16	1
#define AFMT_PCM_ULAW	2
#define AFMT_MPEG2		3
#define AFMT_AMR		4
#define AFMT_AAC		5
#define RECORDPATH		"/mnt/mmc/ingenic.h264"

#define PCM_AUDIO_IS_LITTLE_ENDIAN 0
extern unsigned int audioNumChannels_t;
extern unsigned int audioFormat_t;
extern unsigned int audioSamplingFrequency_t;
extern unsigned int audioOutputBitrate_t;

#endif
