/*************************************************************************
	> File Name: video_input.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月20日 星期四 14时27分02秒
 ************************************************************************/

#include <stdio.h>
#include <errno.h>
#include "ysx_video.h"
#include "video_input.h"
#include "ingenic_video.h"
#include <imp/imp_log.h>


extern struct encoder_channel gEncoderChns[FS_CHN_NUM];
extern struct chn_conf chn[FS_CHN_NUM];
extern bool IROnStatus;
extern unsigned int IR_AUTO_ENABLE;
extern unsigned int IR_Thread;
extern pthread_t ir_tid;
//extern pthread_mutex_t lock;
extern QCAM_IR_MODE last_mode;
pthread_mutex_t lock;
extern volatile bool IRAutoEnable;
extern int exit_ir_thread;
pthread_mutex_t ir_mutex;

static int g_enc_init = 0;
uint8_t _loop;

int fps_right = 1;

// osd相关资源
extern struct ysx_osd time_cfg[2];

int res_framesource_streamoff(int chn_num)
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

int res_framesource_exit(int chn_num, bool is_destroy_channel)
{
	int ret = 0;

	if (is_destroy_channel == true) {
		/*Destroy channel i*/
		ret = IMP_FrameSource_DestroyChn(chn_num);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn_num, ret);
			return -1;
		}
	}
	return 0;
}
int res_framesource_init(int chn_num, IMPFSChnAttr *imp_chn_attr, bool is_create_channel)
{
	int ret = 0;

	if (is_create_channel == true) {
		/* create channel chn_num */
		ret = IMP_FrameSource_CreateChn(chn_num, imp_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error!\n", chn_num);
			goto err_IMP_FrameSource_CreateChn;
		}
	}

	ret = IMP_FrameSource_SetChnAttr(chn_num, imp_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(%d) error: %d\n", ret, chn_num);
		goto err_IMP_FrameSource_SetChnAttr;
	}

	/* Check channel chn_num attr */
	IMPFSChnAttr imp_chn_attr_check;
	ret = IMP_FrameSource_GetChnAttr(chn_num, &imp_chn_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_GetChnAttr(%d) error: %d\n", ret, chn_num);
		goto err_IMP_FrameSource_GetChnAttr;
	}

	return 0;

err_IMP_FrameSource_GetChnAttr:
err_IMP_FrameSource_SetChnAttr:
	if (is_create_channel == true) {
		IMP_FrameSource_DestroyChn(chn_num);
	}
err_IMP_FrameSource_CreateChn:
	return -1;
}

int res_framesource_streamon(int chn_num)
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
int res_encoder_exit(uint32_t grp_num, uint32_t chn_num, bool is_destroy_group)
{
	IMP_Encoder_UnRegisterChn(chn_num);
	IMP_Encoder_DestroyChn(chn_num);
	if (is_destroy_group == true) {
		IMP_Encoder_DestroyGroup(grp_num);
	}
	return 0;
}

static int set_isp_antiflicker_hz(int mode)
{
    int ret = 0;
    IMPISPAntiflickerAttr flick_attr;

    if (mode == 2) {
        flick_attr = IMPISP_ANTIFLICKER_60HZ;
		fps_right =  FPS_60HZ_RIGHT;
    }
    else if (mode == 1){
        flick_attr = IMPISP_ANTIFLICKER_50HZ;
		fps_right =  FPS_50HZ_RIGHT;
    }else if(mode == 0){
		
	    flick_attr = IMPISP_ANTIFLICKER_DISABLE;
	}

	#ifdef USE_REDUCE_FPS_RIPPLE
	if(fps_right == FPS_60HZ_RIGHT)
		ret = IMP_ISP_Tuning_SetSensorFPS(15, 1);
	else if(fps_right == FPS_50HZ_RIGHT)
		ret = IMP_ISP_Tuning_SetSensorFPS(25, 2);
	 if(ret < 0){
	 	  QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
	 	   return 1;
	 }
	#endif
    ret = IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to set antiflicker!\n");
        return 1; 
    }

    ret = IMP_ISP_Tuning_GetAntiFlickerAttr(&flick_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to set antiflicker!\n");
        return 1; 
    }  
    printf("set antiflicker mode: %d\n", flick_attr);
    return 0;
}

int QCamSetMainsFrequency(int mode){
if((mode != 0) && (mode != 1) && (mode != 2)){
  		printf("no support mode for MAINS FREQUENCY\n");
        return 1;	
	}
	return set_isp_antiflicker_hz(mode);

}

int QCamVideoInput_SetResolution (int channel, int resolution)
{
	int picWidth;
	int picHeight;
	int ret ;
	printf("[%s] channel  %d\n",__FUNCTION__,channel);
	//printf("====================== SetResolution start ===============================\n");
	if(channel != 0 && channel != 1){
		printf("not supported channel\n");
		return -1;
	}	
	if(ResolutionCheck_By_Res(resolution,&picWidth,&picHeight) < 0)
		return -1;
	
	//if(resolution == QCAM_VIDEO_RES_720P){
	//	picWidth = 1280;
	//	picHeight = 720;
	//}
	
	ret = VideoInput_Stream_Stop(channel);
	if(ret < 0){
		printf("VideoInput_Stream_Stop failed\n");

	}
	
	IMPEncoderChnAttr channel_attr;
	memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	ret = IMP_Encoder_GetChnAttr(channel,&channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetChnAttr error: %d\n", ret);
		return -1;
	}
	
	printf("%d %d %d %d %d %d\n",channel_attr.encAttr.uWidth,channel_attr.encAttr.uHeight,\
		channel_attr.rcAttr.attrRcMode.rcMode,channel_attr.rcAttr.outFrmRate.frmRateNum,\
		channel_attr.rcAttr.outFrmRate.frmRateDen,channel_attr.gopAttr.uGopLength);
	
	res_encoder_exit(0, channel, false);
	
	


	ret = reset_encode_res(channel,picWidth,picHeight,&channel_attr);
	if(ret < 0){
		printf("reset_encode_res failed\n");

	}
	ret = VideoInput_Start_Reset(channel);
	if(ret < 0){
		printf("VideoInput_Start_Reset failed\n");

	}
	//printf("====================== SetResolution end ===============================\n");
	return 0;
}
int QCamVideoInput_SetCodec(int channel, int codec)
{
	
	int ret ;
	//printf("====================== SetCodec start ===============================\n");
	if(channel != 0 && channel != 1){
		printf("not supported channel\n");
		return -1;
	}
	if(codec != 0 && codec != 1){
		printf("not supported codec\n");
		return -1;
	}	


	#if 1
	ret = VideoInput_Stream_Stop(channel);
	if(ret < 0){
		printf("VideoInput_Stream_Stop failed\n");

	}
	IMPEncoderChnAttr channel_attr;
	memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	ret = IMP_Encoder_GetChnAttr(channel,&channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetChnAttr error: %d\n", ret);
		return -1;
	}
	
	printf("%d %d %d %d %d %d\n",channel_attr.encAttr.uWidth,channel_attr.encAttr.uHeight,\
		channel_attr.rcAttr.attrRcMode.rcMode,channel_attr.rcAttr.outFrmRate.frmRateNum,\
		channel_attr.rcAttr.outFrmRate.frmRateDen,channel_attr.gopAttr.uGopLength);
	res_encoder_exit(0, channel, false);
	
	
	#endif

	ret = reset_encode_codec(channel,codec,&channel_attr);
	if(ret < 0){
		printf("reset_encode_codec failed\n");

	}
	set_payload(channel,codec);
	ret = VideoInput_Start_Reset(channel);
	if(ret < 0){
		printf("VideoInput_Start_Reset failed\n");

	}
	//printf("====================== SetCodec end ===============================\n");
	return 0;




}
//#define LIB_VID_VERSION  "0.01.02.01_201030"
void set_ivs_channel_depth(int channel,int depth){

	IMP_FrameSource_SetFrameDepth(channel, depth);//360方案不用移动追踪，一直需要yuv数据，而且人像识别的yuv也一直在取流，所以这个一直保持为1
	return ;
}
int QCamVideoInput_Init()
{
	int ret,i;
	//return;
	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}
	//set_isp_antiflicker_hz(mode);
	//printf("step 00xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");	
	IMPVersion ver;
	IMP_System_GetVersion(&ver);
	VID_LOG("CPU info : %s , IMP version : %s\n",IMP_System_GetCPUInfo(),ver.aVersion);
	_loop = 1;

	/* Step.2 FrameSource init */
	//ret = sample_framesource_ext_hsv_init();
	//if (ret < 0) {
	//	IMP_LOG_ERR(TAG, "FrameSource init ext hsv failed\n");
	//	return -1;
	//}
	
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}
	///IMP_FrameSource_SetSource(1, 1); 
	
	#ifdef USE_MOTION
	if (System_IVS_Init() != 0) {
		return -1;
    }
	#endif
	if (system_osd_init() != 0) {
		return -1;
	}

	/* Step.3 Encoder init */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}
	#if 0
	ret = sample_encoder_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}
	#endif
	
	ret = sample_jpeg_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Jpeg init failed\n");
		return -1;
	}
	
	/* Step.4 Bind */
    ret = system_module_bind();
    if (ret < 0) {
		IMP_LOG_ERR(TAG, "System_Module_Bind failed\n");
		return -1;
	}
	
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}
    
	for (i = 0; i < FS_CHN_NUM; i++) {
		memset(gEncoderChns+i,0,sizeof(struct encoder_channel));
		gEncoderChns[i].enable = 0;
	}
	
	set_ivs_channel_depth(1,1);
	
	
	VID_LOG("Video Init OK\n");
	#ifdef USE_IRCUT 
	ret = ipc_ircut_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ipc_ircut_init failed\n");
		return -1;
	}
    #endif
	QCamSetIRCut(0);
	/*
    ret = pthread_mutex_init(&ir_mutex, NULL);
    if (ret < 0) {
        ret = errno > 0 ? errno : -1;
        printf("mutex init failed with: %s\n", strerror(ret));
        return -1;
    }
	*/
	#ifdef USE_VERITY
	int status = ysx_verify();
	if(status){
		printf("decrypt error\n");
		not_match_key_drawrect(1);
	}else{
		printf("verify success\n");

	}
	#endif
	return 0;
}

extern int t_payload[FS_CHN_NUM] ;
//IMPEncoderChnAttr channel_attr_copy;
#define LOW_BITSTREAM

int EncoderChannelAdd(QCamVideoInputChannel channel){
	int i, ret, ch = 0, chnNum = 0;
	//IMPFSChnAttr *imp_chn_attr_tmp;
	if(ch != 0 && ch != 1 && ch != 2 && ch != 3){
		printf("not recognized channel num\n");
		return -1 ;
	}
	printf("%d %d %d \n",channel.bitrate,channel.fps,channel.gop);
	ch = channel.channelId;
	IMPEncoderChnAttr channel_attr;
	int picWidth , picHeight;
	/*
	if(channel.payloadType != IMP_ENC_PROFILE_AVC_BASELINE && channel.payloadType != IMP_ENC_PROFILE_AVC_MAIN && \
			channel.payloadType != IMP_ENC_PROFILE_AVC_HIGH && channel.payloadType != IMP_ENC_PROFILE_HEVC_MAIN){
		printf("error payloadType for encoder!!!!\n");
		return -1;

	}*/
	IMPEncoderProfile payloadType;
	IMPEncoderRcMode S_RC_METHOD;
	
	switch(channel.encoding)
	{
		case 0:
			payloadType = IMP_ENC_PROFILE_AVC_HIGH;break;
		case 1:
			payloadType = IMP_ENC_PROFILE_HEVC_MAIN;break;
		default:
			VID_LOG("invalid payload type[%d]\n",channel.encoding);
			return -1;
	}
	switch(channel.vbr)
	{
		case 0:
			S_RC_METHOD = IMP_ENC_RC_MODE_CBR;break;
		case 1:
			S_RC_METHOD = IMP_ENC_RC_MODE_VBR;
			#define USE_IMP_ENC_RC_MODE_CAPPED_VBR
			#ifdef USE_IMP_ENC_RC_MODE_CAPPED_VBR
			S_RC_METHOD = IMP_ENC_RC_MODE_CAPPED_VBR;
			#endif
			break;
		default:
			VID_LOG("invalid enc mode[%d]\n",channel.vbr);
			return -1;
	}
	     //S_RC_METHOD = IMP_ENC_RC_MODE_CAPPED_QUALITY;
	      // if (chn[ch].enable) {
            //imp_chn_attr_tmp = &chn[ch].fs_chn_attr;
            //chnNum = chn[ch].index;
			t_payload[ch] = channel.encoding;
            memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
		    #if 1
			if(ResolutionCheck(channel,&picWidth,&picHeight) < 0)
				return -1;
			
			int uTargetBitRate = channel.bitrate * 1;///BITRATE_720P_Kbs * ratio;///
			if(uTargetBitRate <= 0 || uTargetBitRate >= 4000)
				uTargetBitRate = 1000;
			int fps = channel.fps;
			if(fps <= 0 || fps > 20)
				fps = 15;
			int gop = channel.gop;
			if(gop < 1 || gop >20)
				gop = 2 ;
			//if(S_RC_METHOD == IMP_ENC_RC_MODE_CBR)
			//	uTargetBitRate = 
			//chnNum = chn.channelId;
			int uTargetBitRate2 = uTargetBitRate;
			if(S_RC_METHOD == IMP_ENC_RC_MODE_CAPPED_VBR)
				uTargetBitRate2 = uTargetBitRate*3/4;
			printf("%d %d %d \n",channel.bitrate,channel.fps,channel.gop);
            ret = IMP_Encoder_SetDefaultParam(&channel_attr, payloadType, S_RC_METHOD,
                    picWidth, picHeight,
                    fps, 1,
                    (fps*gop) , 1,
                    (S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
                    uTargetBitRate2);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
                return -1;
            }
			#else
			float ratio = 1;
			if (((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) > (1280 * 720)) {
				ratio = log10f(((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720.0)) + 1;
			} else {
				ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight)) + 1);
			}
            ratio = ratio > 0.1 ? ratio : 0.1;
			unsigned int uTargetBitRate = BITRATE_720P_Kbs * ratio;///channel.bitrate * ratio;
			ret = IMP_Encoder_SetDefaultParam(&channel_attr, chn[ch].payloadType, S_RC_METHOD,
                    imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
                    channel.fps, 1,
                    channel.fps* 2 / 1, 2,
                    (S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
                    uTargetBitRate);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
                return -1;
            }
			#endif
#ifdef LOW_BITSTREAM

			IMPEncoderRcAttr *rcAttr = &channel_attr.rcAttr;
			//uTargetBitRate /= 2;
			

			switch (rcAttr->attrRcMode.rcMode) {
				case IMP_ENC_RC_MODE_FIXQP:
					rcAttr->attrRcMode.attrFixQp.iInitialQP = 38;
					break;
				case IMP_ENC_RC_MODE_CBR:
					/*
					rcAttr->attrRcMode.attrCbr.uTargetBitRate = uTargetBitRate*3/4;
					rcAttr->attrRcMode.attrCbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrCbr.iMinQP = 30;
					rcAttr->attrRcMode.attrCbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrCbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrCbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCbr.uMaxPictureSize = uTargetBitRate ;
					*/
					break;
				case IMP_ENC_RC_MODE_VBR:
					rcAttr->attrRcMode.attrVbr.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrVbr.iMinQP = 34;
					rcAttr->attrRcMode.attrVbr.iMaxQP = 51;
					rcAttr->attrRcMode.attrVbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrVbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
					break;
				case IMP_ENC_RC_MODE_CAPPED_VBR:
					rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = uTargetBitRate*3/4;
					rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = uTargetBitRate ;
					rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
					rcAttr->attrRcMode.attrCappedVbr.iMinQP = 30;
					if(uTargetBitRate == 2048)
					rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 58;	
					else if(uTargetBitRate == 1536)
					rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 54;	
					else
					rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 52;
					rcAttr->attrRcMode.attrCappedVbr.iIPDelta = -1;
					rcAttr->attrRcMode.attrCappedVbr.iPBDelta = -1;
					rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = uTargetBitRate ;
					rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
					break;
				case IMP_ENC_RC_MODE_CAPPED_QUALITY:
					rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = uTargetBitRate;
					rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
					rcAttr->attrRcMode.attrCappedQuality.iMinQP = 34;
					rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 51;
					rcAttr->attrRcMode.attrCappedQuality.iIPDelta = -1;
					rcAttr->attrRcMode.attrCappedQuality.iPBDelta = -1;
					rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
					rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = uTargetBitRate * 4 / 3;
					rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
					break;
				case IMP_ENC_RC_MODE_INVALID:
					IMP_LOG_ERR(TAG, "unsupported rcmode:%d, we only support fixqp, cbr vbr and capped vbr\n", rcAttr->attrRcMode.rcMode);
					return -1;
			}
#endif
		//	memset(&channel_attr_copy,0,sizeof(channel_attr_copy));
		//	memcpy(&channel_attr_copy,&channel_attr,sizeof(channel_attr));
			
            ret = IMP_Encoder_CreateChn(ch, &channel_attr);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", ch);
                return -1;
            }
			
			ret = IMP_Encoder_RegisterChn(ch,ch);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", ch, ch, ret);
				return -1;
			}
			memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
			ret = IMP_Encoder_GetChnAttr(ch,&channel_attr);
				if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetChnAttr error: %d\n", ret);
				return -1;
			}
			printf("%d %d %d %d %d %d\n",channel_attr.encAttr.uWidth,channel_attr.encAttr.uHeight,\
				channel_attr.rcAttr.attrRcMode.rcMode,channel_attr.rcAttr.outFrmRate.frmRateNum,\
				channel_attr.rcAttr.outFrmRate.frmRateDen,channel_attr.gopAttr.uGopLength);
		//}
			return 0;
}

int reset_encode_res(int ch,int wid,int hig,IMPEncoderChnAttr*channel_attr)
{

	int ret = -1;
	//channel_attr_copy.encAttr.uWidth = wid;
	//channel_attr_copy.encAttr.uHeight= hig;

	
	channel_attr->encAttr.uWidth = wid;
	channel_attr->encAttr.uHeight= hig;
	
	//ret = IMP_Encoder_CreateChn(ch, &channel_attr_copy);
	ret = IMP_Encoder_CreateChn(ch, channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", ch);
		return -1;
	}
	//printf("step1================\n");
	ret = IMP_Encoder_RegisterChn(ch,ch);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", ch, ch, ret);
		return -1;
	}
	//printf("step2================\n");
	//memset(&channel_attr_copy, 0, sizeof(IMPEncoderChnAttr));
	//memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	ret = IMP_Encoder_GetChnAttr(ch,channel_attr);
		if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetChnAttr error: %d\n", ret);
		return -1;
	}
	printf("%d %d %d %d %d %d\n",channel_attr->encAttr.uWidth,channel_attr->encAttr.uHeight,\
		channel_attr->rcAttr.attrRcMode.rcMode,channel_attr->rcAttr.outFrmRate.frmRateNum,\
		channel_attr->rcAttr.outFrmRate.frmRateDen,channel_attr->gopAttr.uGopLength);

}

int reset_encode_codec(int ch,int codec,IMPEncoderChnAttr*channel_attr)
{

	int ret = -1;
	
	//if(codec == 0)
	//	channel_attr_copy.encAttr.eProfile = IMP_ENC_PROFILE_AVC_HIGH;
	//else 
	//	channel_attr_copy.encAttr.eProfile =  IMP_ENC_PROFILE_HEVC_MAIN;
	if(codec == 0)
		channel_attr->encAttr.eProfile = IMP_ENC_PROFILE_AVC_HIGH;
	else 
		channel_attr->encAttr.eProfile =  IMP_ENC_PROFILE_HEVC_MAIN;
	//ret = IMP_Encoder_CreateChn(ch, &channel_attr_copy);
	ret = IMP_Encoder_CreateChn(ch, channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", ch);
		return -1;
	}
	//printf("step1================\n");
	ret = IMP_Encoder_RegisterChn(ch,ch);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", ch, ch, ret);
		return -1;
	}
	//printf("step2================\n");
	//memset(&channel_attr_copy, 0, sizeof(IMPEncoderChnAttr));
	//memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	ret = IMP_Encoder_GetChnAttr(ch,channel_attr);
		if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetChnAttr error: %d\n", ret);
		return -1;
	}
	printf("%d %d %d %d %d %d\n",channel_attr->encAttr.uWidth,channel_attr->encAttr.uHeight,\
		channel_attr->rcAttr.attrRcMode.rcMode,channel_attr->rcAttr.outFrmRate.frmRateNum,\
		channel_attr->rcAttr.outFrmRate.frmRateDen,channel_attr->gopAttr.uGopLength);

}


int QCamVideoInput_AddChannel(QCamVideoInputChannel cn)
{
	int channel = cn.channelId;
	if(channel > FS_CHN_NUM){
		VID_LOG("invalid channel %d",channel);
		return -1;
	}

	VID_LOG("Add channel %d\n",channel);
	if(EncoderChannelAdd(cn) < 0)
	{
		VID_LOG("Encoder channel add error\n");
		return -1;
	}
	

	memcpy(&(gEncoderChns[channel].ChnCfg), &cn , sizeof(QCamVideoInputChannel));
	gEncoderChns[channel].alive   = true;
	gEncoderChns[channel].enable  = 1;
	gEncoderChns[channel].enc_id  = channel;
	gEncoderChns[channel].enc_grp = channel;

	int i;
	for(i=0;i<FS_CHN_NUM;i++){
		printf("FS_CHN_NUM:%d\n",FS_CHN_NUM);
		QLOG(FATAL, "channel %d enable=%d, enc_id=%d, enc_grp=%d\n",
				i,
				gEncoderChns[i].enable,
				gEncoderChns[i].enc_id,
				gEncoderChns[i].enc_grp);
	}

    g_enc_init = 1;
	return 0;    
}

pthread_t stream_tid[FS_CHN_NUM] = {0};

int QCamVideoInput_Start()
{
	/* Step.5 Stream On */
	int ret;

	int i = 0;
	for (; i < FS_CHN_NUM; i++){	
		//	for (; i < 2; i++){	
		if (gEncoderChns[i].enable){
			QLOG(TRACE,"Start recv stream [%d]",gEncoderChns[i].enc_id);
			ret = pthread_create(&stream_tid[i], NULL, ysx_stream, &(gEncoderChns[i].enc_id));
			if (ret < 0) {
				QLOG(FATAL, "Failed Create pthread to ysx_stream with result %d\n",ret);
				return -1;
			}		   
		}
	}

    EMGCY_LOG("@@@ start to set ircut...\n");
	usleep(800 * 1000);       //wait for AWB startup
	//QCamSetIRCut(0);
/*
#ifdef USE_IRCUT 
   	QCamSetIRMode(QCAM_IR_MODE_AUTO);   // set ir auto
    if (QCAM_IR_MODE_AUTO != last_mode) {
        QCamSetIRMode(last_mode);
    }
    else{
        ysx_ircut_set(QCAM_IR_MODE_OFF);
    }
#endif
*/
/*
#ifdef USE_FILL_LIGHT
	//QCamSetIRMode(QCAM_IR_MODE_AUTO); 
	QCamSetFillAuto();
#endif
*/
    exit_ir_thread = 0;

	return 0;
}

int VideoInput_Start_Reset(int channel)
{
	int ret = -1;
	gEncoderChns[channel].enable = 1;
	if (gEncoderChns[channel].enable){
			QLOG(TRACE,"Start recv stream [%d]",gEncoderChns[channel].enc_id);
			ret = pthread_create(&stream_tid[channel], NULL, ysx_stream, &(gEncoderChns[channel].enc_id));
			if (ret < 0) {
				QLOG(FATAL, "Failed Create pthread to ysx_stream with result %d\n",ret);
				return -1;
			}		   
		}



}

int VideoInput_Stream_Stop(int channel)
{
	gEncoderChns[channel].enable = 0;
    pthread_join(stream_tid[channel], NULL);


}
int QCamVideoInput_CatchYUV(int w, int h, char *buf, int bufLen)
{
	int ret;
	IMPFrameInfo *frame;
	
	if(!buf || bufLen < 10){
		VID_LOG("invalid: buf is %p , len is %d\n",buf,bufLen);
		return -1;
	}
	static int pic_num = 0;
	//pthread_mutex_lock(&frame_mutex);

	ret = IMP_FrameSource_GetFrame(YUV_CHN, &frame);
	if (ret < 0) {
		QLOG(FATAL, "%s(%d):IMP_FrameSource_GetFrame failed", __func__, __LINE__);
		//pthread_mutex_unlock(&frame_mutex);
		return -1;
	}
	
	//printf("snap num :%d\n",pic_num++);
	
	#ifdef TEST_YUV_PIC
	FILE *fp;
	fp = fopen("/tmp/snap.yuv", "wb");
	fwrite((void *)frame->virAddr, frame->size, 1, fp);
	fclose(fp);
	#endif
	if(bufLen < frame->size)
		memcpy(buf,(void *)frame->virAddr,bufLen);
	else if(bufLen >= frame->size) 
		memcpy(buf,(void *)frame->virAddr,frame->size);
	IMP_FrameSource_ReleaseFrame(YUV_CHN, frame);
	if (ret < 0) {
		QLOG(FATAL, "%s(%d):IMP_FrameSource_ReleaseFrame failed", __func__, __LINE__);
		//pthread_mutex_unlock(&frame_mutex);
		return -1;
	}
	
	//pthread_mutex_unlock(&frame_mutex);
	return 0;
}



int QCamSetIRCut(int ir)
{
	IMPISPRunningMode pmode;
	switch(ir){
		case 0:
				
				IMP_ISP_Tuning_GetISPRunningMode(&pmode);
				if (pmode!=IMPISP_RUNNING_MODE_DAY) 
					IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
		
				ysx_ircut_set2(ir);
				break;
		case 1:
				//IMPISPRunningMode pmode;
				IMP_ISP_Tuning_GetISPRunningMode(&pmode);
				if (pmode!=IMPISP_RUNNING_MODE_NIGHT) 
					IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_NIGHT);
		
				ysx_ircut_set2(ir);
				break;
		default:
				break;
	}
	//ircut_status = ir;
	
	return 0;


}
int QCamSuspendLightDetect(int suspend)
{
	if(suspend != 0 && suspend != 1){
		return -1;
	}
	suspend_light_set(suspend);
	return 0;
}


void QCamSetIRMode(QCAM_IR_MODE mode)
{
	
    int ret;

	EMGCY_LOG("--> %d\n", __LINE__);
    QLOG(TRACE,"set ir mode = %d", mode);
 	ipc_ircut_mode_set(mode);
 	
/*
    switch(mode) 
    {   
        case QCAM_IR_MODE_AUTO:			
            if (IR_Thread == 0) {  
                IR_Thread = 1;
                ret = pthread_create(&ir_tid, NULL, Thread_IR_Auto, NULL);
                if (ret < 0) {
                    QLOG(FATAL, "Failed Create pthread to Thread_IR_Auto with result %d\n",ret);
                    return ;
                }         
            }
            pthread_mutex_lock(&ir_mutex);	
            IRAutoEnable = true;
	        pthread_mutex_unlock(&ir_mutex);		
            return ;
            
        case QCAM_IR_MODE_ON:
	        pthread_mutex_lock(&ir_mutex);		              
            IRAutoEnable = false;
			ysx_ircut_set(mode);
 			pthread_mutex_unlock(&ir_mutex); 
            break;
            
        case QCAM_IR_MODE_OFF:		
	        pthread_mutex_lock(&ir_mutex);		               
            IRAutoEnable = false;
			ysx_ircut_set(mode);
 			pthread_mutex_unlock(&ir_mutex);
            break;

        //case QCAM_IR_MODE_NORMAE:
        //   break;

        //case QCAM_IR_MODE_SMART:
         //   break;
            
        default:
            break;
    }
*/  
    last_mode = mode;
}

int QCamVideoInput_SetBitrate(int channel, int bitrate, int isVBR)
{
	printf("[%s] isVBR  %d\n",__FUNCTION__,isVBR);
	IMPEncoderAttrRcMode rcAttr;

	if(channel > FS_CHN_NUM){
		QLOG(TRACE,"Invalid channel %d\n",channel);
		return -1;
	}

	if(gEncoderChns[channel].alive == false)
	{
		VID_LOG("channel %d is not added\n",channel);
		return -1;
	}

	QLOG(TRACE,"set bitrate = %d",bitrate);

	memset(&rcAttr,0,sizeof(IMPEncoderAttrRcMode));

	if (IMP_Encoder_GetChnAttrRcMode(channel,&rcAttr) != 0) 
	{
		QLOG(FATAL, "IMP_Encoder_GetChnRcAttr Error");
		return -1;
	}  
	#if 1
	//rcAttr.attrH264Smart.maxBitRate = bitrate;
	//rcAttr.attrCbr.uTargetBitRate = bitrate;
	if(isVBR == 0){
		//rcAttr.rcMode = IMP_ENC_RC_MODE_CBR;
		//rcAttr.attrCbr.uTargetBitRate = bitrate*3/4;
		rcAttr.attrCbr.uMaxPictureSize = bitrate;
	}else{
		//rcAttr.rcMode = IMP_ENC_RC_MODE_VBR;
		rcAttr.attrCappedVbr.uTargetBitRate = bitrate*3/4;
		rcAttr.attrCappedVbr.uMaxBitRate = bitrate;
		rcAttr.attrCappedVbr.uMaxPictureSize = bitrate;
		if(bitrate == 2048)
		rcAttr.attrCappedVbr.iMaxQP = 58;
		else if(bitrate == 1536)
		rcAttr.attrCappedVbr.iMaxQP = 54;
		else
		rcAttr.attrCappedVbr.iMaxQP = 52;	
	}
	#endif
	//rcAttr.attrCappedVbr.uTargetBitRate = bitrate;
	if (IMP_Encoder_SetChnAttrRcMode(channel,&rcAttr) != 0) 
	{
		QLOG(FATAL, "IMP_Encoder_SetChnRcAttr Error");
		return -1;
	} 

	QLOG(TRACE, "QCamVideoInput_SetBitrate Done");		 
	return 0;
}

typedef enum {
    VAVA_MIRROR_TYPE_NORMAL             = 0,    // 不翻转
    VAVA_MIRROR_TYPE_HORIZONTALLY       = 1,    // 水平翻转
    VAVA_MIRROR_TYPE_VERTICALLY         = 2,    // 垂直翻转
    VAVA_MIRROR_TYPE_BOTH               = 3     // 双向都翻转
} VAVA_MIRROR_TYPE;

int QCamVideoInput_SetInversion(int enable)
{
    
    switch(enable)
    {
        case VAVA_MIRROR_TYPE_NORMAL:
            IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_DISABLE);
            IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_DISABLE);        
        break;
        case VAVA_MIRROR_TYPE_HORIZONTALLY:
            IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_ENABLE);
            IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_DISABLE);           
        break;
        case VAVA_MIRROR_TYPE_VERTICALLY:
            IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_DISABLE);
            IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_ENABLE);
        break;
        case VAVA_MIRROR_TYPE_BOTH:
            IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_ENABLE);
            IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_ENABLE);
        break;
        default:
        break;
    }

    QLOG(TRACE, "QCamVideoInput_SetInversion Done");
    return 0;
}



int QCamVideoInput_SetIFrame(int channel)
{
	if(channel > FS_CHN_NUM){
		QLOG(TRACE,"Invalid channel %d\n",channel);
		return -1;
	}

	if(gEncoderChns[channel].alive == false)
	{
		VID_LOG("channel %d is not added\n",channel);
		return -1;
	}    

	if (IMP_Encoder_RequestIDR(channel) != 0) 
	{
		QLOG(FATAL, "IMP_Encoder_RequestIDR Error");
		return -1;
	} 
	//printf("RequestIDR success\n");
	return 0;
}

//int initialize_encode


int QCamVideoInput_CatchJpeg(char *buf, int *bufLen)
{
	int ret,i;
	unsigned int len;	 

	if(!buf){
		QLOG(FATAL, "invalid buffer");
		return -1;
	}

	ret = IMP_Encoder_StartRecvPic(JPG_CHN);
	if (ret < 0) {
		QLOG(FATAL, "IMP_Encoder_StartRecvPic(%d) failed\n", JPG_CHN);
		return -1;
	}

	/* Polling JPEG Snap, set timeout as 1000msec */
	ret = IMP_Encoder_PollingStream(JPG_CHN, 1000);
	if (ret < 0) {
		QLOG(FATAL,"Polling stream timeout\n");
		return -1;
	}

	IMPEncoderStream stream;
	/* Get JPEG Snap */
	ret = IMP_Encoder_GetStream(JPG_CHN, &stream, 1);
	if (ret < 0) {
		QLOG(FATAL, "IMP_Encoder_GetStream() failed\n");
		return -1;
	}

	len = 0;
	for(i=0;i<stream.packCount;i++)
	{
		len += stream.pack[i].length;
	}

	if(len > *bufLen)
	{
		QLOG(FATAL,"buffer len %d is too samall\n",*bufLen);
		ret = -1;
	}
	else
	{
		len = 0;
		for(i=0;i<stream.packCount;i++)
		{
			memcpy(buf+len,(void *)stream./*pack[i].*/virAddr,stream.pack[i].length);
			len += stream.pack[i].length;
		}	 
		*bufLen = len;
		ret = 0;
	}

	IMP_Encoder_ReleaseStream(JPG_CHN, &stream);

	IMP_Encoder_StopRecvPic(JPG_CHN);


	return ret;

}



int QCamVideoInput_HasLight()
{
	return get_has_light_status();
}

QCam_Light_Detect_cb cbx = NULL;
static int hasLight_exit = 0;
void * ysx_has_light(void *arg){
	  prctl(PR_SET_NAME,"ysx_has_light");
	  static int lst_light = 0;
	  //QCamSetIRCut(0);
	  while(!hasLight_exit){
		int has_light = get_has_light_status();
		if(has_light != lst_light){
			cbx(has_light);
			lst_light = has_light;
		}
		sleep(1);
	  }

	  pthread_exit(0);
}



int QCamSetLightDetectCallback(QCam_Light_Detect_cb cb)
{
	int ret;
	/*
	int has_light = get_has_light_status();
	if(cb){
		cb(has_light);

	}
	*/
	pthread_t has_light_thread_id;
	cbx = cb;
	ret = pthread_create(&has_light_thread_id, NULL, ysx_has_light, NULL);
    if (ret < 0) {
        QLOG(FATAL,"Create pthread ysx_has_light Failed!\n");
        return  -1;
    }
    
	return 0;
}

int QCamVideoInput_SetFps(int channel, int fps, int gop)
{
	printf("[%s] channel  %d\n",__FUNCTION__,channel);	
	if(channel > FS_CHN_NUM || channel < 0){
		QLOG(TRACE,"Invalid channel %d\n",channel);
		return -1;
	}
	
	
		
	if(gEncoderChns[channel].alive == false)
	{
		VID_LOG("channel %d is not added\n",channel);
		return -1;
	}  
	
	if(fps >= SENSOR_FRAME_RATE_NUM || fps <= 0){
			printf("max isp fps is %d\n",SENSOR_FRAME_RATE_NUM);
			return -1;
	
	}else{
		IMPEncoderFrmRate tmp_rate;
		tmp_rate.frmRateNum = fps;
		tmp_rate.frmRateDen = 1;
		IMP_Encoder_SetChnFrmRate(channel,&tmp_rate);
	}
	
	if(gop >= 10 || gop <= 0){
		printf("gop is invalid %d\n",gop);
		return -1;
		
	}else{
		IMP_Encoder_SetChnGopLength(channel,gop*fps);
	}
	
	
}


int QCamVideoInput_Uninit()
{
    int ret = 0;
    int index;
    
    QLOG(FATAL, "video source uninit...\n");
	
	set_ivs_channel_depth(1,0);
	
	#ifdef USE_VERITY
	draw_rect_osd_exit(0);	
	draw_rect_osd_exit(1);	
	#endif
    // 释放ircut控制线程
    hasLight_exit = 1;
    ipc_ircut_exit();
    exit_ir_thread = 1;
	_loop = 0;
//    pthread_join(ir_tid, NULL);

    // 释放osd资源
    for (index = 0; index < FS_CHN_NUM; index ++) {
        if (time_cfg[index].enable) {
            time_cfg[index].enable = 0;
            pthread_join(time_cfg[index].osd_tid, NULL);  
            QLOG(FATAL, "release osd channel %02d source...\n", index);
        }
    }
	QLOG(FATAL, "Release OSD pthread\n"); 

    //QCamUninitMotionDetect();
	QLOG(FATAL, "Release IVS pthread\n");
    // 二维码扫描,没有初始化编码器资源,反初始化会报错
    if (g_enc_init) {
        index = MAX_FS - 1;
        for (index; index >= 0; index --) {
            gEncoderChns[index].enable = 0;
            pthread_join(stream_tid[index], NULL);
        }
    }
	QLOG(FATAL, "Release Stream pthread\n"); 

	/* Step.12 Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}
	QLOG(FATAL, "sample_framesource_streamoff\n"); 	

    ret = system_module_unbind();
    if (ret < 0) {
        printf("system module unbind failed!\n");
        return -1;
    }
	QLOG(FATAL, "system_module_unbind\n"); 	
  
    // 二维码扫描,没有初始化编码器资源,反初始化会报错
    if (g_enc_init) {    
        fprintf(stderr, "sample_encoder_exit\n");
        ret = sample_encoder_exit();
    	if (ret < 0) {
    		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
    		return -1;
    	}
        g_enc_init = 0;
    }
	QLOG(FATAL, "sample_encoder_exit\n"); 	

    ret = system_osd_uninit();
    if (ret < 0) {
        QLOG(FATAL, "IMP_IVS_DestroyGroup failed\n"); 
        return -1;
    }
	QLOG(FATAL, "system_osd_uninit\n");

	#ifdef USE_MOTION
    ret = system_ivs_uninit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "ivs uninit failed!\n");
        return -1;
    }
	QLOG(FATAL, "system_ivs_uninit\n");
	#endif
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}
	QLOG(FATAL, "sample_framesource_exit\n");

	//ret = sample_framesource_ext_hsv_exit();
	//if (ret < 0) {
	//	IMP_LOG_ERR(TAG, "FrameSource exit hsv exit failed\n");
	//	return -1;
	//}
	//QLOG(FATAL, "sample_framesource_ext_hsv_exit\n");
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}
	//pthread_mutex_destroy(&frame_mutex);
	QLOG(FATAL, "sample_system_exit\n");
	
    EMGCY_LOG("video source deinit...\n");
	return 0;
}
/*
int video_input_start(video_input_context * vc)
{
    int ret = 0;
    vc -> started = 0;
    vc -> destroyed = 0;
    ret = pthread_create(&vc -> thread_id, NULL, (void *)get_video_stream, (void *)vc);
    if (ret < 0) {
        ret = errno > 0 ? errno : -1;
        fprintf("create thread failed with: %s!\n", strerror(ret));
        return -1;
    }  
    return 0;
}
*/

int QCamGetSensorType(char * sensor)
{
  int len = strlen(sensor);
  //printf("%d\n",len);
  if(sensor == NULL || len < strlen(SENSOR_NAME)-1){
	//printf("buffer to small\n");
	return -1;
  }
  strcpy(sensor,SENSOR_NAME);
  return 0;

}

#ifdef USE_IVS_FREE
#include <qcam_smart.h>

smart_init_t smt;
IMPIVSInterface *intefacex = NULL;

int smt_exit = 0;
void* smt_proc(void*arg){
	QCamSmartTarget  pTarget;
	memset(&pTarget,0,sizeof(pTarget));
	while(!smt_exit){
		
		QCamSmartGetTarget(&pTarget);
		QCamDrawRect(pTarget.pd_rect,pTarget.pd_target_num);



		usleep(100*1000);
	}




}
int create_smt_draw()
{
	pthread_t smt_id;
	int ret = pthread_create(&smt_id, NULL,smt_proc, NULL);
    if (ret < 0) {
    	printf("create thread failed!\n");
        return -1;
    }
	pthread_detach(smt_id);
	

}

void destroy_smt_draw()
{
	smt_exit=1;
}

int QCamSmartCreate(QCamSmartInParam *pInParam)
{
	//return 0;
	int ret = -1,index = 0;
	/*
	ret = sample_ivs_figure_init(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_figure_init(0) failed\n");
		return -1;
	}
	*/
	//return;

	memset(&smt,0,sizeof(smt));
	//smt.figure_exit = 0;
	pthread_mutex_init(&smt.smart_lock,NULL);
	//pthread_mutex_init(&smt.frame_lock,NULL);
	smt.ivs_enable = 1;
	set_skip_yuvframe(pInParam->smart_enable);
	ret = sample_ivs_figure_start(pInParam->width,pInParam->height,0, 1,&intefacex);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_figure_start(0, 0) failed\n");
		return -1;
	}
	//return  0;
	for (index; index < 2; index ++) {
        set_osd_rect(index);
    }
	//printf("figure_thread start\n");
	//ret = pthread_create(&smt.fig_thread_id, NULL, figure_thread, NULL);
	ret = pthread_create(&smt.fig_thread_id, NULL, figure_thread, (void*)intefacex);
    if (ret < 0) {
        QLOG(FATAL,"Create pthread for motion detecton Failed!\n");
        return -1;
    }
	return 0 ;

}

int QCamSmartDestory()
{
	//return 0;
	int ret = -1;
	smt.figure_exit = 1;

    pthread_join(smt.fig_thread_id, NULL);
	pthread_mutex_destroy(&smt.smart_lock);
	ret = sample_ivs_figure_stop(1, intefacex);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_figure_stop(0) failed\n");
		return -1;
	}
	

	return 0;
}
int QCamSmartGetTarget(QCamSmartTarget *pTarget)
{
		pthread_mutex_lock(&smt.smart_lock);
		int i ;
		pTarget->pd_target_num = smt.ft.pd_num;
		for(i=0;i<smt.ft.pd_num;i++){
			pTarget->pd_rect[i].x1 = (double)smt.ft.pd_rect[i].x1/640;
			pTarget->pd_rect[i].y1 = (double)smt.ft.pd_rect[i].y1/360;
			pTarget->pd_rect[i].x2 = (double)smt.ft.pd_rect[i].x2/640;
			pTarget->pd_rect[i].y2 = (double)smt.ft.pd_rect[i].y2/360;

			//printf(" %d %lf %lf %lf %lf\n",i,pTarget->pd_rect[i].x1,pTarget->pd_rect[i].y1,pTarget->pd_rect[i].x2,pTarget->pd_rect[i].y2);  
		}
		//pTarget->score = smt.ft.score ;
		//printf("target %d\n",pTarget->pd_target_num);
		//printf("%lf %lf %lf %lf\n",pTarget->pd_rect[i].x1,pTarget->pd_rect[i].y1,pTarget->pd_rect[i].x2,pTarget->pd_rect[i].y2);  
		pthread_mutex_unlock(&smt.smart_lock);
	

		return 0 ;
}
#define rshu 5

int QCamDrawRect(QCamSmartRect *pRect, int num)
{
		static int  lastNum = 0;
		int grp_id, i,x0,y0,x1,y1;
		if(pRect == NULL || num == 0){
			for(grp_id=0;grp_id<2;grp_id++){
            	for(i=0;i<rshu;i++)
					resetRectshow(grp_id,i);
			}
			return 0 ;
		}
	
		if(lastNum != num){
			for(grp_id=0;grp_id<2;grp_id++){
            	for(i=0;i<rshu;i++)
					resetRectshow(grp_id,i);
				     

                }
		}
		
		for(grp_id=0;grp_id<2;grp_id++){
        	for (i = 0; i < num; i++) {
            	x0=(int)(pRect[i].x1*640);
                y0=(int)(pRect[i].y1*360);
                x1=(int)(pRect[i].x2*640);
                y1=(int)(pRect[i].y2*360);
				//printf("%lf %lf %lf %lf\n",pRect[i].x1,pRect[i].y1,pRect[i].x2,pRect[i].y2);  
                //printf("%d %d %d %d\n",x0,y0,x1,y1);              
                drawRect_(grp_id,i,x0,y0,x1,y1);
			   
            }
		}

		lastNum  = num;
		return 0;
}

int QCamSmartSetAiAlg(int enable, QCamSmartInParam *pInParam){
	pthread_mutex_lock(&smt.smart_lock);
	smt.ivs_enable = enable;
	pthread_mutex_unlock(&smt.smart_lock);
	return 0;
}

#endif

int QCamSetWdr(int wdr){
	if(wdr != 0 && wdr != 1){
		printf("invalid wdr val\n");
		return -1;
	}
	if(wdr == 0){
		
		IMP_ISP_Tuning_SetDRC_Strength(128);
		//return ;
	}else{
			
		IMP_ISP_Tuning_SetDRC_Strength(135);
	}
		return 0;
}	
