#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_dynamic_tracker.h>
#include <imp/imp_ivs.h>

#include "sample-common.h"

#define TAG "SAMPLE-MOVE"

//#define USE_MOTOR_DRIVER
#ifdef USE_MOTOR_DRIVER
/* ioctl cmd */
#define MOTOR_STOP			0x1
#define MOTOR_RESET			0x2
#define MOTOR_MOVE			0x3
#define MOTOR_GET_STATUS		0x4
#define MOTOR_SPEED			0x5

#define MOTOR_DIRECTIONAL_UP		0x0
#define MOTOR_DIRECTIONAL_DOWN		0x1
#define MOTOR_DIRECTIONAL_LEFT		0x2
#define MOTOR_DIRECTIONAL_RIGHT		0x3

struct motor_move_st {
	int motor_directional;
	int motor_move_steps;
	int motor_move_speed;
};

enum motor_status {
	MOTOR_IS_STOP,
	MOTOR_IS_RUNNING,
};

struct motor_message {
	int x;
	int y;
	enum motor_status status;
	int speed;
};

struct motors_steps{
	int x;
	int y;
};

struct motor_status_st {
	int directional_attr;
	int total_steps;
	int current_steps;
	int min_speed;
	int cur_speed;
	int max_speed;
	int move_is_min;
	int move_is_max;
};

struct motor_reset_data {
	unsigned int x_max_steps;
	unsigned int y_max_steps;
	unsigned int x_cur_step;
	unsigned int y_cur_step;
};

struct motors_steps jb_motors_steps;
struct motor_message jb_motor_message;
struct motor_status_st motor_status;
struct motor_move_st motor_action;
struct motor_reset_data motor_reset_data;

static int trackerFd = -1;

static int setMotorPtz(int dx, int dy)
{
	if(abs(dx) >= 80)
		jb_motors_steps.x = dx;
	else
		jb_motors_steps.x = 0;

	// if(abs(dy) >= 60)
	// 	jb_motors_steps.y = dy*0.8;
	// else
		jb_motors_steps.y = 0;

	if(jb_motors_steps.x != 0)
		ioctl(trackerFd, MOTOR_MOVE, (unsigned long) &jb_motors_steps);
	return 0;
}
#endif

int sample_ivs_dynamic_tracker_init(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_CreateGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	return 0;
}

int sample_ivs_dynamic_tracker_exit(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_DestroyGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}

int sample_ivs_dynamic_tracker_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	dynamic_tracker_param_input_t param;

	memset(&param, 0, sizeof(dynamic_tracker_param_input_t));
	param.frameInfo.width = sensor_sub_width;
	param.frameInfo.height = sensor_sub_height;
    param.obj_min_width = 0;
    param.obj_min_height = 0;
    param.move_sense = 4;
	param.skip_frame_cnt = 0;
	*interface = Dynamic_TrackerInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "Dynamic_TrackerInterfaceInit failed\n");
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

int sample_ivs_dynamic_tracker_stop(int chn_num, IMPIVSInterface *interface)
{
	int ret = 0;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	sleep(1);

	ret = IMP_IVS_UnRegisterChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
		return -1;
	}

	Dynamic_TrackerInterfaceExit(interface);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0, i = 0, j = 0;
	IMPIVSInterface *inteface = NULL;
	dynamic_tracker_param_output_t *result = NULL;
	dynamic_tracker_param_input_t param;

#ifdef USE_MOTOR_DRIVER
      trackerFd= open("/dev/motor", 0);
      if (trackerFd < 0){
	printf("Error /dev/motor!\n");
	exit(-1);
      }
      motor_action.motor_move_speed = 400;
      ioctl(trackerFd, MOTOR_SPEED, (unsigned int) (&motor_action.motor_move_speed));

      struct motor_reset_data rdata;
      memset(&rdata, 0, sizeof(rdata));
      ret = ioctl(trackerFd, MOTOR_RESET, (unsigned int) (&rdata));
      if (ret != 0){
	close(trackerFd);
	exit(-1);
      }
#endif

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/* Step.2 FrameSource init */
	IMPFSChnAttr fs_chn_attr;
	memset(&fs_chn_attr, 0, sizeof(IMPFSChnAttr));
	fs_chn_attr.pixFmt = PIX_FMT_NV12;
	fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE;
	fs_chn_attr.outFrmRateDen = 1;
	fs_chn_attr.nrVBs = 3;
	fs_chn_attr.type = FS_PHY_CHANNEL;

	fs_chn_attr.crop.enable = 0;
	fs_chn_attr.crop.top = 0;
	fs_chn_attr.crop.left = 0;
	fs_chn_attr.crop.width = sensor_main_width;
	fs_chn_attr.crop.height = sensor_main_height;

	fs_chn_attr.scaler.enable = 1;	/* ivs use the second framesource channel, need scale*/
	fs_chn_attr.scaler.outwidth = sensor_sub_width;
	fs_chn_attr.scaler.outheight = sensor_sub_height;

	fs_chn_attr.picWidth = sensor_sub_width;
	fs_chn_attr.picHeight = sensor_sub_height;

	ret = sample_framesource_init(FS_SUB_CHN, &fs_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Encoder init */
	ret = sample_ivs_dynamic_tracker_init(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_init(0) failed\n");
		return -1;
	}

	/* Step.4 Bind */
	IMPCell framesource_cell = {DEV_ID_FS, FS_SUB_CHN, 0};
	IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};

	ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
		return -1;
	}

	/* Step.5 Stream On */
	IMP_FrameSource_SetFrameDepth(FS_SUB_CHN, 0);
	ret = sample_framesource_streamon(FS_SUB_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	ret = sample_ivs_dynamic_tracker_start(0, 0, &inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_start(0, 0) failed\n");
		return -1;
	}

	/* Step.6 Get result */
	for (i = 0; i < NR_FRAMES_TO_IVS; i++) {
		ret = IMP_IVS_PollingResult(0, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			return -1;
		}
		ret = IMP_IVS_GetResult(0, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
			return -1;
		}
        
		printf("frame[%d],result->count=%d,dx=%d,dy=%d,step=%d\n",
                i, result->count, result->dx, result->dy, result->step);
       
		
		for (j = 0; j < result->count; j++) {
            printf("frame[%d], rect[%d]=((%d,%d),(%d,%d))\n", i, j,
                    result->rect[j].ul.x, result->rect[j].ul.y, result->rect[j].br.x, result->rect[j].br.y);
        }
		
		printf("frame[%d],result->count=%d,dx=%d,dy=%d,step=%d\n",
		       i, result->count, result->dx, result->dy, result->step);
	  	
		
		for (j = 0; j < result->count; j++) {
	    	IMP_LOG_INFO(TAG, "frame[%d], rect[%d]=((%d,%d),(%d,%d))\n", i, j,
			 		result->rect[j].ul.x, result->rect[j].ul.y, result->rect[j].br.x, result->rect[j].br.y);
	  	}

		ret = IMP_IVS_GetParam(0, &param);
		if (ret < 0){
			IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", 0);
			return -1;
		}

#ifdef USE_MOTOR_DRIVER
		setMotorPtz(result->dx, result->dy);
		ioctl(trackerFd, MOTOR_GET_STATUS, (unsigned long) &jb_motor_message);
		param.is_motor_stop = ((MOTOR_IS_STOP == jb_motor_message.status)? 1 : 0);
		param.is_feedback_motor_status = 1;
#else
		param.is_motor_stop = 1;
		param.is_feedback_motor_status = 1;
#endif

	  ret = IMP_IVS_SetParam(0, &param);
	  if (ret < 0){
	    IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", 0);
	    return -1;
	  }

	  ret = IMP_IVS_ReleaseResult(0, (void *)result);
	  if (ret < 0) {
	    IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
	    return -1;
	  }
	}

	ret = sample_ivs_dynamic_tracker_stop(0, inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_stop(0) failed\n");
		return -1;
	}

	/* Step.7 Stream Off */
	ret = sample_framesource_streamoff(FS_SUB_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.b UnBind */
	ret = IMP_System_UnBind(&framesource_cell, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
		return -1;
	}

	/* Step.c ivs exit */
	ret = sample_ivs_dynamic_tracker_exit(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs mode exit failed\n");
		return -1;
	}

	/* Step.d FrameSource exit */
	ret = sample_framesource_exit(FS_SUB_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource(%d) exit failed\n", FS_SUB_CHN);
		return -1;
	}

	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}
