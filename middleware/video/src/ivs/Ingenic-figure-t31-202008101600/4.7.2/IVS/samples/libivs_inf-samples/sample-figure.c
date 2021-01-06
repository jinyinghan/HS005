#include <string.h>
#include <unistd.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_figure.h>
#include <imp/imp_ivs.h>

#include "sample-common.h"

#define TAG "SAMPLE-FIGURE"

int sample_ivs_figure_init(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_CreateGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	return 0;
}


int sample_ivs_figure_exit(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_DestroyGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}

int sample_ivs_figure_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	figure_param_input_t param;

	memset(&param, 0, sizeof(figure_param_input_t));
	param.frameInfo.width = sensor_sub_width;
	param.frameInfo.height = sensor_sub_height;
	*interface = FigureInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
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

int sample_ivs_figure_stop(int chn_num, IMPIVSInterface *interface)
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

	FigureInterfaceExit(interface);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0, i = 0, m = 0;
	IMPIVSInterface *inteface = NULL;
	figure_param_output_t *result = NULL;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0;

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
	ret = sample_ivs_figure_init(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_figure_init(0) failed\n");
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

	ret = sample_ivs_figure_start(0, 0, &inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_figure_start(0, 0) failed\n");
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
		if(result->count > 0) {
			for (m = 0; m < result->count; m++) {
				x0 = result->rects[m].ul.x;
				y0 = result->rects[m].ul.y;
				x1 = result->rects[m].br.x;
				y1 = result->rects[m].br.y;
				//IMP_LOG_DBG(TAG, "(%d,%d,%d,%d)\n", x0, y0, x1, y1);
				printf("rect[%d], (%d,%d,%d,%d)\n", m, x0, y0, x1, y1);
			}
		}
		IMP_LOG_INFO(TAG, "frame[%d], result->ret=%d\n", i, result->count);

		ret = IMP_IVS_ReleaseResult(0, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
			return -1;
		}
	}

	ret = sample_ivs_figure_stop(0, inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_figure_stop(0) failed\n");
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
	ret = sample_ivs_figure_exit(0);
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
