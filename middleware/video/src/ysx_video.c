#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdbool.h>
#include "ysx_video.h"
#include "ingenic_video.h"
#include "ysx_gpio.h"
#include "ysx_utils.h"
#include <imp/imp_log.h>

#define DAY_FPS 15

extern struct chn_conf chn[];
struct encoder_channel gEncoderChns[FS_CHN_NUM];

// IR mode choose
//static QCAM_IR_MODE  LastIRMod = QCAM_IR_MODE_AUTO;
// Auto IR enable flag
volatile bool IRAutoEnable = false;
// current IR status : false -> day , true -> night
bool IROnStatus = false; 
// for current fps save
static volatile int CurrentFps = 0;
unsigned int IR_AUTO_ENABLE = false, IR_Thread = 0;
///unsigned char ircut_status = true;
QCAM_IR_MODE last_mode = QCAM_IR_MODE_AUTO;
//static unsigned char auto_fps_cnt=0,low_fps=true;
bool low_fps=true;
pthread_t ir_tid = 0;
static pthread_mutex_t IRLock = PTHREAD_MUTEX_INITIALIZER;

int exit_ir_thread = 0;
uint8_t _loop;







int ircut_gpio_open(int pin)
{
	char buf[128];

	if(pin < 0 || pin > 96){
		printf("illegal pin number!\n");
		return -1;
	}

	CLEAR(buf);
	snprintf(buf,128,"echo %d > /sys/class/gpio/export",pin);
	system(buf);    

	return 0;
}

int ircut_gpio_output(int pin)
{
	char buf[128];
	int fd;
	CLEAR(buf);
	snprintf(buf,128,"/sys/class/gpio/gpio%d/direction",pin);
	fd = open(buf,O_RDWR);
	if(fd < 0)
	{
		printf("set gpio %d output error [%d] %s!\n",pin,__LINE__,strerror(errno));
		return -1;
	}
	write(fd,"out",sizeof("out"));
	close(fd);

	return 0;
}


static int ircut_gpio_write(int pin , int value)
{
    int fd;
	char buf[128];
	
    CLEAR(buf);
    snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/value",pin);
    fd = open(buf,O_WRONLY);
    if(fd < 0)
    {
        printf("set gpio %d direction error [%s]!\n",pin,strerror(errno));
        return -1;        
    }
    if (value == 0)
        write(fd,"0",sizeof("0"));
    else
        write(fd,"1",sizeof("1"));
        
    close(fd);
   

    return 0;
}


int ircut_gpio_request()
{
	int ret;
	ret = ircut_gpio_open(IRCUT_P);
	if(ret < 0)
	{
		printf("open IRCUT_P %d faield\n",IRCUT_P);
		return -1;
	}
	ret = ircut_gpio_output(IRCUT_P);
	if(ret < 0)
	{
		printf("ysx_gpio_output IRCUT_P %d faield\n",IRCUT_P);
		return -1;
	}


	ret = ircut_gpio_open(IRCUT_N);
	if(ret < 0)
	{
		printf("open IRCUT_N %d faield\n",IRCUT_N);
		return -1;
	}
	ret = ircut_gpio_output(IRCUT_N);
	if(ret < 0)
	{
		printf("ysx_gpio_output IRCUT_N %d faield\n",IRCUT_N);
		return -1;
	}


	ret = ircut_gpio_open(IR_LED);
	if(ret < 0)
	{
		printf("open IR_LED %d faield\n",IR_LED);
		return -1;
	}
	ret = ircut_gpio_output(IR_LED);
	if(ret < 0)
	{
		printf("ysx_gpio_output IR_LED %d faield\n",IR_LED);
		return -1;
	}

    ircut_gpio_write(IRCUT_N, 0);  
    ircut_gpio_write(IRCUT_P, 0); 
    ircut_gpio_write(IR_LED, IR_LED_OFF);

	return 0;
}




void ivs_skipframe_set(int value)
{
    return ;
}



#define S301
 

#include <ivs/ivs_inf_custpersonDet.h>
#include <ivs/iaac.h>
#include <ivs/verify.h>

int System_IVS_Init()
{
	int ret;
	/*add IVS group to SECOND stream*/
	ret = IMP_IVS_CreateGroup(0);   //IVS group num is 0
	if(ret < 0){
		QLOG(FATAL, "IMP_IVS_CreateGroup failed\n");
		return -1;
	}
#if 0
#ifdef YSX_PERSONDET 
    printf("IAAC_Init for T30A~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    IAACInfo ainfo = {
         .license_path = "/system_rw/perdetec/license.txt",
          .cid = 370283637,
          .fid = 2131859305,
          .sn = "2b1f801d7d5c060be6bb3f6e4eba605c",
    };
 
     ret = IAAC_Init(&ainfo);
      if (ret) {
         IMP_LOG_ERR(TAG, "IAAC_Init error!\n");
		 IMP_LOG_ERR(TAG,"wo have not license to init IVS persondet\n");
         return 0;//ret;
     }
#endif
		
#endif
	return ret;
}



int system_ivs_uninit(void)
{
    int ret = 0;
	#ifdef YSX_PERSONDET 
	IAAC_DeInit();
	#endif
    ret = IMP_IVS_DestroyGroup(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup failed!\n");
        return -1;
    }
    return 0;
}


#if 0
int sample_jpeg_init()
{
	int ret;
	int enc_grp = 0;
	//IMPEncoderAttr *enc_attr;
	IMPEncoderChnAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;
	memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	enc_attr = &channel_attr.encAttr;
	enc_attr->enType = PT_JPEG;
	enc_attr->bufSize = 0;
	enc_attr->profile = 0; 		 // image quality 0 ~ 2
	enc_attr->picWidth = 640;//SENSOR_WIDTH;
	enc_attr->picHeight = 360;//SENSOR_HEIGHT;


	/* Create Channel */
	ret = IMP_Encoder_CreateChn(JPG_CHN, &channel_attr);
	if (ret < 0) {
		QLOG(FATAL,"IMP_Encoder_CreateChn(%d) error: %d\n",JPG_CHN, ret);
		return -1;
	}

	/* Resigter Channel */
	ret = IMP_Encoder_RegisterChn(enc_grp, JPG_CHN);
	if (ret < 0) {
		QLOG(FATAL, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",JPG_CHN, ret);
		return -1;
	}
	QLOG(TRACE,"sample_jpeg_init OK");	
	return 0;
}
#endif
#define JPG_FS_ATTR 2 
int sample_jpeg_init()
{
	//int i, ret;
	int ret;
	IMPEncoderChnAttr channel_attr;
//	IMPFSChnAttr *imp_chn_attr_tmp;
	int grp_num = 0;
	printf("jpg channel init\n");

	//imp_chn_attr_tmp = &chn[JPG_FS_ATTR].fs_chn_attr;
	memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	//ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
		//	imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
			//imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, 25, 0);
	ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
			640, 360,
			10, 1, 0, 0, 25, 0);
	/* Create Channel */
	ret = IMP_Encoder_CreateChn(JPG_CHN, &channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
		JPG_CHN, ret);
		return -1;
	}

	/* Resigter Channel */
	ret = IMP_Encoder_RegisterChn(grp_num, JPG_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
		JPG_CHN, ret);
		return -1;
	}
	

	return 0;
}




int ResolutionCheck(QCamVideoInputChannel chn , int *width , int *height)
{
	switch(chn.res)
	{
		case QCAM_VIDEO_RES_360P:
			*width  = 640;	
			*height = 360;	
			break;

		case QCAM_VIDEO_RES_480P:
			*width  = 640;	
			*height = 480;	
			break;

		case QCAM_VIDEO_RES_720P:
			*width  = 1280;	
			*height = 720;	
			break;

		case QCAM_VIDEO_RES_1080P:
			*width  = 1920;	
			*height = 1080;	
			break;
		case QCAM_VIDEO_RES_1296P:
			*width  = 2304;	
			*height = 1296;	
			break;
		
		case QCAM_VIDEO_RES_300W:
			*width  = 2048;	
			*height = 1520;	
			break;

		case QCAM_VIDEO_RES_400W:
			*width	= 2592; 
			*height = 1520; 
			break;

		case QCAM_VIDEO_RES_500W:
			*width  = 2560;	
			*height = 1920;	
			break;
		
		default:
			VID_LOG("# invalid resolution #\n");
			return -1;
	}
		return 0;
}
int ResolutionCheck_By_Res(QCAM_VIDEO_RESOLUTION res, int *width , int *height)
{
	switch(res)
	{
		case QCAM_VIDEO_RES_360P:
			*width  = 640;	
			*height = 360;	
			break;

		case QCAM_VIDEO_RES_480P:
			*width  = 640;	
			*height = 480;	
			break;

		case QCAM_VIDEO_RES_720P:
			*width  = 1280;	
			*height = 720;	
			break;

		case QCAM_VIDEO_RES_1080P:
			*width  = 1920;	
			*height = 1080;	
			break;
		case QCAM_VIDEO_RES_1296P:
			*width  = 2304;	
			*height = 1296;	
			break;
		
		case QCAM_VIDEO_RES_300W:
			*width  = 2048;	
			*height = 1520;	
			break;

		case QCAM_VIDEO_RES_400W:
			*width	= 2592; 
			*height = 1520; 
			break;

		case QCAM_VIDEO_RES_500W:
			*width  = 2560;	
			*height = 1920;	
			break;
		
		default:
			VID_LOG("# invalid resolution #\n");
			return -1;
	}
		return 0;
}

#if 0
int EncoderChannelAdd(QCamVideoInputChannel chn)
{
	int i, ret, chnNum = 0;
	IMPEncoderAttr *enc_attr;
	IMPEncoderRcAttr *rc_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;
	int picWidth , picHeight;
	int bitRate = chn.bitrate;
	int S_RC_METHOD = ENC_RC_MODE_SMART;

	if(ResolutionCheck(chn,&picWidth,&picHeight) < 0)
		return -1;
	QLOG(FATAL, "channel: %02d, bitreate: %04d\n",
		chn.channelId,
		chn.bitrate);
	
	memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
	enc_attr = &channel_attr.encAttr;

    switch(chn.payloadType)
	{
		case 0:
			enc_attr->enType = PT_H264;break;
		case 1:
			enc_attr->enType = PT_H265;break;
		default:
			VID_LOG("invalid payload type[%d]\n",chn.payloadType);
			return -1;
	}

	enc_attr->bufSize   = picWidth*picHeight*3/2/2;	//0;
	enc_attr->profile   = 2;						// 0: baseline; 1:MP; 2:HP
	enc_attr->picWidth  = picWidth;
	enc_attr->picHeight = picHeight;	
	chnNum = chn.channelId;
	VID_LOG("Encoder chn %d: %d * %d\n",chnNum,enc_attr->picWidth,enc_attr->picHeight);

	if(enc_attr->enType == PT_H264) 
	{
		rc_attr = &channel_attr.rcAttr;
		rc_attr->outFrmRate.frmRateNum = DAY_FPS;
		rc_attr->outFrmRate.frmRateDen = 1;
		rc_attr->maxGop = 2 * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;
		if (S_RC_METHOD == ENC_RC_MODE_CBR) 
		{
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_CBR;
			rc_attr->attrRcMode.attrH264Cbr.outBitRate = bitRate;
			rc_attr->attrRcMode.attrH264Cbr.maxQp = YSX_MAX_QP;
			rc_attr->attrRcMode.attrH264Cbr.minQp = YSX_MIN_QP;
			rc_attr->attrRcMode.attrH264Cbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH264Cbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH264Cbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH264Cbr.adaptiveMode = false;
			rc_attr->attrRcMode.attrH264Cbr.gopRelation = false;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;	     
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else if (S_RC_METHOD == ENC_RC_MODE_VBR) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
			rc_attr->attrRcMode.attrH264Vbr.maxQp = YSX_MAX_QP;
			rc_attr->attrRcMode.attrH264Vbr.minQp = YSX_MIN_QP;
			rc_attr->attrRcMode.attrH264Vbr.staticTime = 2;
			rc_attr->attrRcMode.attrH264Vbr.maxBitRate = bitRate;
			rc_attr->attrRcMode.attrH264Vbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH264Vbr.changePos = 80;
			rc_attr->attrRcMode.attrH264Vbr.qualityLvl = 2;
			rc_attr->attrRcMode.attrH264Vbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH264Vbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH264Vbr.gopRelation = false;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;	
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} else if (S_RC_METHOD == ENC_RC_MODE_SMART) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
			rc_attr->attrRcMode.attrH264Smart.maxQp = YSX_MAX_QP;
			rc_attr->attrRcMode.attrH264Smart.minQp = YSX_MIN_QP;
			rc_attr->attrRcMode.attrH264Smart.staticTime = 2;
			rc_attr->attrRcMode.attrH264Smart.maxBitRate = bitRate;
			rc_attr->attrRcMode.attrH264Smart.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH264Smart.changePos = 80;
			rc_attr->attrRcMode.attrH264Smart.qualityLvl = 2;
			rc_attr->attrRcMode.attrH264Smart.frmQPStep = 3;
			rc_attr->attrRcMode.attrH264Smart.gopQPStep = 15;
			rc_attr->attrRcMode.attrH264Smart.gopRelation = false;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
			rc_attr->attrHSkip.hSkipAttr.n = 1;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 1;	
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} 
	} 
	else if(enc_attr->enType == PT_H265)  //PT_H265
	{ 
		rc_attr = &channel_attr.rcAttr;
		rc_attr->outFrmRate.frmRateNum = DAY_FPS;
		rc_attr->outFrmRate.frmRateDen = 1;
		rc_attr->maxGop = chn.gop * rc_attr->outFrmRate.frmRateNum / rc_attr->outFrmRate.frmRateDen;
		if (S_RC_METHOD == ENC_RC_MODE_CBR) {
			rc_attr->attrRcMode.attrH265Cbr.maxQp = YSX_MAX_QP;
			rc_attr->attrRcMode.attrH265Cbr.minQp = YSX_MIN_QP;
			rc_attr->attrRcMode.attrH265Cbr.staticTime = 2;
			rc_attr->attrRcMode.attrH265Cbr.outBitRate = bitRate;
			rc_attr->attrRcMode.attrH265Cbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH265Cbr.frmQPStep = 2;		//default:3
			rc_attr->attrRcMode.attrH265Cbr.gopQPStep = 30;		//default:15	
			rc_attr->attrRcMode.attrH265Cbr.flucLvl = 0;		//default:2

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;	
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} 
		else if (S_RC_METHOD == ENC_RC_MODE_VBR) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
			rc_attr->attrRcMode.attrH265Vbr.maxQp = YSX_MAX_QP;
			rc_attr->attrRcMode.attrH265Vbr.minQp = YSX_MIN_QP;
			rc_attr->attrRcMode.attrH265Vbr.staticTime = 2;
			rc_attr->attrRcMode.attrH265Vbr.maxBitRate = bitRate;
			rc_attr->attrRcMode.attrH265Vbr.iBiasLvl = 0;
			rc_attr->attrRcMode.attrH265Vbr.changePos = 80;
			rc_attr->attrRcMode.attrH265Vbr.qualityLvl = 2;
			rc_attr->attrRcMode.attrH265Vbr.frmQPStep = 3;
			rc_attr->attrRcMode.attrH265Vbr.gopQPStep = 15;
			rc_attr->attrRcMode.attrH265Vbr.flucLvl = 2;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = 0;
			rc_attr->attrHSkip.hSkipAttr.n = 0;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;	
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} 
		else if (S_RC_METHOD == ENC_RC_MODE_SMART) {
			rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
			rc_attr->attrRcMode.attrH265Smart.maxQp = YSX_MAX_QP;
			rc_attr->attrRcMode.attrH265Smart.minQp = YSX_MIN_QP;
			rc_attr->attrRcMode.attrH265Smart.staticTime = 2;
			rc_attr->attrRcMode.attrH265Smart.maxBitRate = bitRate;
			rc_attr->attrRcMode.attrH265Smart.iBiasLvl = 2;
			rc_attr->attrRcMode.attrH265Smart.changePos = 80;
			rc_attr->attrRcMode.attrH265Smart.qualityLvl = 2;
			rc_attr->attrRcMode.attrH265Smart.frmQPStep = 2;
			rc_attr->attrRcMode.attrH265Smart.gopQPStep = 30;
			rc_attr->attrRcMode.attrH265Smart.flucLvl = 0;

			rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
			rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
			rc_attr->attrHSkip.hSkipAttr.n = 1;
			rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 1;	//6,
			rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
			rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
			rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
		} 
	}
/*
	ret = IMP_Encoder_SetFisheyeEnableStatus(chnNum, 1);
	if (ret < 0) {
		VID_LOG("IMP_Encoder_SetFisheyeEnableStatus(%d) error !\n", chnNum);
		return -1;
	}	
*/
	ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
	if (ret < 0) {
		VID_LOG("IMP_Encoder_CreateChn(%d) error !\n", chnNum);
		return -1;
	}

	ret = IMP_Encoder_RegisterChn(chnNum, chnNum);
	if (ret < 0) {
		VID_LOG("IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chnNum, chnNum, ret);
		return -1;
	}

	return 0;
}
#endif


struct srb_info
{
unsigned int ch;
unsigned int seq_num;
};
int t_payload[FS_CHN_NUM] ={0};

typedef enum{
	PAYLOAD_H264=0,
    PAYLOAD_H265
   
}PAYLOAD_TYPE;
typedef struct _NaluUnit
{
		int type;
		int size;
		//unsigned char *data;
}NaluUnit;

void set_payload(int index,int val){

	t_payload[index] = val;

}

int ReadOneNaluFromBuf(NaluUnit *nalu,unsigned char *buf, unsigned int size,int stream_type)
	{
		unsigned int  nalhead_pos = 0;
	
		int nal_offset=nalhead_pos;
		nalu->size = 0;
		int one_nalu = 1; //本地读取buf是否包含多个nal
		if(size < 4)
		{
			printf("buf size is too small %d\n",size);
			return 0;
		}
	
		while(nalhead_pos + 4 <size)
		{
			//search for nal header ，NALU 单元的开始, 必须是 "00 00 00 01" 或 "00 00 01",
			// find next nal header 00 00 00 01 to calu the length of last nal
			if(buf[nalhead_pos++] == 0x00 &&
					buf[nalhead_pos++] == 0x00 &&
					buf[nalhead_pos++] == 0x00 &&
					buf[nalhead_pos++] == 0x01)
			{
				goto gotnal_head;
			}
			else
			{
				continue;
			}
			//search for nal tail which is also the head of next nal
	gotnal_head:
			nal_offset = nalhead_pos;
			while (nal_offset + 4 < size)
			{
				// find next nal header 00 00 00 01 to calu the length of last nal
				if(buf[nal_offset++] == 0x00 &&
						buf[nal_offset++] == 0x00 &&
						buf[nal_offset++] == 0x00 &&
						buf[nal_offset++] == 0x01)
				{
					nalu->size = (nal_offset-4)-nalhead_pos; //nal头4个字节
					one_nalu = 0;
					break;
				}
				else
					continue;
	
			}
			if(one_nalu)
			{
				nal_offset = size;
				nalu->size = nal_offset-nalhead_pos; //nal头4个字节
			}
			if(stream_type == PAYLOAD_H265)
				nalu->type = (buf[nalhead_pos]&0x7E)>>1;
			else
				nalu->type = buf[nalhead_pos]&0x1f; 	// 7-> pps , 8->sps
			
	
			nalhead_pos =(one_nalu==1) ? nal_offset:(nal_offset-4);
			break;
		}
	
		return nalhead_pos;
}

//#define SHOW_FRM_BITRATE
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

void * ysx_stream(void *arg)
{
	struct timeval tv = {0,0};
	int ret,i;
	uint32_t len;
	int EncCh = *((int *)arg);
	printf("=======EncCh %d\n",EncCh);
	printf("start polling stream chn %d\n",EncCh);
	IMPEncoderStream stream;
	int keyframe;


	/*start recive frame */
	ret = IMP_Encoder_StartRecvPic(EncCh);
	if (ret < 0) {
		QLOG(FATAL, "IMP_Encoder_StartRecvPic Error With %d\n",ret);
		return (void *)-1;
	} 

	char pr_name[64];
	memset(pr_name, 0, sizeof(pr_name));
	sprintf(pr_name, "ysx_stream[%d]", EncCh);
	prctl(PR_SET_NAME, pr_name);
	int frm_cnt = 0;
	struct srb_info*srbinfo=NULL;
	srbinfo = (struct srb_info *)malloc(sizeof(struct srb_info));
    memset(srbinfo, 0, sizeof(struct srb_info));
	int stream_type;
	if(t_payload[EncCh] == 1){
		
		stream_type = PAYLOAD_H265;
	}else
		stream_type = PAYLOAD_H264;
	//static int num=0;
	//#define TEST_POC
	#ifdef TEST_POC
	char tmp_path[128];
	memset(tmp_path,0,sizeof(tmp_path));
	if(stream_type == PAYLOAD_H265)
		sprintf(tmp_path,"%s","/tmp/mmcblk0p1/T31_CESHI.h265");
	else
		sprintf(tmp_path,"%s","/tmp/mmcblk0p1/T31_CESHI.h264");
	FILE*fp1=fopen(tmp_path,"w+");
	
	#endif
	while(_loop)
	{
		/* Polling video Stream, set timeout as 1000msec */
		ret = IMP_Encoder_PollingStream(EncCh, 2000);
		if (ret < 0) {
			QLOG(FATAL, "Polling stream timeout,chn[%d]\n",EncCh);
			continue;
		}

		/* Get video Stream */
		ret = IMP_Encoder_GetStream(EncCh, &stream, 1);
		if (ret < 0) {
			QLOG(FATAL, "IMP_Encoder_GetStream() failed\n");
			continue;
		}
		//IMPEncoderStream*data_hdr=(IMPEncoderStream*)stream;
		if (stream.seq - srbinfo->seq_num  > 1)
		{
             printf("==================================================================================================\n");
             printf("|||||||===== Frame skipped, stream->seq(%u), srbinfo->seq_num(%u) ======\n", stream.seq, srbinfo->seq_num);
             printf("==================================================================================================\n");
        }
        srbinfo->seq_num = stream.seq;
		for (i = 0, len = 0; i < stream.packCount; i++) {            
			len += stream.pack[i].length;
		}
		#ifdef SHOW_FRM_BITRATE
   
    	bitrate_sp[EncCh] += len;
    	frmrate_sp[EncCh]++;

    	int64_t now = IMP_System_GetTimeStamp() / 1000;
    	if(((int)(now - statime_sp[EncCh]) / 1000) >= FRM_BIT_RATE_TIME){
      		double fps = (double)frmrate_sp[EncCh] / ((double)(now - statime_sp[EncCh]) / 1000);
      		double kbr = (double)bitrate_sp[EncCh] * 8 / (double)(now - statime_sp[EncCh]);

      	printf("streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", EncCh, fps, kbr);
      

      	frmrate_sp[EncCh] = 0;
      	bitrate_sp[EncCh] = 0;
      	statime_sp[EncCh] = now;
    	}
		#endif
		//if(num++%150==0)
		//printf("encode video timestamp:%llu\n",stream.pack->timestamp);
		gettimeofday(&tv, NULL);   
		keyframe = 0;
		//keyframe = (stream.refType == IMP_Encoder_FSTYPE_IDR)?1:0;	
		NaluUnit naluUnit ;//= {0};
		memset(&naluUnit,0,sizeof(naluUnit));
		int offset = 0;
		int cnt = 0;
		while(ret = ReadOneNaluFromBuf(&naluUnit,(uint8_t*)(stream.virAddr)+offset,len-offset,stream_type)){
			if(stream_type == PAYLOAD_H264){
				if(naluUnit.type == 7){
					///if(EncCh == 0) printf("[%d] sps %d\n",__LINE__,cnt);
				}else if(naluUnit.type == 8){

					//if(EncCh == 0) printf("[%d] pps %d\n",__LINE__,cnt);
				}else if(naluUnit.type == 5){
					//if(EncCh == 0) printf("[%d] I frame %d\n",__LINE__,cnt);
					keyframe = 1;
				}
			}else{
				if(naluUnit.type == 32){
					//if(EncCh == 0) printf("[%d] vps %d\n",__LINE__,cnt);
				}else if(naluUnit.type == 33){

					//if(EncCh == 0) printf("[%d] pps %d\n",__LINE__,cnt);
				}else if(naluUnit.type == 34){

					//if(EncCh == 0) printf("[%d] pps %d\n",__LINE__,cnt);
				}else if(naluUnit.type == 19){
					//if(EncCh == 0) printf("[%d] I frame %d\n",__LINE__,cnt);
					keyframe = 1;
				}

			
			}
			memset(&naluUnit,0,sizeof(NaluUnit));
			offset += ret;
			if(offset >= len)
				break;
			cnt++;
		}
		if(keyframe == 1)
			frm_cnt = 0 ;
        if (gEncoderChns[EncCh].ChnCfg.cb) {
			//if(EncCh==1) printf("[%d] %s %d\n",__LINE__,keyframe ? "I frame":"P frame",++frm_cnt);
		    gEncoderChns[EncCh].ChnCfg.cb(&tv, (void *)stream./*pack[0].*/virAddr, len, keyframe);
			#ifdef TEST_POC
			if(EncCh==1&&num<1500){
			fwrite((void *)stream.virAddr,len,1,fp1);	
			num++;
			}
			#endif
        }
      
		IMP_Encoder_ReleaseStream(EncCh, &stream);
        if (!gEncoderChns[EncCh].enable) {
            break;
        }
	}
	if(srbinfo)
		free(srbinfo);
	ret = IMP_Encoder_StopRecvPic(EncCh);
	if (ret < 0) {
		QLOG(FATAL, "IMP_Encoder_StopRecvPic Error With %d\n",ret);
		return (void *)-1;
	} 
	 #ifdef TEST_POC
     fclose(fp1);
	 #endif
	printf("###thread ysx_stream[%d] exit !###\n",EncCh);
	pthread_exit(0);

}

void IRModeSet(QCAM_IR_MODE mode)
{
	switch(mode) 
	{
		case QCAM_IR_MODE_ON:

			QLOG(FATAL,"IMP_ISP_Tuning_SetSensorFPS mode Night, fps %d\n", SENSOR_FPS_NIGHT);

			if (IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_NIGHT, 1) != 0) {
				QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed");
				return ;
			}	   
			if (IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_NIGHT) != 0) {
				QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error");
				return ;
			}
		
            ircut_gpio_write(IRCUT_N, 0);
            ircut_gpio_write(IRCUT_P, 1);  
            usleep(300*1000);
            ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
            ircut_gpio_write(IRCUT_P, 0);   
            usleep(100*1000);
            ircut_gpio_write(IR_LED, IR_LED_ON);

			CurrentFps = SENSOR_FPS_NIGHT;
			if (access("/tmp/ircut_nighton",F_OK) != 0) { 
            	system("touch /tmp/ircut_nighton");
        	}    
			break;
            
		case QCAM_IR_MODE_OFF:
			
            ircut_gpio_write(IRCUT_N, 1);
            ircut_gpio_write(IRCUT_P, 0); 
            usleep(300*1000);
            ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
            ircut_gpio_write(IRCUT_P, 0); 
            usleep(100*1000);
            ircut_gpio_write(IR_LED, IR_LED_OFF);

			QLOG(FATAL,"IMP_ISP_Tuning_SetSensorFPS mode Day, fps %d\n", SENSOR_FPS_DAY);

			if (IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_DAY, 1) != 0) {
				QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed");
				return ;
			}	   
			if (IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY) != 0) {
				QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error");
				return ;
			}
			
			CurrentFps = SENSOR_FPS_DAY;
			if(access("/tmp/ircut_nighton",F_OK) == 0){	 
				system("rm /tmp/ircut_nighton");
			}	 	
			break;
            
		default:
//			isp_mode = QCAM_IR_MODE_UNSUPPORT;
			return ;
	}

	return ;
}


/*
 * 功  能: ircut切换逻辑
 * 参  数: top,进入白天的值, button进入夜晚的值
 * 返回值:
 */
int hardware_ir_process(int top, int button, int flag)
{
	int ret, AdcVal;
	IMPISPEVAttr bright;
	static int day_cnt = 0, night_cnt = 0;
	static int val_ref = 0;

	ret = SU_ADC_GetChnValue(ADC_CHN, &AdcVal);
	if (ret < 0) {
		VID_LOG("[%d]:SU_ADC_GetChnValue failed !\n",__LINE__);
		return -1;
	}

	if (flag) {
		EMGCY_LOG("auto=%02d, adc value=%04d, top=%06d, button=%06d, ir status=%02d, night_cnt=%02d day_cnt=%02d\n",
                IRAutoEnable,
				AdcVal,
				top,
				button,
				IROnStatus,
				night_cnt,
				day_cnt); 
    }

    if (AdcVal < top) {
		day_cnt ++; 
		night_cnt = 0;
		if (day_cnt > 4) {
			day_cnt = 0;
			pthread_mutex_lock(&IRLock);
			if (IRAutoEnable  && IROnStatus) {
				IRModeSet(QCAM_IR_MODE_OFF);   
				IROnStatus = false;
			}
			pthread_mutex_unlock(&IRLock);		
		}
	}    
	else {
		day_cnt = 0;
	}

	if (IMP_ISP_Tuning_GetEVAttr(&bright) != 0) {
		QLOG(FATAL, "IMP_ISP_Tuning_GetEVAttr Error ");
		return -1;
	} 

	if (flag) {
		VID_LOG("ev = %d, light_little = %d\n", bright.ev, top);
    }
    
    if (AdcVal > button) {
		night_cnt ++;
		day_cnt = 0;
		if (night_cnt > 4) {
			night_cnt = 0;
			pthread_mutex_lock(&IRLock);
			if (IRAutoEnable && (!IROnStatus) ){
				IRModeSet(QCAM_IR_MODE_ON);   
				IROnStatus = true;  
				val_ref = AdcVal;
			}
			pthread_mutex_unlock(&IRLock);
		}
	}
	else {
		night_cnt = 0;
	}

	return 0;
}


int DayAutoFpsSet(int light_little,int light_more)
{
	IMPISPEVAttr bright;
	static int fps_cnt = 0;

	if (IMP_ISP_Tuning_GetEVAttr(&bright) != 0) 
	{
		QLOG(FATAL, "IMP_ISP_Tuning_GetEVAttr Error");
		return -1;
	} 

	if(bright.ev < light_more )
	{
		if(CurrentFps != SENSOR_FPS_DAY )
		{
			fps_cnt++;
			if (fps_cnt > 4)	// 20 fps
			{
				fps_cnt = 0;
				QLOG(TRACE,"--> Set Fps To %d fps ",SENSOR_FPS_DAY);	

				if(IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_DAY, 1) != 0)
				{
					QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed");
					return -1;
				}
				CurrentFps = SENSOR_FPS_DAY;

				IMPISPAntiflickerAttr flick_attr = IMPISP_ANTIFLICKER_50HZ; 
				if(IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr) != 0)
				{
					QLOG(TRACE,"IMP_ISP_Tuning_SetAntiFlickerAttr failed !");
				}
			}
		}
	}
	else if(bright.ev > light_little )
	{
		if(CurrentFps == SENSOR_FPS_DAY )
		{
			fps_cnt++;
			if(fps_cnt > 4)   // 10 fps
			{
				fps_cnt = 0;
				QLOG(TRACE,"--> Set Fps To %d fps ",SENSOR_FPS_NIGHT);	

				if(IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_NIGHT, 1) != 0){
					QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
					return -1;
				}

				CurrentFps = SENSOR_FPS_NIGHT;

				IMPISPAntiflickerAttr flick_attr = IMPISP_ANTIFLICKER_DISABLE; 
				if(IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr) != 0)
				{
					QLOG(TRACE,"IMP_ISP_Tuning_SetAntiFlickerAttr failed !");
				}
			}
		}
	}
	else
	{
		fps_cnt = 0;
	}

	return 0;
}


// get current IR mode
QCAM_IR_MODE QCamGetIRMode()
{
	QCAM_IR_MODE mode;
	mode = ipc_ircut_mode_get();
//	mode = IROnStatus == true ? QCAM_IR_MODE_ON : QCAM_IR_MODE_OFF ;
	return mode;
}

QCAM_IR_STATUS QCamGetIRStatus()
{
	QCAM_IR_STATUS status;
	status = ipc_ircut_status_get();
//	mode = IROnStatus == true ? QCAM_IR_MODE_ON : QCAM_IR_MODE_OFF ;
	return status;
}

//static bool auto_fps_enable = true;

#if 0
void *Thread_IR_Auto(void *arg)
{
	int ret;
	int lg_top = 0 , lg_button=0,ev_log = 0;   
	int ev1=0,ev2=0;
	bool soft_ir = false;

	prctl(PR_SET_NAME,"Thread_IR_Auto");

	ev1 = 727942;
	ev2 = 855137;
	ysx_read_config(SYMBOL_EV_LOG, &ev_log);  
	ysx_read_config(SYMBOL_EV1_VAL, &ev1); 
	ysx_read_config(SYMBOL_EV2_VAL, &ev2);
	ysx_read_config(SYMBOL_LIGHTNESS_TOP2, &lg_top);	 /*read config file*/
	ysx_read_config(SYMBOL_LIGHTNESS_BUTTON2, &lg_button);

	if(access("/system_rw/soft_ir",F_OK) ==0 )  
	{
		soft_ir = true;
	}
	else
	{
		soft_ir = false;           
	}

	QLOG(TRACE,"Ev value:ev1(%d),ev2(%d)",ev1,ev2);    
	if (!soft_ir) {
		ret = SU_ADC_Init();
		if(ret < 0){
			printf("[%d]:SU_ADC_Init failed !\n",__LINE__);
			return NULL;
		}

		ret = SU_ADC_EnableChn(ADC_CHN);    
		if(ret < 0){
			printf("[%d]:SU_ADC_EnableChn failed !\n",__LINE__);
			return NULL;
		}
	}

	while(1)
	{

		if (!soft_ir) {
			hardware_ir_process(lg_top,lg_button,ev_log);           

		}
		else {
			VID_LOG("--> Not support software switch IR\n");
		}

		// low fps at day when light is little
		if (IROnStatus == false && auto_fps_enable)
			DayAutoFpsSet(ev1, ev2);

		sleep(1);
	}

	if(!soft_ir)  
	{
		SU_ADC_DisableChn(ADC_CHN);
		SU_ADC_Exit();
	}

	QLOG(TRACE, "Thread_IR_Auto exit !\n");    
	return NULL;
}
#endif 



int ysx_ircut_set(QCAM_IR_MODE mode)
{
    int ret;
    int fps=SENSOR_FPS_DAY;
    IMPISPRunningMode  isp_mode;
    
    switch(mode) 
    {
         case QCAM_IR_MODE_ON:
             isp_mode =  IMPISP_RUNNING_MODE_NIGHT; printf("Nigth mode...");
             break;
         case QCAM_IR_MODE_OFF:
             isp_mode = IMPISP_RUNNING_MODE_DAY ; printf("Day mode...");
             break;
         default:
             isp_mode = QCAM_IR_MODE_UNSUPPORT;
             return -1;
     }
	printf("set isp running mode %d \n ",isp_mode);
	if (isp_mode == IMPISP_RUNNING_MODE_NIGHT) {
	    ret = IMP_ISP_Tuning_SetISPRunningMode(isp_mode);   
	    if (ret < 0) {
	        QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error with result %d\n",ret);
	        return -1;
	    }
	}

	if (isp_mode == IMPISP_RUNNING_MODE_DAY) { 
        ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_DAY, 1);
        if (ret < 0) {
            QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
            return -1;
        }      
        QLOG(TRACE,"Set Sensor Fps = %d , mode = %d\n",SENSOR_FPS_DAY,mode);
        sleep(1);
    }

    if (QCAM_IR_MODE_ON == mode) {
        fps = SENSOR_FPS_NIGHT;        
//        ircut_status = true;
        IROnStatus = true;
        low_fps = true;    
        printf("######ircut turn on!######\n");

        ircut_gpio_write(IRCUT_N, 0);
        ircut_gpio_write(IRCUT_P, 1);  
        usleep(300*1000);
		ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
		ircut_gpio_write(IRCUT_P, 0);   
		usleep(100*1000);
        ircut_gpio_write(IR_LED, IR_LED_ON);

    }
    else if(QCAM_IR_MODE_OFF == mode) {
      
        fps = SENSOR_FPS_DAY;
//        ircut_status = false;  
        IROnStatus = false;
        low_fps = false;
        printf("######ircut turn off!######\n"); 

        ircut_gpio_write(IRCUT_N, 1);
        ircut_gpio_write(IRCUT_P, 0); 
 		usleep(300*1000);
		ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
		ircut_gpio_write(IRCUT_P, 0); 
        usleep(100*1000);
        ircut_gpio_write(IR_LED, IR_LED_OFF);   
		
    }
    
	if (isp_mode == IMPISP_RUNNING_MODE_DAY) {
		sleep(1);
		ret = IMP_ISP_Tuning_SetISPRunningMode(isp_mode);	
		if (ret < 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error with result %d\n",ret);
			return -1;
		}
	}
	if (isp_mode == IMPISP_RUNNING_MODE_NIGHT) {       
        ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_NIGHT, 1);
        if (ret < 0) {
            QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
            return -1;
        }      
        QLOG(TRACE,"Set Sensor Fps = %d , mode = %d\n",SENSOR_FPS_NIGHT,mode);
    }
	// ivs_skipframe_set(fps);
	/*
	IMPISPAntiflickerAttr flick_attr; 
	
	if (QCAM_IR_MODE_ON == mode) {
		flick_attr = IMPISP_ANTIFLICKER_DISABLE ;
	}
	
	if (QCAM_IR_MODE_OFF == mode) {
		flick_attr = IMPISP_ANTIFLICKER_50HZ;
	}

	ret = IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr);
	if (ret != 0) {
    	QLOG(TRACE,"IMP_ISP_Tuning_SetAntiFlickerAttr=%d failed !\n",flick_attr);
	}
	*/
    return 0;
}

int ysx_ircut_set2(int mode)
{
    int ret;
	IMPISPRunningMode  isp_mode;
    int fps = 0;
	/*
    switch(mode) 
    {
         case QCAM_IR_MODE_ON:
             isp_mode =  IMPISP_RUNNING_MODE_NIGHT; printf("Nigth mode...");
			 fps = SENSOR_FPS_NIGHT;
             break;
         case QCAM_IR_MODE_OFF:
             isp_mode = IMPISP_RUNNING_MODE_DAY ; printf("Day mode...");
			  fps = SENSOR_FPS_DAY;
             break;
         default:
             isp_mode = QCAM_IR_MODE_UNSUPPORT;
             return -1;
     }
	
	
	 ret = IMP_ISP_Tuning_SetISPRunningMode(isp_mode);   
	 if (ret < 0) {
	 		QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error with result %d\n",ret);
	        return -1;
	 }
	

	
     ret = IMP_ISP_Tuning_SetSensorFPS(fps, 1);
     if (ret < 0) {
     		QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
            return -1;
     }      

	*/
	

    if (1 == mode) {
	
		
		printf("######ircut turn off  led on !######\n"); 
        ircut_gpio_write(IRCUT_N, 1);
        ircut_gpio_write(IRCUT_P, 0);  
        usleep(300*1000);
		ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
		ircut_gpio_write(IRCUT_P, 0);   
		usleep(100*1000);
		printf("[%d] %d %d\n",__LINE__,mode,IR_LED_ON);
        ircut_gpio_write(IR_LED, IR_LED_ON);

    }
    else if(0 == mode) {
      
 
        printf("######ircut turn on led off !######\n"); 

        ircut_gpio_write(IRCUT_N, 0);//60
        ircut_gpio_write(IRCUT_P, 1); 
 		usleep(300*1000);
		ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
		ircut_gpio_write(IRCUT_P, 0); 
        usleep(100*1000);
		printf("[%d] %d %d\n",__LINE__,mode,IR_LED_OFF);
        ircut_gpio_write(IR_LED, IR_LED_OFF);   
		
    }
    
	ipc_ircut_mode_set(mode);
	
    return 0;
}


static int dayMode = CURRENT_MODE_DAY;
int QCamGetModeStatus()
{
return dayMode;

}
int qcam_get_fps(){
	int fps_num = 0, fps_den = 0;
	int  ret = IMP_ISP_Tuning_GetSensorFPS(&fps_num, &fps_den);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to get fps!\n");
        return -1;
    }
	int real_fps = 0;
	if(fps_den != 0)
		real_fps = fps_num/fps_den;
	
	return real_fps;
	//return 0;
}
#if 0
int auto_fps_set(int button,int top)
{
    int ret;
    IMPISPEVAttr bright;
	static uint8_t agian_cnt =0;
	
	static uint8_t gain_low = true, gain_hig = true;
    ret = IMP_ISP_Tuning_GetEVAttr(&bright);
    if (ret < 0) {
        QLOG(FATAL, "IMP_ISP_Tuning_GetEVAttr Error with result %d\n",ret);
        return -1;
    } 
	int register_value = 0;
	//QLOG(TRACE,"[%s]--ev=%d\n",__FUNCTION__,bright.ev);
	if(access("/tmp/lzf_ok_1",F_OK) == 0)
	QLOG(TRACE,"[%s]--again=%d --gain_log2=%d\n",__FUNCTION__,bright.again,bright.gain_log2);
	#if 1
	if(dayMode == CURRENT_MODE_DAY){
	if(bright.again < 64 && gain_hig == 1){
	   agian_cnt++;
	 if(agian_cnt > 4){
	   agian_cnt = 0;
	 // AMCSystemCmd("echo w sen_reg 0x3009 0x02 > /proc/jz/isp/isp-w00");
	  ret = IMP_ISP_SetSensorRegister(0x3009,0x02);
	  //printf("write 0x 3009 0x02 =============================\n");
	  ret = IMP_ISP_GetSensorRegister(0x3009,&register_value);
                if(!ret)
                        printf("get sensor register value %d\n",register_value);
	  gain_low = 1;
	  gain_hig = 0;
	  
	}
	}else if(bright.again >= 64 && gain_low == 1){
	 agian_cnt++;
	 if(agian_cnt > 4){
	   agian_cnt = 0;
	   //AMCSystemCmd("echo w sen_reg 0x3009 0x12 > /proc/jz/isp/isp-w00");
	   ret = IMP_ISP_SetSensorRegister(0x3009,0x12);
	  // printf("write 0x 3009 0x12 ==============================\n");
	      ret = IMP_ISP_GetSensorRegister(0x3009,&register_value);
                if(!ret)
                        printf("get sensor register value %d\n",register_value);
	   gain_low = 0;
	   gain_hig = 1;
	 }

	}
	}
	#endif
	static uint8_t fps_low = true, fps_hig = true;
    if(bright.ev < top )
    {
    	if(fps_hig == 1)
    	{
            auto_fps_cnt++;
            if(auto_fps_cnt > 4)	// 20 fps
            {
            	dayMode = CURRENT_MODE_DAY;
                auto_fps_cnt = 0;
                QLOG(TRACE,"Set Fps To %d fps\n",SENSOR_FPS_DAY);
			 
	 	      // ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_DAY, 1);
	 	      #ifdef USE_REDUCE_FPS_RIPPLE
	 	      if(fps_right == FPS_60HZ_RIGHT)
				 ret = IMP_ISP_Tuning_SetSensorFPS(15, 1);
			  else if(fps_right == FPS_50HZ_RIGHT)
			  	 ret = IMP_ISP_Tuning_SetSensorFPS(25, 2);
			  #else
				 ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_DAY, 1);
			  #endif
	 	        if(ret < 0){
	 	            QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
	 	            return -1;
	 	        }
				if (IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY) != 0) {
						QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error\n");
						return -1;
				}
			    fps_hig = 0;
				fps_low = 1;
				
               // low_fps = false;
                ivs_skipframe_set(SENSOR_FPS_DAY);
				//IMPISPAntiflickerAttr flick_attr = IMPISP_ANTIFLICKER_50HZ; 
				//ret = IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr);
				//if(ret != 0){
				//	QLOG(TRACE,"IMP_ISP_Tuning_SetAntiFlickerAttr=%d failed !\n",flick_attr);
				//}
         	}
    	}
    }
    else if(bright.ev > button )
    {
        if(fps_low == 1)
        {
            auto_fps_cnt++;
            if(auto_fps_cnt > 4)   // 10 fps
            {
            	dayMode = CURRENT_MODE_NIGHT;
                auto_fps_cnt = 0;
                QLOG(TRACE,"Set Fps To %d fps\n",SENSOR_FPS_NIGHT);
	 	        ret = IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_NIGHT, 1);
	 	        if(ret < 0){
	 	            QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS failed\n");
	 	            return -1;
	 	        }
				if (IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_NIGHT) != 0) {
					QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error\n");
					return -1;
				}
				fps_hig = 1;
				fps_low = 0;
				//low_fps = true;
                ivs_skipframe_set(SENSOR_FPS_NIGHT);
				//IMPISPAntiflickerAttr flick_attr = IMPISP_ANTIFLICKER_DISABLE; 
				//ret = IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr);
				//if(ret != 0){
				//	QLOG(TRACE,"IMP_ISP_Tuning_SetAntiFlickerAttr=%d failed !\n",flick_attr);
				//}
            }
        }
    }
    else
    {
    	//dayMode=CURRENT_MODE_DAY;
        auto_fps_cnt = 0;
    }
    return 0;
}
#endif
int YSX_IVS_Init(){
	 //int ret = -1;
#ifdef YSX_PERSONDET 
		printf("IAAC_Init for T30A~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		IAACInfo ainfo = {
			 .license_path = "/system_rw/perdetec/license.txt",
			  .cid = 370283637,
			  .fid = 2131859305,
			  .sn = "2b1f801d7d5c060be6bb3f6e4eba605c",
		};
	 
		 ret = IAAC_Init(&ainfo);
		  if (ret) {
			 IMP_LOG_ERR(TAG, "IAAC_Init error!\n");
			 IMP_LOG_ERR(TAG,"wo have not license to init IVS persondet\n");
			 return 0;//ret;
		 }
#endif
	return 0;
}
#include <imp/imp_osd.h>
IMPRgnHandle rHanderRect[2];

int drawRect(int grp_id,int x0 , int y0, int x1 , int y1)
{
	int ret;
		//	int x0, y0,x1,y1;
#if 0
	if (grp_id == 0) {
		x0 = 1920 * x0 / 640;
			y0 = 1080 * y0 / 360;
			x1 = 1920*x1 / 640;
			y1 = 1080*y1 / 360;
	}

#endif
	IMPRgnHandle handler = rHanderRect[grp_id];

	// rAttr.data.lineRectData.color = color;
	/*3. 设置OSD组区域属性和区域属性*/
	IMPOSDRgnAttr rAttrRect;
#if 0
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = x0;
	rAttrRect.rect.p0.y = y0;
	rAttrRect.rect.p1.x = x1;     //p0 is start锛宎nd p1 well be epual p0+width(or heigth)-1
	rAttrRect.rect.p1.y = y1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_RED;
#else
	IMP_OSD_GetRgnAttr(handler, &rAttrRect);
	rAttrRect.rect.p0.x = x0;
	rAttrRect.rect.p0.y = y0;
	rAttrRect.rect.p1.x = x1;    
	rAttrRect.rect.p1.y = y1;	
	rAttrRect.data.lineRectData.color = OSD_GREEN;
	if (grp_id == 0) {
		rAttrRect.data.lineRectData.linewidth = 4;
	}
    if (grp_id == 1)
    {
        rAttrRect.data.lineRectData.linewidth = 2;
    }
	else {
		rAttrRect.data.lineRectData.linewidth = 1;
	}
	
#endif 

	// 设置区域属性
	ret = IMP_OSD_SetRgnAttr(handler, &rAttrRect);
	if (ret < 0) {
		printf("IMP_OSD_SetRgnAttr Cover error !\n");
		return -1;
	}

	if (IMP_OSD_ShowRgn(handler, grp_id, 1) < 0) {
		printf("%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;
		
}

int draw_rect_osd_init(int grp)
{
	int ret;
	IMPRgnHandle handle;	
	IMPOSDRgnAttr rAttrRect;

	handle = IMP_OSD_CreateRgn(NULL);
	if (handle == INVHANDLE) {
		printf("IMP_OSD_CreateRgn Rect error !\n");
		return -1;
	}

	ret = IMP_OSD_RegisterRgn(handle, grp, NULL);
	if (ret < 0) {
		printf("IVS IMP_OSD_RegisterRgn failed\n");
		return -1;
	}

	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));

	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = 300;
	rAttrRect.rect.p0.y = 300;
	rAttrRect.rect.p1.x = rAttrRect.rect.p0.x + 600 - 1;
	rAttrRect.rect.p1.y = rAttrRect.rect.p0.y + 300 - 1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
//	rAttrRect.data.lineRectData.color = OSD_GREEN;
	rAttrRect.data.lineRectData.color = OSD_RED;
	rAttrRect.data.lineRectData.linewidth = 5;

#if 0
	if (channel == 0) {
		rAttrRect.data.lineRectData.linewidth = 1;
	}
	else {
		rAttrRect.data.lineRectData.linewidth = 1;
	}
#endif 

	ret = IMP_OSD_SetRgnAttr(handle, &rAttrRect);
	if (ret < 0) {
		printf("IMP_OSD_SetRgnAttr Rect error !\n");
		return -1;
	}
	IMPOSDGrpRgnAttr grAttrRect;

	if (IMP_OSD_GetGrpRgnAttr(handle, grp, &grAttrRect) < 0) {
		printf("IMP_OSD_GetGrpRgnAttr Rect error !\n");
		return -1;

	}
	memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
//	grAttrRect.show = 1;
	grAttrRect.show = 0;
	grAttrRect.layer = 1;
	grAttrRect.scalex = 1;
	grAttrRect.scaley = 1;
	if (IMP_OSD_SetGrpRgnAttr(handle, grp, &grAttrRect) < 0) {
		printf("IMP_OSD_SetGrpRgnAttr Rect error !\n");
		return -1;
	}


	ret = IMP_OSD_Start(grp);
	if (ret < 0) {
		printf("IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
		return -1;
	}

	rHanderRect[grp] = handle;
	return 0;
}
int drect_init = 0;
int draw_rect_osd_exit(int grp){
	if(drect_init == 0)
		return 0;
	int ret ;
	ret = IMP_OSD_ShowRgn(rHanderRect[grp], grp, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(rHanderRect[grp], grp);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
	}

	IMP_OSD_DestroyRgn(rHanderRect[grp]);
	printf("draw rect exit %d\n",grp);
	return 0; 
}

void not_match_key_drawrect(int inited){
	drect_init = inited;
	draw_rect_osd_init(0);
	draw_rect_osd_init(1);
    draw_rect_osd_init(2);
	drawRect(0,576,324,1728,972);
	drawRect(1,320,180,960,540);
    drawRect(2,160,90,480,270);

	return ;
}

