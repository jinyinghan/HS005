/*
 * Ingenic IMP RTSPServer VideoInput
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define  QCAM_AV_C_API
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>

#include "qcam.h"
#include "qcam_audio_input.h"
#include "qcam_audio_output.h"
#include "qcam_log.h"
#include "qcam_motion_detect.h"
#include "qcam_sys.h"
#include "qcam_video_input.h"

#include <fstream>

#include "bitmapinfo.h"
#include "logodata_100x100_bgra.h"

#include "Options.hh"
#include "VideoInput.hh"
#include "H264VideoStreamSource.hh"

#define TAG 						"sample-RTSPServer"

#define IVS_MOVE 	0x00
#define IVS_FACE 	0x01
#define IVS_FIGURE  0x02

static int ivs_flag = 0;
//extern void 
/* Encoder param */
/* chn0:main-0 h264, chn1:second h264, chn2:main-0 jpeg, chn3:main-1 h264 */

Boolean VideoInput::fHaveInitialized = False;

LNode gLinkListHead;
int start_flag = 0;
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;


VideoInput* VideoInput::createNew(UsageEnvironment& env, int streamNum) {
    if (!fHaveInitialized) {
        if (!initialize(env)) return NULL;
        fHaveInitialized = True;
    }

    VideoInput *videoInput = new VideoInput(env, streamNum);

    return videoInput;
}
void osd_on()
{
    //	set_osd_rgn(0,"logo.bmp",0,100);
    set_osd_rect(0);
}


VideoInput::VideoInput(UsageEnvironment& env, int streamNum)
    : Medium(env), fVideoSource(NULL), fpsIsStart(False), fontIsStart(False),
    osdIsStart(False), osdStartCnt(0), nrFrmFps(0),
    totalLenFps(0), startTimeFps(0), streamNum(streamNum), scheduleTid(-1),
    ivsTid(-1), orgfrmRate(gconf_FPS_Num), hasSkipFrame(false) {
        //	memset(ivsIsStart, 0, sizeof(ivsIsStart));

    }

VideoInput::~VideoInput() {	
    /*step 4, stop stream and uninit */
    QCamVideoInput_Uninit();
}
/*
enum NALUnitType {
    NAL_TRAIL_N    = 0,
    NAL_TRAIL_R    = 1,
    NAL_TSA_N      = 2,
    NAL_TSA_R      = 3,
    NAL_STSA_N     = 4,
    NAL_STSA_R     = 5,
    NAL_RADL_N     = 6,
    NAL_RADL_R     = 7,
    NAL_RASL_N     = 8,
    NAL_RASL_R     = 9,
    NAL_BLA_W_LP   = 16,
    NAL_BLA_W_RADL = 17,
    NAL_BLA_N_LP   = 18,
    NAL_IDR_W_RADL = 19,
    NAL_IDR_N_LP   = 20,
    NAL_CRA_NUT    = 21,
    NAL_VPS        = 32,
    NAL_SPS        = 33,
    NAL_PPS        = 34,
    NAL_AUD        = 35,
    NAL_EOS_NUT    = 36,
    NAL_EOB_NUT    = 37,
    NAL_FD_NUT     = 38,
    NAL_SEI_PREFIX = 39,
    NAL_SEI_SUFFIX = 40,
};

static int hevc_probe(const void* pbuf, int buf_size)
{
    unsigned int code = -1;
    int vps = 0, sps = 0, pps = 0, irap = 0;
    int i;

    for (i = 0; i < buf_size - 1; i++) {
        code = (code << 8) + pbuf[i];
        if ((code & 0xffffff00) == 0x100) {
            char nal2 = pbuf[i + 1];
            int type = (code & 0x7E) >> 1;

            if (code & 0x81) 
                return 0;

            if (nal2 & 0xf8) 
                return 0;

            switch (type) {
                case NAL_VPS:        vps++;  break;
                case NAL_SPS:        sps++;  break;
                case NAL_PPS:        pps++;  break;
                case NAL_BLA_N_LP:
                case NAL_BLA_W_LP:
                case NAL_BLA_W_RADL:
                case NAL_CRA_NUT:
                case NAL_IDR_N_LP:
                case NAL_IDR_W_RADL: irap++; break;
            }
        }
    }

    return irap;
}
*/

void ysx_video_cb(const struct timeval *tv, const void *data, const int len, const int keyframe)
{
    if(start_flag == 0)
        return;

    int link_length = GetLength(&gLinkListHead);
    if( link_length > MAX_VID_BUF)
    {
        /*drop the frame */
        fprintf(stderr, "Now video length is %d, we will drop this frame\n", link_length);
        return ;
    }

    pthread_mutex_lock(&list_lock);
    /*
    if(hevc_probe(data,len)){
        printf("key frame!!!!!!!!!!!!!\n");
        pkt->keyframe = 1;
    }
    */
    ////printf("%d\n",len);
    AVPacket *pkt = (AVPacket *)malloc(sizeof(AVPacket));
    pkt->data = (uint8_t *)malloc(len);
    pkt->size = len;
    memcpy(pkt->data,data,len);

    InsertNode(&gLinkListHead,1,pkt);
    pthread_mutex_unlock(&list_lock);

}

void *save_vid_stream(void *arg)
{
    FILE *fp ;
    int link_length = -1;
    LNode *list;

    fp = fopen("stream.h264","w");
    if(!fp)
    {
        printf("open file stream.h264 error !\n");
        return 0;
    }

    list = &gLinkListHead;
    while(1)
    {

        link_length = GetLength(list);
        if(link_length)
        {

            pthread_mutex_lock(&list_lock);
            AVPacket *pkt = GetNodePkt(list, link_length);
            printf("%s:link_length = %d , getNode:size = %d\n",__FUNCTION__,link_length,pkt->size);	
            free(pkt->data);
            free(pkt);
            DeleteNode(list, link_length);
            pthread_mutex_unlock(&list_lock);			
        }
        else
            usleep(500*1000);

    }
    fclose(fp);

}
static int switch_status = 0;
void has_light_cb(int has_light){
    printf("[%s][%d] %d\n",__FUNCTION__,__LINE__,has_light);
    if(has_light == 1 && switch_status == 1){
       QCamSetIRCut(0);
       switch_status = 0;
    
    }else if(has_light == 0 && switch_status == 0){
      QCamSetIRCut(1);
      switch_status = 1;

    }
    sleep(1);

}
#include <qcam_smart.h>
bool VideoInput::initialize(UsageEnvironment& env) {
    int ret;

    ret = InitList(&gLinkListHead);
    if(ret == false){
        printf("InitList ERROR !\n");
        return false;
    }

    QCamVideoInputChannel ysx_chn;

    ysx_chn.bitrate = videoBitrate;//1024*1.5;
    ysx_chn.cb = ysx_video_cb;
    ysx_chn.channelId = 1; ///CHN_MAIN;
    ysx_chn.fps = gconf_FPS_Num;
    ysx_chn.gop = 1;
    ysx_chn.res = QCAM_VIDEO_RES_360P;//resolution;
    ysx_chn.vbr = 0;      /*choose CBR mode*/
    ysx_chn.encoding = 0;


    /*step 1, input init*/
    ret = QCamVideoInput_Init();
    if(ret < 0){
        printf("QCamVideoInput_Init ERROR !\n");
        return false;
    }

    /*step 2, add channel*/
    ret = QCamVideoInput_AddChannel(ysx_chn);
    if(ret < 0){
        printf("QCamVideoInput_AddChannel ERROR !\n");
        return false;
    }
    /*step 3, start stream */
    ret = QCamVideoInput_Start();
    if(ret < 0){
        printf("QCamVideoInput_Start ERROR !\n");
        return -1;
    }
    /*
    QCamSmartInParam pInParam;
    memset(&pInParam,0,sizeof(pInParam));
    QCamSmartCreate(&pInParam);
    */
    /*step 4 ,add osd */
    QCamVideoInputOSD osd_attr;
    memset(&osd_attr,0,sizeof(QCamVideoInputOSD));
    osd_attr.pic_enable = 0;
    osd_attr.time_enable = 1;

    if(resolution == QCAM_VIDEO_RES_1080P)
    {
        osd_attr.time_x = 1464; //1920-19*24 = 1464
        osd_attr.time_y = 0;
    }else{
        osd_attr.time_x = 10; //1280-19*16 = 976
        osd_attr.time_y = 0;
    }
    ret = QCamVideoInput_SetOSD(CHN_MAIN,&osd_attr);
    if(ret < 0){
        printf("QCamVideoInput_Start ERROR !\n");
        return -1;
    }

    ///QCamVideoInput_SetInversion(gconf_Inversion);

    //	osd_on();
    ///QCamSetIRMode((QCAM_IR_MODE)IRMode);
    ///QCamSetLightDetectCallback(has_light_cb);

    //	pthread_t save_id;

    //	ret = pthread_create(&save_id, NULL, save_vid_stream, NULL);
    //	if (ret < 0) {
    //		printf("Failed Create pthread to save_vid_stream with result %d\n",ret);
    //		return false;
    //	}	

    return true;
}

FramedSource* VideoInput::videoSource() {
    IMP_Encoder_FlushStream(streamNum);
    fVideoSource = new H264VideoStreamSource(envir(), *this);
    return fVideoSource;
}

int VideoInput::getStream(void* to, unsigned int* len, struct timeval* timestamp, unsigned fMaxSize) {
    int ret;
    int link_length = -1;
    LNode *list;
    int length = 0;

    list = &gLinkListHead;
    link_length = GetLength(list);
    if(link_length)
    {

        pthread_mutex_lock(&list_lock);
        AVPacket *pkt = GetNodePkt(list, link_length);
        //		printf("%s:link_length = %d , getNode:size = %d\n",__FUNCTION__,link_length,pkt->size);	
        if(pkt->size > fMaxSize)
        {
            printf("drop frame:length=%d, fMaxSize=%d\n", pkt->size, fMaxSize);
            length = 0;			
        }else{
            memcpy(to,pkt->data,pkt->size);
            length = pkt->size;
        }
        *len = length;
        free(pkt->data);
        free(pkt);
        DeleteNode(list, link_length);		
        pthread_mutex_unlock(&list_lock);
    }

    gettimeofday(timestamp, NULL);
    return 0;
}

int VideoInput::pollingStream(void)
{
    int link_length = -1;
    LNode *list;

    list = &gLinkListHead;

    link_length = GetLength(list);
    //	printf("poll stream lenght = %d\n",link_length);
    while(link_length == 0){
        usleep(100*1000);
        link_length = GetLength(list);
        //		printf("poll stream lenght = %d\n",link_length);
    }

    return 0;
}
int VideoInput::streamOn(void)
{
    start_flag = 1;
    QCamVideoInput_SetIFrame(CHN_MAIN);
    return 0;
}

int VideoInput::streamOff(void)
{
    int ret;
    start_flag = 0;
    return 0;
}

