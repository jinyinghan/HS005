/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define SENSOR_FRAME_RATE		25
#define NR_FRAMES_TO_IVS		200
#define FS_MAIN_CHN 			0
#define FS_SUB_CHN				1

#define OSD_REGION_WIDTH		32
#define OSD_REGION_HEIGHT		32

extern int sensor_main_width;
extern int sensor_main_height;
extern int sensor_sub_width;
extern int sensor_sub_height;

int sample_system_init();
int sample_system_exit();

int sample_framesource_streamon(int chn_num);
int sample_framesource_streamoff(int chn_num);

int sample_framesource_init(int chn_num, IMPFSChnAttr *imp_chn_attr);
int sample_framesource_exit(int chn_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
