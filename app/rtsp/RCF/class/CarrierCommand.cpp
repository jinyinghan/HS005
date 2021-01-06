#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include <imp/imp_common.h>
#include <imp/imp_isp.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_log.h>
#include <imp/imp_ivs.h>
#include <ivs/ivs_inf_move.h>
#include <imp/imp_ivs_move.h>

#include "CarrierCommand.hh"

#include "../../Options.hh"
#include "../../VideoInput.hh"
#include "../../AudioInput.hh"

using namespace std;

#define TAG		"CarrierCommand"

CarrierCommandImpl::CarrierCommandImpl()
{
}

CarrierCommandImpl::~CarrierCommandImpl()
{
}

int CarrierCommandImpl::WDRSwitch(int on)
{
	IMPISPTuningOpsMode wdr;
	int ret = -1;

	ret = IMP_ISP_Tuning_GetWDRAttr(&wdr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_GetWDRAttr Error" << endl;
		return -1;
	}

	if(on) {
		wdr = IMPISP_TUNING_OPS_MODE_ENABLE;
	} else {
		wdr = IMPISP_TUNING_OPS_MODE_DISABLE;
	}

	ret = IMP_ISP_Tuning_SetWDRAttr(wdr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_SetWDRAttr Error" << endl;
		return -1;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::Denoise3DSwitch(int on)
{
	IMPISPTemperDenoiseAttr dis_3d;
	int ret = -1;

	ret = IMP_ISP_Tuning_GetTemperDnsAttr(&dis_3d);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_GetTemperDnsAttr Error" << endl;
		return -1;
	}

	if(on) {
		dis_3d.type = IMPISP_TEMPER_AUTO;
	} else {
		dis_3d.type = IMPISP_TEMPER_DISABLE;
	}

	ret = IMP_ISP_Tuning_SetTemperDnsAttr(&dis_3d);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_SetTemperDnsAttr Error" << endl;
		return -1;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::IridixSwitch(int on)
{
	IMPISPDrcAttr drc;
	int ret = -1;

	ret = IMP_ISP_Tuning_GetRawDRC(&drc);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_GetRawDRC Error" << endl;
		return -1;
	}

	if(on) {
		drc.mode = IMPISP_DRC_MEDIUM;
	} else {
		drc.mode = IMPISP_DRC_DISABLE;
	}

	ret = IMP_ISP_Tuning_SetRawDRC(&drc);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_SetRawDRC Error" << endl;
		return -1;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::NightModeSwitch(int on)
{
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;

	IMPISPRunningMode mode;
	if (on)
		mode = IMPISP_RUNNING_MODE_NIGHT;
	else
		mode = IMPISP_RUNNING_MODE_DAY;

	IMP_ISP_Tuning_SetISPRunningMode(mode);
	return 0;
}

int CarrierCommandImpl::GetWDRSwitch(int &on)
{
	IMPISPTuningOpsMode wdr;
	int ret = -1;

	ret = IMP_ISP_Tuning_GetWDRAttr(&wdr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_GetWDRAttr Error" << endl;
		return -1;
	}

	if(wdr == IMPISP_TUNING_OPS_MODE_ENABLE) {
		on = 1;
	} else {
		on = 0;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetDenoise3DSwitch(int &on)
{
	IMPISPTemperDenoiseAttr dis_3d;
	int ret = -1;

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << __LINE__ << endl;
	ret = IMP_ISP_Tuning_GetTemperDnsAttr(&dis_3d);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_GetTemperDnsAttr Error" << endl;
		return -1;
	}

	if(dis_3d.type == IMPISP_TEMPER_AUTO) {
		on = 1;
	} else {
		on = 0;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetIridixSwitch(int &on)
{
	IMPISPDrcAttr drc;
	int ret = -1;

	ret = IMP_ISP_Tuning_GetRawDRC(&drc);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << " IMP_ISP_Tuning_GetRawDRC Error" << endl;
		return -1;
	}

	if(drc.mode == IMPISP_DRC_DISABLE) {
		on = 0;
	} else {
		on = 1;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetNightModeSwitch(int &on)
{
	IMPISPColorfxMode color;

	IMP_ISP_Tuning_GetColorfxMode(&color);

	on = color == IMPISP_COLORFX_MODE_BW ? 1 : 0;

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;

	return 0;
}

int CarrierCommandImpl::GetISPEffect(ISPEffect &parameters)
{
	IMPISPTuningOpsMode hflip;
	IMPISPTuningOpsMode vflip;
	IMPISPAntiflickerAttr pattr;
        unsigned char contrast, brightness, saturation, enableMirror, enableFlip, sharpness;

	IMP_ISP_Tuning_GetContrast(&contrast);
	IMP_ISP_Tuning_GetBrightness(&brightness);
	IMP_ISP_Tuning_GetSaturation(&saturation);
	IMP_ISP_Tuning_GetSharpness(&sharpness);

	IMP_ISP_Tuning_GetISPHflip(&hflip);
	IMP_ISP_Tuning_GetISPVflip(&vflip);

	IMP_ISP_Tuning_GetAntiFlickerAttr(&pattr);

	if(hflip == IMPISP_TUNING_OPS_MODE_ENABLE)
		enableMirror = 1;
	else
		enableMirror = 0;

	if(vflip == IMPISP_TUNING_OPS_MODE_ENABLE)
		enableFlip = 1;
	else
		enableFlip = 0;

	parameters.contrast	= contrast;
	parameters.brightness	= brightness;
	parameters.saturation	= saturation;
	parameters.enableMirror	= enableMirror;
	parameters.enableFlip	= enableFlip;
	parameters.sharpness    = sharpness;

	switch(pattr) {
		case IMPISP_ANTIFLICKER_DISABLE:
			parameters.antiflicker = 0;
			break;
		case IMPISP_ANTIFLICKER_50HZ:
			parameters.antiflicker = 1;
			break;
		case IMPISP_ANTIFLICKER_60HZ:
			parameters.antiflicker = 2;
			break;
		default:
			IMP_LOG_DBG(TAG, "antiflicker(%d) not unsupport !\n", parameters.antiflicker);
			break;
	}
#if 0
	IMP_LOG_DBG(TAG, "parameters.contrast	= %d\n", contrast  );
	IMP_LOG_DBG(TAG, "parameters.brightness	= %d\n", brightness);
	IMP_LOG_DBG(TAG, "parameters.saturation	= %d\n", saturation);
	IMP_LOG_DBG(TAG, "parameters.enableMirror	= %d\n", enableMirror);
	IMP_LOG_DBG(TAG, "parameters.enableFlip	= %d\n", enableFlip);
#endif
	return 0;
}

int CarrierCommandImpl::SetISPEffect(ISPEffect &parameters)
{
	IMPISPTuningOpsMode hflip;
	IMPISPTuningOpsMode vflip;
	IMPISPAntiflickerAttr pattr;
	unsigned char contrast = parameters.contrast;
	unsigned char brightness = parameters.brightness;
	unsigned char saturation = parameters.saturation;
	unsigned char sharpness = parameters.sharpness;
#if 0
	IMP_LOG_DBG(TAG, "parameters.contrast	= %d\n", parameters.contrast	);
	IMP_LOG_DBG(TAG, "parameters.brightness	= %d\n", parameters.brightness	);
	IMP_LOG_DBG(TAG, "parameters.saturation	= %d\n", parameters.saturation	);
	IMP_LOG_DBG(TAG, "parameters.enableMirror	= %d\n", parameters.enableMirror);
	IMP_LOG_DBG(TAG, "parameters.enableFlip	= %d\n", parameters.enableFlip	);
#endif
	if(parameters.enableMirror == 1)
		hflip = IMPISP_TUNING_OPS_MODE_ENABLE;
	else
		hflip = IMPISP_TUNING_OPS_MODE_DISABLE;

	if(parameters.enableFlip == 1)
		vflip = IMPISP_TUNING_OPS_MODE_ENABLE;
	else
		vflip = IMPISP_TUNING_OPS_MODE_DISABLE;

	IMP_ISP_Tuning_SetSharpness(sharpness);
	IMP_ISP_Tuning_SetContrast(contrast);
	IMP_ISP_Tuning_SetBrightness(brightness);
	IMP_ISP_Tuning_SetSaturation(saturation);
	IMP_ISP_Tuning_SetISPHflip(hflip);
	IMP_ISP_Tuning_SetISPVflip(vflip);

	printf("parameters.antiflicker : %d\n", parameters.antiflicker);

	pattr = IMPISP_ANTIFLICKER_DISABLE;
	switch(parameters.antiflicker) {
		case 0:
			pattr = IMPISP_ANTIFLICKER_DISABLE;
			break;
		case 1:
			pattr = IMPISP_ANTIFLICKER_50HZ;
			break;
		case 2:
			pattr = IMPISP_ANTIFLICKER_60HZ;
			break;
		default:
			IMP_LOG_DBG(TAG, "antiflicker(%d) not unsupport !\n", parameters.antiflicker);
			break;
	}
	IMP_ISP_Tuning_SetAntiFlickerAttr(pattr);

	return 0;
}

int CarrierCommandImpl::GetSensorEffect(SensorEffect &parameters)
{
	IMP_ISP_GetSensorRegister(parameters.reg, &parameters.value);
#if 0
	IMP_LOG_DBG(TAG, "reg : %x\n", parameters.reg);
	IMP_LOG_DBG(TAG, "value : %x\n", parameters.value);
#endif
	return 0;
}

int CarrierCommandImpl::SetSensorEffect(SensorEffect &parameters)
{
#if 0
	IMP_LOG_DBG(TAG, "reg : %x\n", parameters.reg);
	IMP_LOG_DBG(TAG, "value : %x\n", parameters.value);
#endif

	IMP_ISP_SetSensorRegister(parameters.reg, parameters.value);
	return 0;
}

int CarrierCommandImpl::GetSensorFPS(int &fpsNum, int &fpsDen)
{
	fpsNum = 25;
	fpsDen = 2;
	IMP_LOG_DBG(TAG, "%s: %d/%d\n", fpsNum, fpsDen);

	return 0;
}

int CarrierCommandImpl::SetSensorFPS(int fpsNum, int fpsDen)
{
	IMPEncoderGOPSizeCfg pstGOPSizeCfg = {.gopsize = fpsNum * 2 / fpsDen};

	IMP_LOG_DBG(TAG, "%s: %d/%d\n", __func__, fpsNum, fpsDen);

	IMP_ISP_Tuning_SetSensorFPS(fpsNum, fpsDen);
	IMP_Encoder_SetGOPSize(0, &pstGOPSizeCfg);
	IMP_Encoder_SetGOPSize(1, &pstGOPSizeCfg);

	return 0;
}

int CarrierCommandImpl::GetRCMethod(int chn, enum RCMethod &rc)
{
	int ret = -1;
	IMPEncoderRcAttr attr;

	memset(&attr, 0, sizeof(IMPEncoderRcAttr));
	attr.rcMode = ENC_RC_MODE_H264INV;
	ret = IMP_Encoder_GetChnRcAttr(chn, &attr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetChnRcAttr Error" << endl;
		return ret;
	}

	switch(attr.rcMode) {
		case ENC_RC_MODE_H264FIXQP:
			{
				rc = FixQP;
			}
			break;
		case ENC_RC_MODE_H264CBR:
			{
				rc = CBR;
			}
			break;
		default:
			cout << "Encoding mode not supported " << attr.rcMode << endl;
			return -1;
	}

	return 0;
}

int CarrierCommandImpl::GetRCAttr(int chn, RcAttrCBR &attrCbr)
{
	int ret = -1;
	IMPEncoderRcAttr attr;
	IMPEncoderGOPSizeCfg pstGOPSizeCfg;
	IMPEncoderFrmRate pstFpsCfg;

	ret = IMP_Encoder_GetGOPSize(chn, &pstGOPSizeCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetGOPSize Error" << endl;
	}

	ret = IMP_Encoder_GetChnFrmRate(chn, &pstFpsCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetChnFrmRate Error" << endl;
	}

	ret = IMP_Encoder_GetChnRcAttr(chn, &attr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetChnRcAttr Error" << endl;
		return ret;
	}

	attrCbr.maxGop = pstGOPSizeCfg.gopsize;
	attrCbr.outFrmRateNum = pstFpsCfg.frmRateNum;
	attrCbr.outFrmRateDen = pstFpsCfg.frmRateDen;
	attrCbr.outBitRate = attr.attrH264Cbr.outBitRate;
	attrCbr.maxQp = attr.attrH264Cbr.maxQp;
	attrCbr.minQp = attr.attrH264Cbr.minQp;
	attrCbr.maxFPS = attr.attrH264Cbr.maxFPS;
	attrCbr.minFPS = attr.attrH264Cbr.minFPS;
	attrCbr.IBiasLvl = attr.attrH264Cbr.IBiasLvl;
	attrCbr.FrmQPStep = attr.attrH264Cbr.FrmQPStep;
	attrCbr.GOPQPStep = attr.attrH264Cbr.GOPQPStep;
	attrCbr.AdaptiveMode = attr.attrH264Cbr.AdaptiveMode;
	attrCbr.GOPRelation = attr.attrH264Cbr.GOPRelation;

	cout << "func:" << __func__ << "line:" << __LINE__ << "Ok" << endl;
	return 0;
}

int CarrierCommandImpl::SetRCAttr(int chn, RcAttrCBR &attrCbr)
{
	IMPEncoderGOPSizeCfg pstGOPSizeCfg;
	IMPEncoderFrmRate pstFpsCfg;
	int ret = -1;

	if(attrCbr.outBitRate < 128)
		attrCbr.outBitRate = 128;
	if(attrCbr.outBitRate > 16*1024*1024)
		attrCbr.outBitRate = 16*1024*1024;

	if(attrCbr.minQp < 0)
		attrCbr.minQp = 0;
	if(attrCbr.maxQp > 51)
		attrCbr.maxQp = 51;

	if (attrCbr.minQp > attrCbr.maxQp) {
		attrCbr.minQp = attrCbr.maxQp;
	}

	if (attrCbr.IBiasLvl < -3) {
		attrCbr.IBiasLvl = -3;
	}

	if (attrCbr.IBiasLvl > 3) {
		attrCbr.IBiasLvl = 3;
	}

	if (attrCbr.FrmQPStep < 0) {
		attrCbr.FrmQPStep = 0;
	}
	if (attrCbr.FrmQPStep > 51) {
		attrCbr.FrmQPStep = 51;
	}

	if (attrCbr.GOPQPStep < 0) {
		attrCbr.GOPQPStep = 0;
	}
	if (attrCbr.GOPQPStep > 51) {
		attrCbr.GOPQPStep = 51;
	}

	rc_attr.rcMode = ENC_RC_MODE_H264CBR;
	rc_attr.attrH264Cbr.maxGop = attrCbr.maxGop;
	rc_attr.attrH264Cbr.outFrmRate.frmRateNum = attrCbr.outFrmRateNum;
	rc_attr.attrH264Cbr.outFrmRate.frmRateDen = attrCbr.outFrmRateDen;
	rc_attr.attrH264Cbr.outBitRate = attrCbr.outBitRate;
	rc_attr.attrH264Cbr.maxQp = attrCbr.maxQp;
	rc_attr.attrH264Cbr.minQp = attrCbr.minQp;
	rc_attr.attrH264Cbr.maxFPS = attrCbr.maxFPS;
	rc_attr.attrH264Cbr.minFPS = attrCbr.minFPS;
	rc_attr.attrH264Cbr.IBiasLvl = attrCbr.IBiasLvl;
	rc_attr.attrH264Cbr.FrmQPStep = attrCbr.FrmQPStep;
	rc_attr.attrH264Cbr.GOPQPStep = attrCbr.GOPQPStep;
	rc_attr.attrH264Cbr.AdaptiveMode = !!attrCbr.AdaptiveMode;
	rc_attr.attrH264Cbr.GOPRelation = !!attrCbr.GOPRelation;

	pstGOPSizeCfg.gopsize = attrCbr.maxGop;
	pstFpsCfg.frmRateNum = attrCbr.outFrmRateNum;
	pstFpsCfg.frmRateDen = attrCbr.outFrmRateDen;

	ret = IMP_Encoder_SetGOPSize(chn, &pstGOPSizeCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_SetGOPSize Error" << endl;
		return ret;
	}

	ret = IMP_Encoder_SetChnFrmRate(chn, &pstFpsCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_SetChnFrmRate Error" << endl;
	}

	ret = IMP_Encoder_SetChnRcAttr(chn, &rc_attr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_SetChnRcAttr Error" << endl;
		return ret;
	}

	ret = IMP_Encoder_RequestIDR(chn);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_RequestIDR Error" << endl;
		return ret;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << "Ok" << endl;

	return 0;
}

int CarrierCommandImpl::GetRCAttr(int chn, RcAttrFixQP &attrFixQp)
{
	int ret = -1;
	IMPEncoderRcAttr attr;
	IMPEncoderGOPSizeCfg pstGOPSizeCfg;
	IMPEncoderFrmRate pstFpsCfg;

	ret = IMP_Encoder_GetGOPSize(chn, &pstGOPSizeCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetGOPSize Error" << endl;
	}

	ret = IMP_Encoder_GetChnFrmRate(chn, &pstFpsCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetChnFrmRate Error" << endl;
	}

	ret = IMP_Encoder_GetChnRcAttr(chn, &attr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_GetChnRcAttr Error" << endl;
		return ret;
	}

	attrFixQp.maxGop = pstGOPSizeCfg.gopsize;
	attrFixQp.outFrmRateNum = pstFpsCfg.frmRateNum;
	attrFixQp.outFrmRateDen = pstFpsCfg.frmRateDen;
	attrFixQp.qp = attr.attrH264FixQp.qp;

	cout << "func:" << __func__ << "line:" << __LINE__ << "Ok" << endl;

	return 0;
}

int CarrierCommandImpl::SetRCAttr(int chn, RcAttrFixQP &attrFixQp)
{
	int ret = -1;
	IMPEncoderGOPSizeCfg pstGOPSizeCfg;
	IMPEncoderFrmRate pstFpsCfg;

	if (attrFixQp.qp < 0) {
		attrFixQp.qp = 0;
	}
	if (attrFixQp.qp > 51) {
		attrFixQp.qp = 51;
	}

	rc_attr.rcMode = ENC_RC_MODE_H264FIXQP;
	rc_attr.attrH264FixQp.maxGop = attrFixQp.maxGop;
	rc_attr.attrH264FixQp.outFrmRate.frmRateNum = attrFixQp.outFrmRateNum;
	rc_attr.attrH264FixQp.outFrmRate.frmRateDen = attrFixQp.outFrmRateDen;
	rc_attr.attrH264FixQp.qp = attrFixQp.qp;

	pstGOPSizeCfg.gopsize = attrFixQp.maxGop;
	pstFpsCfg.frmRateNum = attrFixQp.outFrmRateNum;
	pstFpsCfg.frmRateDen = attrFixQp.outFrmRateDen;

	ret = IMP_Encoder_SetGOPSize(chn, &pstGOPSizeCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_SetGOPSize Error" << endl;
		return ret;
	}

	ret = IMP_Encoder_SetChnFrmRate(chn, &pstFpsCfg);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_SetChnFrmRate Error" << endl;
	}

	ret = IMP_Encoder_SetChnRcAttr(chn, &rc_attr);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_SetChnRcAttr Error" << endl;
		return ret;
	}

	ret = IMP_Encoder_RequestIDR(chn);
	if(ret != 0) {
		cout << "func:" << __func__ << "line:" << __LINE__ << "IMP_Encoder_RequestIDR Error" << endl;
		return ret;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << "Ok" << endl;
	return 0;
}

int CarrierCommandImpl::GetBitRate(int chn, double &fps, double &bitrate)
{
	if(VideoInput::fpsIsOn[chn] == False)
		VideoInput::fpsIsOn[chn] = True;

	fps = VideoInput::gFps[chn];
	bitrate = VideoInput::gBitRate[chn];

	return 0;
}

int CarrierCommandImpl::GetSecondVideoInfo(int &w, int &h)
{
	w = gconf_Second_VideoWidth;
	h = gconf_Second_VideoHeight;

	return 0;
}

int CarrierCommandImpl::GetMainVideoInfo(int &w, int &h)
{
	w = gconf_Main_VideoWidth;
	h = gconf_Main_VideoHeight;
	IMP_LOG_DBG(TAG, "%s %d: w = %d, h = %d\n", __func__, __LINE__, w, h);

	return 0;
}

pthread_t thread_id0;
void * audio_start_func(void *arg)
{
	cout << "func:" << __func__ << "line:" << __LINE__ << "in audio_start_func......" << endl;
	AudioInput::streamRecv();

	return NULL;
}

int CarrierCommandImpl::AudioStart(void)
{
	cout << "func:" << __func__ << "line:" << __LINE__ << "pthread create......" << endl;
	if (pthread_create(&thread_id0, NULL, audio_start_func, NULL) != 0) {
		IMP_LOG_DBG(TAG, "Create thread error!\n");
		exit(1);
	}

	pthread_detach(thread_id0);

	return 0;
}

int CarrierCommandImpl::AudioStop(void)
{
	AudioInput::ptfunc = 0;
	//pthread_cancel(thread_id0);
	pthread_join(thread_id0, NULL);
	cout << "func:" << __func__ << "line:" << __LINE__ << ": pthread join ok......" << endl;

	return 0;
}

#define PicturePATH "/tmp/ingenic.raw"

extern int encH264SaveChnNum;

static int SetIsvSwitch(int on)
{
	if (on == 1) {
		VideoInput::isvIsOn = on;
		VideoInput::moveIvsOnBitmap |= MOVE_ISV_SWITCH;
		VideoInput::ivsIsOn[IVS_MOVE_DETECT] = True;
	} else if (on == 0) {
		VideoInput::isvIsOn = on;
		VideoInput::moveIvsOnBitmap &= ~MOVE_ISV_SWITCH;
		VideoInput::ivsIsOn[IVS_MOVE_DETECT] = False;
	}

	return 0;
}

int CarrierCommandImpl::DownloadInit(std::size_t bandwidth)
{
	if(bandwidth > 1000 * 1024) {
		IMP_LOG_DBG(TAG, "bandwidth is too big !\n");
		return -1;
	}

	bytebuffer = new RCF::ByteBuffer(bandwidth);
	if(bytebuffer == NULL) {
		IMP_LOG_DBG(TAG, "bytebuffer new error !\n");
		return -1;
	}

	return 0;
}

int CarrierCommandImpl::DownloadExit(void )
{
	if(bytebuffer == NULL)
		return 0;

	delete bytebuffer;

	return 0;
}

int CarrierCommandImpl::DownloadRecordStart(void)
{
	SetIsvSwitch(1);
	return 0;
}

int CarrierCommandImpl::DownloadRecordStop(void)
{
	SetIsvSwitch(0);
	return 0;
}

long int CarrierCommandImpl::DownloadRecordGetinfo(void)
{
	int fd;
	long int length;

	fd = open(RECORDPATH, O_RDONLY);
	if(fd < 0) {
		IMP_LOG_DBG(TAG, "open error !\n");
		return -1;
	}

	length = (long int)lseek(fd, 0, SEEK_END);

	close(fd);

	return length;
}

RCF::ByteBuffer CarrierCommandImpl::DownloadRecord(int num, int length)
{
	int i;
	char buf[length];

	read(fdRecord, buf, length);
	for (i = 0; i < length; i++)
	{
		bytebuffer->getPtr()[i] = buf[i];
	}

	return *bytebuffer;
}

int CarrierCommandImpl::DownloadRecordOpen(void)
{
	fdRecord = open(RECORDPATH, O_RDONLY);
	if(fdRecord < 0) {
		IMP_LOG_DBG(TAG, "open error !\n");
		return -1;
	}

	return 0;
}

int CarrierCommandImpl::DownloadRecordClose(void)
{
	close(fdRecord);

	return 0;
}

long int CarrierCommandImpl::DownloadPictureGetinfo(void)
{
	int fd;
	long int length;

	IMP_LOG_DBG(TAG, "in %s !\n", __func__);

	fd = open(PicturePATH, O_RDONLY);
	if(fd < 0) {
		IMP_LOG_DBG(TAG, "open error !\n");
		return -1;
	}

	length = (long int)lseek(fd, 0, SEEK_END);

	close(fd);

	return length;
}

RCF::ByteBuffer CarrierCommandImpl::DownloadPicture(int num, int length)
{
	IMP_LOG_DBG(TAG, "in %s !\n", __func__);
	int i;
	char buf[length];

	read(fdPicture, buf, length);
	for (i = 0; i < length; i++)
	{
		bytebuffer->getPtr()[i] = buf[i];
	}

	return *bytebuffer;
}

int CarrierCommandImpl::DownloadPictureOpen(void)
{
	IMP_LOG_DBG(TAG, "in %s !\n", __func__);
	fdPicture = open(PicturePATH, O_RDONLY);
	if(fdPicture < 0) {
		IMP_LOG_DBG(TAG, "open error !\n");
		return -1;
	}

	return 0;
}

int CarrierCommandImpl::DownloadPictureClose(void)
{
	IMP_LOG_DBG(TAG, "in %s !\n", __func__);
	close(fdPicture);

	return 0;
}

int CarrierCommandImpl::DownloadPictureStart(int format)
{
	IMP_LOG_DBG(TAG, "in %s !\n", __func__);
	int ret;

	system("rm -f /tmp/ingenic.raw");
	system("echo 1 > /proc/sys/vm/drop_caches");
	ret = VideoInput::setFrameSourceFormat((char *)PicturePATH, format);
	if(ret != 0) {
		IMP_LOG_DBG(TAG, "VideoInput::setFrameSourceFormat error !\n");
		return -1;
	}
	return 0;
}

int CarrierCommandImpl::LineDetectionSwitch(int on)
{
	VideoInput::ivsIsOn[IVS_LINE_DETECT] = on;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::FaceDetectionSwitch(int on)
{
	VideoInput::ivsIsOn[IVS_FACE_DETECT] = on;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::MotionDetectionSwitch(int on)
{
	VideoInput::moveIvsOnBitmap = (VideoInput::moveIvsOnBitmap & ~1) | ((on > 0) ? 1 : 0);
	VideoInput::ivsIsOn[IVS_MOVE_DETECT] = (VideoInput::moveIvsOnBitmap > 0) ? True : False;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;

	return 0;
}

int CarrierCommandImpl::RoiSwitch(int on)
{
	VideoInput::roiIsOn = on;
	VideoInput::moveIvsOnBitmap = (VideoInput::moveIvsOnBitmap & ~2) | ((on > 0) ? 2 : 0);
	VideoInput::ivsIsOn[IVS_MOVE_DETECT] = (VideoInput::moveIvsOnBitmap > 0) ? True : False;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;

	return 0;
}

int CarrierCommandImpl::SdcSwitch(int on)
{
	VideoInput::sdcIsOn = on;
	VideoInput::moveIvsOnBitmap = (VideoInput::moveIvsOnBitmap & ~4) | ((on > 0) ? 4 : 0);
	VideoInput::ivsIsOn[IVS_MOVE_DETECT] = (VideoInput::moveIvsOnBitmap > 0) ? True : False;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;

	return 0;
}

int CarrierCommandImpl::HumanoidDetectionSwitch(int on)
{
	if ((on > 0) && (VideoInput::bUseMixFigure > 0)) {
		VideoInput::ivsIsOn[IVS_FIGURE_DETECT] = 0;
		sleep(1);
	}

	VideoInput::bUseMixFigure = 0;
	VideoInput::ivsIsOn[IVS_FIGURE_DETECT] = on;

	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::MixhumanoidDetectionSwitch(int on)
{
	if (on > 0) {
		if (VideoInput::ivsIsOn[IVS_FIGURE_DETECT] == True) {
			VideoInput::ivsIsOn[IVS_FIGURE_DETECT] = False;
			sleep(1);
		}
		VideoInput::bUseMixFigure = 1;
		VideoInput::ivsIsOn[IVS_FIGURE_DETECT] = True;
	} else {
		VideoInput::bUseMixFigure = 0;
		VideoInput::ivsIsOn[IVS_FIGURE_DETECT] = False;
	}
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetLineDetectionSwitch(int &on)
{
	on = VideoInput::ivsIsOn[IVS_LINE_DETECT];
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetFaceDetectionSwitch(int &on)
{
	on = VideoInput::ivsIsOn[IVS_FACE_DETECT];
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetMotionDetectionSwitch(int &on)
{
	on = VideoInput::ivsIsOn[IVS_MOVE_DETECT];
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetHumanoidDetectionSwitch(int &on)
{
	if ((VideoInput::ivsIsOn[IVS_FIGURE_DETECT] == True) && (VideoInput::bUseMixFigure > 0)) {
		on = 0;
	}
	on = VideoInput::ivsIsOn[IVS_FIGURE_DETECT];
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetMixhumanoidDetectionSwitch(int &on)
{
	if (VideoInput::bUseMixFigure) {
		on = VideoInput::ivsIsOn[IVS_FIGURE_DETECT];
	} else {
		on = 0;
	}
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetRoiSwitch(int &on)
{
	on = VideoInput::roiIsOn;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::GetSdcSwitch(int &on)
{
	on = VideoInput::sdcIsOn;
	cout << "func:" << __func__ << "line:" << __LINE__ << " " << on << endl;
	return 0;
}

int CarrierCommandImpl::DrawLine(int x0, int y0, int x1, int y1)
{
	Boolean orgIvsStat = VideoInput::ivsIsOn[IVS_LINE_DETECT];

	if (VideoInput::line.size() >= MAX_LINE_NUN) {
		cout << "Not support draw over " << MAX_LINE_NUN << " lines" << endl;
		return -1;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " (" << x0 << ", " << y0 << ") ~ ("
		<< x1 << ", " << y1 << ")" << endl;
	if (VideoInput::ivsIsOn[IVS_LINE_DETECT] > 0) {
		VideoInput::ivsIsOn[IVS_LINE_DETECT] = 0;
		sleep(1);
	}

	if (VideoInput::insert_line(x0, y0, x1, y1) < 0) {
		cout << "Can't insert line(" << x0 << "," << y0 << ", " << x1 << ", "<< y1
			<< "), may it has been draw, please checked" << endl;
		VideoInput::ivsIsOn[IVS_LINE_DETECT] = orgIvsStat;
		return -1;
	}

	VideoInput::ivsIsOn[IVS_LINE_DETECT] = 1;

	return 0;
}

int CarrierCommandImpl::RemoveLine(int x0, int y0, int x1, int y1)
{
	Boolean orgIvsStat = VideoInput::ivsIsOn[IVS_LINE_DETECT];

	if (VideoInput::line.size() <= 0) {
		cout << "line list is empty" << endl;
		return -1;
	}

	cout << "func:" << __func__ << "line:" << __LINE__ << " (" << x0 << ", " << y0 << ") ~ ("
		<< x1 << ", " << y1 << ")" << endl;
	if (VideoInput::ivsIsOn[IVS_LINE_DETECT] > 0) {
		VideoInput::ivsIsOn[IVS_LINE_DETECT] = 0;
		sleep(1);
	}


	if (VideoInput::remove_line(x0, y0, x1, y1) < 0) {
		cout << "Can't remove line(" << x0 << "," << y0 << ", " << x1 << ", "<< y1
			<< "), may it isn't exist, please checked" << endl;
		VideoInput::ivsIsOn[IVS_LINE_DETECT] = orgIvsStat;
		return -1;
	}

	VideoInput::ivsIsOn[IVS_LINE_DETECT] = orgIvsStat;

	return 0;
}

int CarrierCommandImpl::PermDetectionSwitch(int on)
{
//	printf("in %s\n", __func__);

	if(on == 0)
		VideoInput::ivsIsOn[IVS_PERM_DETECT] = 0;

	return 0;
}

int CarrierCommandImpl::DrawPerm(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
	VideoInput::ivsIsOn[IVS_PERM_DETECT] = 1;
//	printf("in %s, (%d-%d)-(%d-%d)-(%d-%d)-(%d-%d)\n", __func__, x0, y0, x1, y1, x2, y2, x3, y3);
	VideoInput::permPoint[0] = x0;
	VideoInput::permPoint[1] = y0;
	VideoInput::permPoint[2] = x1;
	VideoInput::permPoint[3] = y1;
	VideoInput::permPoint[4] = x2;
	VideoInput::permPoint[5] = y2;
	VideoInput::permPoint[6] = x3;
	VideoInput::permPoint[7] = y3;
	return 0;
}

int CarrierCommandImpl::RemovePerm(void )
{
	VideoInput::ivsIsOn[IVS_PERM_DETECT] = 0;
	return 0;
}

int CarrierCommandImpl::ClearLines(void)
{
	if (VideoInput::ivsIsOn[IVS_LINE_DETECT] > 0) {
		VideoInput::ivsIsOn[IVS_LINE_DETECT] = 0;
		sleep(1);
	}

	VideoInput::clear_line();

	return 0;
}


int CarrierCommandImpl::GetResolution(int &w, int &h)
{
	w = gconf_Second_EncWidth;
	h = gconf_Second_EncHeight;

	return 0;
}

int CarrierCommandImpl::SetResolution(int w, int h)
{
	if ((w > gconf_Main_VideoWidth) && (h > gconf_Main_VideoHeight)) {
		IMP_LOG_DBG(TAG, "the set resolution(%dx%d) more than the current main(%dx%d)\n",
				w, h, gconf_Main_VideoWidth, gconf_Main_VideoHeight);
		return -1;
	}

	if ((w < 256) || (h < 16)) {
		IMP_LOG_DBG(TAG, "the set resolution(%dx%d) less than the minresolution(256x16)\n", w, h);
		return -1;
	}

	if ((w == gconf_Second_EncWidth) && (h == gconf_Second_EncHeight)) {
		IMP_LOG_DBG(TAG, "the set resolution(%dx%d) equals to the current second(%dx%d)\n",
				w, h, gconf_Second_EncWidth, gconf_Second_EncHeight);
		return 0;
	}

	IMP_LOG_ERR(TAG, "enter %s(%d) \n", __func__, __LINE__);
	return VideoInput::SetResolution(w, h);
	IMP_LOG_ERR(TAG, "leave %s(%d) \n", __func__, __LINE__);

	return 0;
}

int CarrierCommandImpl::Connect(void )
{
	IMP_LOG_DBG(TAG, "pc connect table !\n");

	return 0;
}

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

int fd;

int CarrierCommandImpl::SetPTZOpen()
{
	fd = open("/dev/motor", O_RDWR);
	if(fd < 0) {
		IMP_LOG_DBG(TAG, "open /dev/motor error !");
		return -1;
	}

	return 0;
}

int CarrierCommandImpl::SetPTZClose()
{
	close(fd);

	return 0;
}

int CarrierCommandImpl::SetPTZ(PTZInfo &info)
{
	struct motor_move_st motor_move;

	if(info.enable == 0) {
		ioctl(fd, MOTOR_STOP);
		return 0;
	}

	switch(info.direction) {
		case 1:
			IMP_LOG_DBG(TAG, "up\n");
			motor_move.motor_directional = MOTOR_DIRECTIONAL_UP;
			motor_move.motor_move_steps = -1 ;
			ioctl(fd, MOTOR_MOVE, &motor_move);
			break;
		case 2:
			IMP_LOG_DBG(TAG, "down\n");
			motor_move.motor_directional = MOTOR_DIRECTIONAL_DOWN;
			motor_move.motor_move_steps = -1 ;
			ioctl(fd, MOTOR_MOVE, &motor_move);
			break;
		case 3:
			IMP_LOG_DBG(TAG, "left\n");
			motor_move.motor_directional = MOTOR_DIRECTIONAL_LEFT;
			motor_move.motor_move_steps = -1 ;
			ioctl(fd, MOTOR_MOVE, &motor_move);
			break;
		case 4:
			IMP_LOG_DBG(TAG, "right\n");
			motor_move.motor_directional = MOTOR_DIRECTIONAL_RIGHT;
			motor_move.motor_move_steps = -1 ;
			ioctl(fd, MOTOR_MOVE, &motor_move);
			break;
		default:
			IMP_LOG_DBG(TAG, "enable : %d\n", info.enable);
			IMP_LOG_DBG(TAG, "direction : %d\n", info.direction);

	}

        return 0;
}
int CarrierCommandImpl::SetPTZSpeed(int speed)
{
	ioctl(fd, MOTOR_SPEED, &speed);

	return 0;
}

int CarrierCommandImpl::SetIRCUT(int enable)
{
	int fd, fd25, fd26;
	char on[4], off[4];

	if (!access("/tmp/setir",0)) {
		if (enable) {
			system("/tmp/setir 0 1");
		} else {
			system("/tmp/setir 1 0");
		}
		return 0;
	}

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/export error !");
		return -1;
	}

	write(fd, "25", 2);
	write(fd, "26", 2);

	close(fd);

	fd25 = open("/sys/class/gpio/gpio25/direction", O_RDWR);
	if(fd25 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio25/direction error !");
		return -1;
	}

	fd26 = open("/sys/class/gpio/gpio26/direction", O_RDWR);
	if(fd26 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio26/direction error !");
		return -1;
	}

	write(fd25, "out", 3);
	write(fd26, "out", 3);

	close(fd25);
	close(fd26);

	fd25 = open("/sys/class/gpio/gpio25/active_low", O_RDWR);
	if(fd25 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio25/active_low error !");
		return -1;
	}

	fd26 = open("/sys/class/gpio/gpio26/active_low", O_RDWR);
	if(fd26 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio26/active_low error !");
		return -1;
	}

	write(fd25, "0", 1);
	write(fd26, "0", 1);

	close(fd25);
	close(fd26);

	fd25 = open("/sys/class/gpio/gpio25/value", O_RDWR);
	if(fd25 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio25/value error !");
		return -1;
	}

	fd26 = open("/sys/class/gpio/gpio26/value", O_RDWR);
	if(fd26 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio26/value error !");
		return -1;
	}

	sprintf(on, "%d", enable);
	sprintf(off, "%d", !enable);

	write(fd25, "0", 1);
	usleep(10*1000);

	write(fd25, on, strlen(on));
	write(fd26, off, strlen(off));

	if (!enable) {
		usleep(10*1000);
		write(fd25, off, strlen(off));
	}

	close(fd25);
	close(fd26);

	return 0;
}

int CarrierCommandImpl::EnableFramesource3D(int enable)
{
//	printf("enable is %d\n", enable);
	if(enable == 1)
		IMP_FrameSource_EnableChnDenoise(0, 150, 90);
	else
		IMP_FrameSource_DisableChnDenoise(0);

	return 0;
}

int CarrierCommandImpl::SetMovesense(int sensor)
{
	int ret = 0;
	int param_size = (sizeof(move_param_input_t)>sizeof(IMP_IVS_MoveParam))?sizeof(move_param_input_t):sizeof(IMP_IVS_MoveParam);
	void *param = calloc(1, param_size);

	if (param == NULL) {
		IMP_LOG_ERR(TAG, "%s(%d):calloc param failed\n", __func__, __LINE__);
		return -1;
	}

	ret = IMP_IVS_GetParam(IVS_MOVE_DETECT, param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", IVS_MOVE_DETECT);
		goto err_IMP_IVS_GetParam;
	}

	if (gconf_use_internal_move == true) {
		int i = 0;
		for (i = 0; i < 4; i++) {
			((IMP_IVS_MoveParam *)param)->sense[i] = sensor;
		}
	} else {
		((move_param_input_t *)param)->sense = sensor;
	}

	ret = IMP_IVS_SetParam(IVS_MOVE_DETECT, param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", IVS_MOVE_DETECT);
		goto err_IMP_IVS_SetParam;
	}

	free(param);

	return 0;

err_IMP_IVS_SetParam:
err_IMP_IVS_GetParam:
	return -1;
}
