
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>

#include "rtspservice.h"
#include "rtputils.h"
#include "ringfifo.h"
//#include "sample_comm.h"

extern int g_s32Quit ;
#if 0
VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
static pthread_t gs_VencPid;
static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
#endif 

/**************************************************************************************************
**
**
**
**************************************************************************************************/
#define FILE_NAME_LEN 256


#if 0
HI_VOID* SAMPLE_COMM_VENC_GetVencStreamProc_RTSP(HI_VOID* p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    VENC_CHN_ATTR_S stVencChnAttr;
    SAMPLE_VENC_GETSTREAM_PARA_S* pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_CHAR aszFileName[VENC_MAX_CHN_NUM][FILE_NAME_LEN];
    FILE* pFile[VENC_MAX_CHN_NUM];
    char szFilePostfix[10];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];

    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*)p;
    s32ChnTotal = pstPara->s32Cnt;

    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    if (s32ChnTotal >= VENC_MAX_CHN_NUM)
    {
        SAMPLE_PRT("input count invaild\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++)
    {
        /* decide the stream file name, and open file to save stream */
        VencChn = i;
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                       VencChn, s32Ret);
            return NULL;
        }
        enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

        s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n", \
                       stVencChnAttr.stVeAttr.enType, s32Ret);
            return NULL;
        }
        snprintf(aszFileName[i], FILE_NAME_LEN, "stream_chn%d%s", i, szFilePostfix);
		
        /*pFile[i] = fopen(aszFileName[i], "wb");
        if (!pFile[i])
        {
            SAMPLE_PRT("open file[%s] failed!\n",
                       aszFileName[i]);
            return NULL;
        }*/

        /* Set Venc Fd. */
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                       VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == pstPara->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
            for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));
                    s32Ret = HI_MPI_VENC_Query(i, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.2 : malloc corresponding number of pack nodes.
                    *******************************************************/
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }

                    /*******************************************************
                     step 2.3 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                                   s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.4 : save frame to file
                    *******************************************************/
                    HisiPutH264DataToBuffer(&stStream);
                    /*s32Ret = SAMPLE_COMM_VENC_SaveStream(enPayLoadType[i], pFile[i], &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("save stream failed!\n");
                        break;
                    }*/
                    /*******************************************************
                     step 2.5 : release stream
                    *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
                    /*******************************************************
                     step 2.6 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    }

    /*******************************************************
    * step 3 : close save-file
    *******************************************************/
    for (i = 0; i < s32ChnTotal; i++)
    {
        fclose(pFile[i]);
    }

    return NULL;
}

/******************************************************************************
* funciton : start get venc stream process thread
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_StartGetStream_RTSP(HI_S32 s32Cnt)
{
    gs_stPara.bThreadStart = HI_TRUE;
    gs_stPara.s32Cnt = s32Cnt;

    return pthread_create(&gs_VencPid, 0, SAMPLE_COMM_VENC_GetVencStreamProc_RTSP, (HI_VOID*)&gs_stPara);
}

/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
HI_VOID* SAMPLE_VENC_1080P_CLASSIC_RTSP(HI_VOID* p)
{
    PAYLOAD_TYPE_E enPayLoad [3]= {PT_H265, PT_H265, PT_H264};
    PIC_SIZE_E enSize[3]= {PIC_HD1080, PIC_HD1080, PIC_D1};
    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig = {0};

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;

    HI_S32 s32ChnNum;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c;

     s32ChnNum = 1;
    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 12;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble        = HI_FALSE;
    stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width       = stSize.u32Width;
    stVpssChnMode.u32Height      = stSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
	
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    /******************************************
     step 5: start stream venc
    ******************************************/
    /*** HD1080P **/
    printf("\t c) cbr.\n");
            enRcMode = SAMPLE_RC_CBR;
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0], \
                                    gs_enNorm, enSize[0], enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream_RTSP(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);


    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
END_VENC_1080P_CLASSIC_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return;
}
#endif 

#if 0
int main(int argc, char* argv[])
{
	int s32MainFd,temp;
//	printf("size of %d",sizeof(HI_U16));
	struct timespec ts = { 0, 200000000 };
	pthread_t id;
	ringmalloc(720*5760);//def:ringmalloc(720*576):720P, ringmalloc(1920*1280):1080P
	printf("RTSP server START\n");
	PrefsInit();
	printf("listen for client connecting...\n");
	signal(SIGINT, IntHandl);
	s32MainFd = tcp_listen(SERVER_RTSP_PORT_DEFAULT);
	if (ScheduleInit() == ERR_FATAL)
	{
		fprintf(stderr,"Fatal: Can't start scheduler %s, %i \nServer is aborting.\n", __FILE__, __LINE__);
		return 0;
	}
	RTP_port_pool_init(RTP_DEFAULT_PORT);
//	pthread_create(&id,NULL, SAMPLE_VENC_1080P_CLASSIC_RTSP,NULL);
	while (!g_s32Quit)
	{
		nanosleep(&ts, NULL);
		EventLoop(s32MainFd);
	}
	sleep(2);
	ringfree();
	printf("The Server quit!\n");

	return 0;
}
#endif 

#include "ysx_video.h"
#include "queue_buf.h"
void stream_main_cb( const struct timeval *tv, const void *data, const int len, const int keyframe)
{
//    printf("channel: %d\n", channel);
    int ret = 0;
	uint64_t tmp_pts = tv->tv_sec;tmp_pts *= 1000;tmp_pts += tv->tv_usec / 1000;  
	//printf("timestamp: %lld\n", tmp_pts);


#ifdef USE_QUEUE_BUF   
    if (data && len > 0) {
        ret = queue_push(&g_video_stream[0].buf, data, len);
        if (ret < 0) {
            printf("queue push failed!\n");
        }
    }

	sleep(5);

    static FILE * fp = NULL;
    static int isfisrt = 1;

    if (isfisrt) {
        fp = fopen("stream-0-f0.h265", "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    fwrite(data, sizeof(char), len, fp);
    fflush(fp);
#endif   
}


void stream_second_cb( const struct timeval *tv, const void *data, const int len, const int keyframe)
{
//    printf("channel: %d\n", channel);

    int ret = 0;
#ifdef USE_QUEUE_BUF   
    if (data && len > 0) {
        ret = queue_push(&g_video_stream[1].buf, data, len);
        if (ret < 0) {
            printf("queue push failed!\n");
        }
    }
    static FILE * fp = NULL;
    static int isfisrt = 1;

    if (isfisrt) {
        fp = fopen("stream-f1.h265", "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    fwrite(data, sizeof(char), len, fp);
    fflush(fp);
#endif 
}



void stream_third_cb( const struct timeval *tv, const void *data, const int len, const int keyframe)
{
//    printf("channel: %d\n", channel);

    int ret = 0;
#ifdef USE_QUEUE_BUF   
    if (data && len > 0) {
        ret = queue_push(&g_video_stream[2].buf, data, len);
        if (ret < 0) {
            printf("queue push failed!\n");
        }
    }

    static FILE * fp = NULL;
    static int isfisrt = 1;

    if (isfisrt) {
        fp = fopen("stream-2-f2.h265", "wb");
        if (NULL == fp) {
            ret = errno > 0 ? errno : -1;
            fprintf(stderr, "open file failed with: %s!\n", strerror(ret));
            exit(-1);
        }
        isfisrt = 0;
    }

    fwrite(data, sizeof(char), len, fp);
    fflush(fp);
#endif 
}

void Md2Snapshot()
{
        int len = 400*1024;

        char *pic_buff = (char *)malloc(len);
        if(NULL == pic_buff){
                printf("malloc buffer Err with %s\n",strerror(errno));
                return;
        }

        if(QCamVideoInput_CatchJpeg(pic_buff,&len) == 0)
        {
                FILE *fp = fopen("/tmp/snap_shot.jpg","wb+");
                fwrite(pic_buff,1,len,fp);
                fclose(fp);
        }
        free(pic_buff);
        return;
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
     //sleep(1);
 
 }
void video_motion_cb(int value, int irmode)
{
    printf("motion type: %d, ircut mode: %d\n", value, irmode);
}


#ifdef USE_IVS_FREE
#include <qcam_smart.h>
#endif

int main(int argc, char ** argv)
{
	QCamVideoInputChannel ysx_chn[3];
	ysx_chn[0].bitrate = 1600;
	ysx_chn[0].cb = stream_main_cb;
	ysx_chn[0].channelId = 0;
	ysx_chn[0].fps = 15;
	ysx_chn[0].gop = 2;
	ysx_chn[0].res = QCAM_VIDEO_RES_1296P;//QCAM_VIDEO_RES_1080P;
	ysx_chn[0].vbr = 0;	   
	ysx_chn[0].encoding = 0;//IMP_ENC_PROFILE_HEVC_MAIN;//

	ysx_chn[1].bitrate = 1024;
	ysx_chn[1].cb = stream_second_cb;
	ysx_chn[1].channelId = 1;
	ysx_chn[1].fps = 15;
	ysx_chn[1].gop = 1;
	ysx_chn[1].res = QCAM_VIDEO_RES_720P;//QCAM_VIDEO_RES_720P;//
	ysx_chn[1].vbr = 0;	   
	ysx_chn[1].encoding = 0;//IMP_ENC_PROFILE_HEVC_MAIN;

	ysx_chn[2].bitrate = 128;
	ysx_chn[2].cb = stream_third_cb;
	ysx_chn[2].channelId = 2;
	ysx_chn[2].fps = 15;
	ysx_chn[2].gop = 1;
	ysx_chn[2].res = QCAM_VIDEO_RES_360P;
	ysx_chn[2].vbr = 0;	   
	ysx_chn[2].encoding = 0;//IMP_ENC_PROFILE_HEVC_MAIN;

	
	int ret = QCamVideoInput_Init();
	if(ret < 0){
		QLOG(FATAL,"QCamVideoInput_Init ERROR !\n");
		return -1;
	}
	int index;
	for (index = 0; index < MAX_FS; index ++) {
		ret = QCamVideoInput_AddChannel(ysx_chn[index]);
		if (ret < 0) {
			QLOG(FATAL,"QCamVideoInput_AddChannel[0] ERROR !\n");
			goto exit;
		}
		EMGCY_LOG("add video channel %02d over...\n", index);
	}

	ret = QCamVideoInput_Start();
	if(ret < 0){
		QLOG(FATAL,"QCamVideoInput_Start ERROR !\n");
		goto exit;
	}
	
	QCamVideoInputOSD osd_attr;
    memset(&osd_attr,0,sizeof(QCamVideoInputOSD));
    osd_attr.pic_enable = 0;
    osd_attr.time_enable = 1;

  

    osd_attr.time_x = 0; //1920-19*24 = 1464
    osd_attr.time_y = 0;

    ret = QCamVideoInput_SetOSD(0, &osd_attr);
    if(ret < 0){
       printf("QCamVideoInput_SetOSD ERROR !\n");
       return -1;
    }
	
	ret = QCamVideoInput_SetOSD(1, &osd_attr);
    if(ret < 0){
       printf("QCamVideoInput_SetOSD ERROR !\n");
       return -1;
    }

    ret = QCamVideoInput_SetOSD(2, &osd_attr);
  	if(ret < 0){
   		printf("QCamVideoInput_SetOSD ERROR !\n");
        return -1;
   	}   
   	QCamSetLightDetectCallback(has_light_cb);
	#ifdef USE_IVS_FREE
	QCamSmartInParam pInParam;
	memset(&pInParam,0,sizeof(pInParam));
	pInParam.width = 640;
	pInParam.height = 360;
	QCamSmartCreate(&pInParam);
	#endif
   	sleep(2);
	QCamInitMotionDetect2(video_motion_cb);
    //Md2Snapshot();  
	sleep(4);

	QCamVideoInput_SetBitrate(0, 512, 0);
	QCamVideoInput_SetBitrate(1, 128, 0);
	QCamVideoInput_SetFps(0,12,2);
	QCamVideoInput_SetInversion(1);
	QCamVideoInput_SetIFrame(0);
	char buf[1280*720*3/2] = {0};
	memset(buf,0,sizeof(buf));
	QCamVideoInput_CatchYUV(1920, 1080, buf, 1280*720*3/2);
	printf("%d\n",buf[0]);
	int light = QCamVideoInput_HasLight();
	printf("%d\n",light);
	char bufs[10];
	memset(bufs,0,sizeof(bufs));
	QCamGetSensorType(bufs);
	printf("%d\n",bufs[0]);
	sleep(10);
	QCamVideoInput_SetResolution(0, QCAM_VIDEO_RES_720P);
	sleep(3);
	QCamVideoInput_SetCodec(0, 1);
	Md2Snapshot();
	sleep(10);

	#ifdef USE_IVS_FREE
	QCamSmartDestory();
	#endif
	QCamUninitMotionDetect();
	QCamVideoInput_Uninit();
exit:
	return 0;
}
