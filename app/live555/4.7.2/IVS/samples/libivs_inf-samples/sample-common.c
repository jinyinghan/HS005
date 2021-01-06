/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_isp.h>
#include <imp/imp_framesource.h>

#include "sample-common.h"

#define TAG "Sample-Common"
 #define SENSOR_JXF23
// #define SENSOR_IMX291
// #define SENSOR_OV2735B
//#define SENSOR_SC2235

IMPSensorInfo sensor_info;

int sensor_main_width = 1280;
int sensor_main_height = 720;
int sensor_sub_width = 640;
int sensor_sub_height = 360;

int sample_system_init()
{
	int ret = 0;

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
#if defined SENSOR_JXH42
	memcpy(sensor_info.name, "jxh42", sizeof("jxh42"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "jxh42", sizeof("jxh42"));
	sensor_info.i2c.addr = 0x30;
    sensor_main_width = 1280;
    sensor_main_height = 720;
#elif defined SENSOR_SC1035
	memcpy(sensor_info.name, "sc1035", sizeof("sc1035"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "sc1035", sizeof("sc1035"));
	sensor_info.i2c.addr = 0x30;
    sensor_main_width = 1280;
    sensor_main_height = 960;
#elif defined SENSOR_OV9750
	memcpy(sensor_info.name, "ov9750", sizeof("ov9750"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "ov9750", sizeof("ov9750"));
	sensor_info.i2c.addr = 0x36;
    sensor_main_width = 1280;
    sensor_main_height = 960;
#elif defined SENSOR_SC2135
	memcpy(sensor_info.name, "sc2135", sizeof("sc2135"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "sc2135", sizeof("sc2135"));
	sensor_info.i2c.addr = 0x30;
    sensor_main_width = 1920;
    sensor_main_height = 1080;
#elif defined SENSOR_SC2235
	memcpy(sensor_info.name, "sc2235", sizeof("sc2235"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "sc2235", sizeof("sc2235"));
	sensor_info.i2c.addr = 0x30;
    sensor_main_width = 1920;
    sensor_main_height = 1080;
#elif defined SENSOR_JXF22
	memcpy(sensor_info.name, "jxf22", sizeof("jxf22"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "jxf22", sizeof("jxf22"));
	sensor_info.i2c.addr = 0x40;
    sensor_main_width = 1920;
    sensor_main_height = 1080;

#elif defined SENSOR_JXF23
	memcpy(sensor_info.name, "jxf23", sizeof("jxf23"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "jxf23", sizeof("jxf23"));
	sensor_info.i2c.addr = 0x40;
    sensor_main_width = 1920;
    sensor_main_height = 1080;

#elif defined SENSOR_OV2710
	memcpy(sensor_info.name, "ov2710", sizeof("ov2710"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "ov2710", sizeof("ov2710"));
	sensor_info.i2c.addr = 0x36;
    sensor_main_width = 1920;
    sensor_main_height = 1080;
#elif defined SENSOR_OV2735
	memcpy(sensor_info.name, "ov2735", sizeof("ov2735"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "ov2735", sizeof("ov2735"));
	sensor_info.i2c.addr = 0x3c;
    sensor_main_width = 1920;
    sensor_main_height = 1080;
#elif defined SENSOR_OV2735B
    memcpy(sensor_info.name, "ov2735b", sizeof("ov2735b"));
    sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
    memcpy(sensor_info.i2c.type, "ov2735b", sizeof("ov2735b"));
    sensor_info.i2c.addr = 0x3c;
    sensor_main_width = 1920;
    sensor_main_height = 1080;
#elif defined SENSOR_IMX291
	memcpy(sensor_info.name, "imx291", sizeof("imx291"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "imx291", sizeof("imx291"));
	sensor_info.i2c.addr = 0x1a;
    sensor_main_width = 1920;
    sensor_main_height = 1080;
#else
	memcpy(sensor_info.name, "ov9712", sizeof("ov9712"));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, "ov9712", sizeof("ov9712"));
	sensor_info.i2c.addr = 0x30;
    sensor_main_width = 1280;
    sensor_main_height = 720;
#endif

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	ret = IMP_ISP_Open();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EmuISPOpen\n");
		return -1;
	}

	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");


	IMP_System_Exit();

	ret = IMP_ISP_DisableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_ISP_DelSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to EmuISPOpen\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

int sample_framesource_streamon(int chn_num)
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_EnableChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn_num);
		return -1;
	}

	return 0;
}

int sample_framesource_streamoff(int chn_num)
{
	int ret = 0;
	/* Enable channels */
	ret = IMP_FrameSource_DisableChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn_num);
		return -1;
	}

	return 0;
}

int sample_framesource_init(int chn_num, IMPFSChnAttr *imp_chn_attr)
{
	int ret = 0;

	/*create channel chn_num*/
	ret = IMP_FrameSource_CreateChn(chn_num, imp_chn_attr);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error!\n", chn_num);
		return -1;
	}

	ret = IMP_FrameSource_SetChnAttr(chn_num, imp_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(%d) error: %d\n", ret, chn_num);
		return -1;
	}

	/* Check channel chn_num attr */
	IMPFSChnAttr imp_chn_attr_check;
	ret = IMP_FrameSource_GetChnAttr(chn_num, &imp_chn_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_GetChnAttr(%d) error: %d\n", ret, chn_num);
		return -1;
	}

	return 0;
}

int sample_framesource_exit(int chn_num)
{
	int ret = 0;

	/*Destroy channel i*/
	ret = IMP_FrameSource_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn_num, ret);
		return -1;
	}
	return 0;
}

