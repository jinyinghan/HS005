/*
 *   Author:xuc@yingshixun.com
 *   Date:2016.7
 *   Function: an example for h264 capture stream data
 */


//#include "sample-common.h"
#include "ysx_video.h"
#include <signal.h>

#define MAX_FRMS (15*100-1)
#define JPEG_ON 
//#define AE_GAIN

FILE *fp1=NULL,*fp2=NULL,*fp3=NULL;
int cnt1 = 0 ,cnt2 = 0,cnt3;

int chn_num;
void ysx_get_stream_cb0(const struct timeval *tv, const void *data, const int len, const int keyframe)
{   
    if(!fp1){
        fp1 = fopen("stream1.h265","wb+");
    }

    fwrite(data,1,len,fp1);
    fflush(fp1);
    if(cnt1 >= MAX_FRMS){
        fclose(fp1);
        printf("*********end time :sec = %d , usec = %d \n",tv->tv_sec,tv->tv_usec);
        
        exit(0);
    }
    cnt1++;

    /*Requst next frame will be I frame*/
//    QCamVideoInput_SetIFrame(chn_num);
}

void ysx_get_stream_cb1(const struct timeval *tv, const void *data, const int len, const int keyframe)
{   
	long time = tv->tv_sec * 1000 + tv->tv_usec /1000 ;
//	printf("time:%ld ,keyframe = %d, len = %d\n",time,keyframe,len);
	return ;

    if(!fp1){
        fp1 = fopen("stream1.h265","wb+");
    }

    fwrite(data,1,len,fp1);
    fflush(fp1);
    if(cnt1 >= MAX_FRMS){
        fclose(fp1);
        printf("*********end time :sec = %d , usec = %d \n",tv->tv_sec,tv->tv_usec);
        
        exit(0);
    }
    cnt1++;

    /*Requst next frame will be I frame*/
//    QCamVideoInput_SetIFrame(chn_num);
}



int main(int argc, char **argv)
{
    int ret;
    int buf_len;
    char pic_buf[1280*720*3];
    QCamVideoInputChannel ysx_chn[3];

    ysx_chn[0].bitrate = 1024*1.5;
    ysx_chn[0].cb = ysx_get_stream_cb0;
    ysx_chn[0].channelId = 0;
    ysx_chn[0].fps = 15;
    ysx_chn[0].gop = 1;
    ysx_chn[0].res = QCAM_VIDEO_RES_1080P;
    ysx_chn[0].vbr = 0;      /*choose CBR mode*/
    ysx_chn[0].payloadType = 1;


    ysx_chn[1].bitrate = 512;
    ysx_chn[1].cb = ysx_get_stream_cb1;
    ysx_chn[1].channelId = 1;
    ysx_chn[1].fps = 15;
    ysx_chn[1].gop = 1;
    ysx_chn[1].res = QCAM_VIDEO_RES_720P;
    ysx_chn[1].vbr = 0;      /*choose CBR mode*/
    ysx_chn[1].payloadType = 0;
	
    ysx_chn[2].bitrate = 512;
    ysx_chn[2].cb = ysx_get_stream_cb1;
    ysx_chn[2].channelId = 2;
    ysx_chn[2].fps = 15;
    ysx_chn[2].gop = 1;
    ysx_chn[2].res = QCAM_VIDEO_RES_720P;
    ysx_chn[2].vbr = 0;      /*choose CBR mode*/
    ysx_chn[2].payloadType = 0;
	
    chn_num = ysx_chn[0].channelId;
    /*step 1, input init*/
    ret = QCamVideoInput_Init();
    if(ret < 0){
        QLOG(FATAL,"QCamVideoInput_Init ERROR !\n");
        return -1;
    }
//    QCamVideoInput_SetInversion(false);
    /*step 2, add channel*/
    ret = QCamVideoInput_AddChannel(ysx_chn[0]);
    if(ret < 0){
        QLOG(FATAL,"QCamVideoInput_AddChannel[0] ERROR !\n");
        goto exit;
    }

   ret = QCamVideoInput_AddChannel(ysx_chn[1]);
    if(ret < 0){
        QLOG(FATAL,"QCamVideoInput_AddChannel[0] ERROR !\n");
        goto exit;
    }
//	ret = QCamVideoInput_AddChannel(ysx_chn[2]);
	 if(ret < 0){
		 QLOG(FATAL,"QCamVideoInput_AddChannel[0] ERROR !\n");
		 goto exit;
	 }

    /*step 3, start stream */
    ret = QCamVideoInput_Start();
    if(ret < 0){
        QLOG(FATAL,"QCamVideoInput_Start ERROR !\n");
        goto exit;
    }
	
#ifdef JPEG_ON    
  
    sleep(1);
  //  printf("IR mode = %d\n",QCamGetIRMode());
    memset(pic_buf,0,1280*720*3);
    if(QCamVideoInput_CatchJpeg(pic_buf,&buf_len) == 0){
        printf("Save Picture !\n");
        FILE *fp = fopen("2016-7-18.jpg","wb+");
        fwrite(pic_buf,1,buf_len,fp);
        fclose(fp);
    }
#endif

#ifdef AE_GAIN
    IMPISPEVAttr bright;
   // bright.

    while(1){
        sleep(1);
        IMP_ISP_Tuning_GetEVAttr(&bright);
        printf("bright is %d\n",bright.ev);
    }
#endif

    sleep(6);
    QCamVideoInput_SetInversion(true);
//    QCamVideoInput_SetBitrate(ysx_chn[0].channelId,256,ysx_chn[0].vbr);

//    int i;
//    for(i = 0;i< 4;i++){
//        
//        IMP_Encoder_RequestIDR(ysx_chn[0].channelId);
//        sleep(1);
//    }
    
   // sleep(5);
    while(1);

exit:    
    printf("ready to exit!\n");

    
    /*step 4, stop stream and uninit */
    QCamVideoInput_Uninit();

   
    return 0;
}
