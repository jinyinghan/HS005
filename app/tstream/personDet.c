#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <imp/imp_osd.h>

#include <sys/prctl.h>

//#include <ivs/ivs_inf_propersonDet.h>
#include<ivs/ivs_inf_custpersonDet.h>
//#include <ivs/ivs_inf_personDet.h>
#include <imp/imp_ivs.h>
#include <iaac.h>
//#include "sample-common.h"

#define TAG "YSX-PERSONDET"
#define TIME_OUT 1500
#include "motion_track.h"
#include "common_env.h"
#define rshu 5
IMPRgnHandle rHanderRect[VIDEO_STREAM_NUM][rshu];
struct osd_rect rect[VIDEO_STREAM_NUM][rshu];

/*
   int ivs_persondet_init(int grp_num) {
   int ret = 0;

   ret = IMP_IVS_CreateGroup(grp_num);
   if (ret < 0) {
   IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
   return -1;
   }

   IAACInfo ainfo = {
   .license_path = "/system_rw/perdetec/license.txt",
   .cid = 370283637,
   .fid = 2131859305,
   .sn = "2b1f801d7d5c060be6bb3f6e4eba605c",
   };

   ret = IAAC_Init(&ainfo);
   if (ret) {
   IMP_LOG_ERR(TAG, "IAAC_Init error!\n");
   return -1;
   }

   return 0;
   }

   int ivs_persondet_exit(int grp_num) {
   int ret = 0;

   IAAC_DeInit();

   ret = IMP_IVS_DestroyGroup(grp_num);
   if (ret < 0) {
   IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
   return -1;
   }
   return 0;
   }
   */
int ivs_persondet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
        int ret = 0;

        //persondet_param_input_t param;
        //propersondet_param_input_t param;
		custpersondet_param_input_t param;
		
        memset(&param, 0, sizeof(custpersondet_param_input_t));
        param.frameInfo.width = 640; //800;
        param.frameInfo.height = 360;; //800;

        param.skip_num = 3;  ///skip num
        param.ptime = false;  ///print time or not
       // param.rwidth = 640;  ///real detect width
       // param.sense = 2;  ///detection sensibility
       param.rotate = false; // true rotate -90 or 270, false no rotate 
       param.max_face_box = 5;
       param.scale_factor = 1.0;//0.8; 
	   //param.switch_face_pose = true;
       //param.switch_face_blur = false;
 
       // *interface = PersonDetInterfaceInit(&param);
       *interface = CustPersonDetInterfaceInit(&param);
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

int ivs_persondet_stop(int chn_num, IMPIVSInterface *interface) {
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

        //PersonDetInterfaceExit(interface);
        CustPersonDetInterfaceExit(interface);

        return 0;
}
static int drawRect(int grp_id, int personNum, int x0, int y0, int x1, int y1)
{
        int ret;
        // printf("x0 %d y0 %d x1 %d y1 %d\n",x0,y0,x1,y1);
        if (0 == grp_id) {
                x0 = 1920 * x0 / 640;
                y0 = 1080 * y0 / 360;	
                x1 = 1920 * x1 / 640;
                y1 = 1080 * y1 / 360;
        }
        else if (1 == grp_id) {
                /*  x0 = 1280 * x0 / 640;
                    y0 = 720 * y0 / 360;	
                    x1 = 1280 * x1 / 640;
                    y1 = 720 * y1 / 360;
                    */
        }


        // ÇøÓò¾ä±ú
        IMPRgnHandle handler = rHanderRect[grp_id][personNum];
        // ÉèÖÃOSD×éÇøÓòÊôÐÔºÍÇøÓòÊôÐÔ
        // OSDÇøÓòÊôÐÔÊý¾Ý
        IMPOSDRgnAttr rAttrRect;
        memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
        rAttrRect.type = OSD_REG_RECT;
        rAttrRect.rect.p0.x = x0;
        rAttrRect.rect.p0.y = y0;
        rAttrRect.rect.p1.x = x1;     //p0 is starté”›å®Žnd p1 well be epual p0+width(or heigth)-1
        rAttrRect.rect.p1.y = y1;
        rAttrRect.fmt = PIX_FMT_MONOWHITE;
        rAttrRect.data.lineRectData.color = OSD_RED;
        if (grp_id == 0) {
                rAttrRect.data.lineRectData.linewidth = 10;//rect[i].linewidth;
        }
        else {
                rAttrRect.data.lineRectData.linewidth = 3;//rect[i].linewidth;
        }
		
        ret = IMP_OSD_SetRgnAttr(handler, &rAttrRect);
        if (ret < 0) {
                LOG("IMP_OSD_SetRgnAttr Cover error !\n");
                return -1;
        }

        // ÉèÖÃ×éÇøÓòÊÇ·ñÏÔÊ¾
        if (IMP_OSD_ShowRgn(handler, grp_id, 1) < 0) {
                LOG("%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
                return -1;
        }

        return 0;
}
int set_osd_rect(uint8_t grp_id)
{
        int ret;
        int i;
        //osd_rect_extract();

        /*1.´´½¨OSDÇøÓò*/
        for(i=0;i<rshu;i++){
                rHanderRect[grp_id][i] = IMP_OSD_CreateRgn(NULL);     //cover
                if (rHanderRect[grp_id][i] < 0) {
                        printf("IMP_OSD_CreateRgn%d failed\n",rHanderRect[grp_id][i]);
                        return -1;
                }

                /*2.×¢²áOSDÇøÓòµ½OSD×éÖÐ*/
                ret = IMP_OSD_RegisterRgn(rHanderRect[grp_id][i], grp_id, NULL);
                if (ret < 0) {
                        printf("IVS IMP_OSD_RegisterRgn failed\n");
                        return -1;
                }

                /*3. ÉèÖÃOSD×éÇøÓòÊôÐÔºÍÇøÓòÊôÐÔ*/
                IMPOSDRgnAttr rAttrRect;
                memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
                rAttrRect.type = OSD_REG_RECT;
                rAttrRect.rect.p0.x = 0;//rect[grp_id][i].x0;
                rAttrRect.rect.p0.y = 0;//rect[grp_id][i].y0;
                rAttrRect.rect.p1.x = 0;//rect[grp_id][i].x1;     //p0 is startï¼Œand p1 well be epual p0+width(or heigth)-1
                rAttrRect.rect.p1.y = 0;//rect[grp_id][i].y1;
                rAttrRect.fmt = PIX_FMT_MONOWHITE;
                rAttrRect.data.lineRectData.color = OSD_BLACK;
				if(grp_id == 0)
                rAttrRect.data.lineRectData.linewidth = 10;//rect[grp_id][i].linewidth;
                else
				rAttrRect.data.lineRectData.linewidth = 3;  
                ret = IMP_OSD_SetRgnAttr(rHanderRect[grp_id][i], &rAttrRect);
                if (ret < 0) {
                        printf("IMP_OSD_SetRgnAttr Cover error !\n");
                        return -1;
                }
                IMPOSDGrpRgnAttr grAttrRect;
                memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
                grAttrRect.show = 1;//1;


                grAttrRect.gAlphaEn = 1;
                grAttrRect.fgAlhpa = 0xff;

                grAttrRect.layer =  i+rshu*grp_id;//i*(grp_id+1); //1+i*(grp_id+1);//i+grp_id;//1+grp_id;
                grAttrRect.scalex = 1;//1;//grp_id;//1;
                grAttrRect.scaley = 1;//1;//grp_id;//1;

                if (IMP_OSD_SetGrpRgnAttr(rHanderRect[grp_id][i], grp_id, &grAttrRect) < 0) {
                        LOG("IMP_OSD_SetGrpRgnAttr Cover error !\n");
                        return -1;
                }
        }
        ret = IMP_OSD_Start(grp_id);
        if (ret < 0) {
                LOG("IMP_OSD_SetGrpRgnAttr failed\n");
                return -1;
        }

        return 0;
}
int resetRectshow(int grp_id,int personNum)
{
        int ret,i;
        IMPOSDRgnAttr rAttrRect;

        IMPRgnHandle handler = rHanderRect[grp_id][personNum];
        // rAttr.data.lineRectData.color = color;
        /*3. è®¾ç½®OSDç»„åŒºåŸŸå±žæ€§å’ŒåŒºåŸŸå±žæ€§*/
        memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
        rAttrRect.type = OSD_REG_RECT;
        rAttrRect.rect.p0.x = 0;
        rAttrRect.rect.p0.y = 0;
        rAttrRect.rect.p1.x = 0;     //p0 is starté”›å®Žnd p1 well be epual p0+width(or heigth)-1
        rAttrRect.rect.p1.y = 0;
        rAttrRect.fmt = PIX_FMT_MONOWHITE;
        rAttrRect.data.lineRectData.color = OSD_BLACK;
		if(grp_id == 0)
        rAttrRect.data.lineRectData.linewidth = 10;//rect[grp_id][personNum].linewidth;//3;//rect[i].linewidth;
        else
		rAttrRect.data.lineRectData.linewidth = 3;
        ret = IMP_OSD_SetRgnAttr(rHanderRect[grp_id][personNum], &rAttrRect);
        if (ret < 0) {
                LOG("IMP_OSD_SetRgnAttr Cover error !\n");
                return -1;
        }

        if (IMP_OSD_ShowRgn(handler, grp_id, 0) < 0) {
                LOG("%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
                return -1;
        }

        return 0;
}
static int set_timer_disable_HBD(int iSetTime, int iResetTimeFlag)
{
        static int iIsStart = 1;
        static time_t tStartTime, tEndTime;

        if (iResetTimeFlag == 1){
                iIsStart = 1;
                return 0;
        }

        if (iIsStart == 1){
                tStartTime = time(NULL);
                iIsStart = 0;
        }else{
                /* Obtain time. */
                tEndTime = time(NULL);
                if ((tEndTime - tStartTime) >= iSetTime){
                        printf("Wait about %d sec. Disable HBD\n", iSetTime);
                        iIsStart = 1;
                        return 1;
                }
        }
        return 0;
}

extern pthread_mutex_t  body_signal_t;
extern pthread_cond_t body_signal_cond ;
extern int g_iIsMotion;
#include"ptz.h"

void * perdetect_thread(void *arg)
{
        IMPIVSInterface *inteface = NULL;
        //persondet_param_output_t *result = NULL;
        custpersondet_param_output_t *result = NULL;

        int ret;
        int chn_num = 1;
        int i,j=0;
        int grp_id = 0;
        long max_area = 0;
        long tmp_area = 0;
        int max_size_number;
        int *sense = (int*)arg;
        int index = 0;
        int pre_res = 0;
        unsigned long loop_count = 0;
        int ulx=0,brx=0,uly=0,bry=0;	 
        static int lastNum=0;
        //int ulx=0;

		while(!getLicenseFile())
			 usleep(100*1000);
        ivs_persondet_start(0, 1, &inteface);
        for (index; index < VIDEO_STREAM_NUM; index ++) {
                set_osd_rect(index);
        }
        LOG("#### perDetect start ...\n");

        //g_enviro_struct.motion_track.started = 1;

        prctl(PR_SET_NAME, "perdetect_thread");
		

        while(g_enviro_struct._loop)
        {   
        		#if 0
                if (VIDEO_QUALITY_TYPE_MAIN == GetYSXCtl(YSX_VQUALITY)) {
                        grp_id = VIDEO_STREAM_MAIN;
                }
                /*
                   else if (VIDEO_QUALITY_TYPE_SECOND == GetYSXCtl(YSX_VQUALITY)) {
                   grp_id = VIDEO_STREAM_SECOND;
                   }
                   */
                else if (VIDEO_QUALITY_TYPE_THREE == GetYSXCtl(YSX_VQUALITY)) {
                        grp_id = VIDEO_STREAM_SECOND;
                }  
				#endif
#if 0
                if(!g_iIsMotion){
                        //printf("g_iIsMotion:%d\n",g_iIsMotion);
                        usleep(100*1000);
                        continue;
                }
#endif

                ret = IMP_IVS_PollingResult(IVS_CHANNEL_ID, 100000);
                if (ret < 0) {
                        printf("IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
                        continue;
                }

                // »ñµÃIVS¹¦ÄÜ¼ÆËã³öµÄÖÇÄÜ·ÖÎö½á¹û
                ret = IMP_IVS_GetResult(IVS_CHANNEL_ID, (void **)&result);
                if (ret < 0) {
                        printf( "IMP_IVS_GetResult(%d) failed\n", 0);
                        continue;
                }
                if(access("/tmp/lzf_ok",F_OK) == 0 && result->count > 0)
                        LOG_LZF("ivs result count:%d alarm:%d\n",result ->count,result->alarm);

                if (result ->count == 0) {
                        int iDisable = set_timer_disable_HBD(4, 0);
                        if(iDisable==1)
                                g_iIsMotion=0;
                }else{
                        set_timer_disable_HBD(4, 1);

                }
                if( result ->count && result ->alarm ){
                        pthread_mutex_lock(&body_signal_t);
                        pthread_cond_signal(&body_signal_cond);
                        pthread_mutex_unlock(&body_signal_t);

                }
				
				if(lastNum!=result ->count){
					 for(grp_id=0;grp_id<VIDEO_STREAM_NUMS-1;grp_id++){
               			 for(i=0;i<rshu;i++)
					
					 resetRectshow(grp_id,i);
				     

                }
					}
				// if( !result ->count )
				// 	continue;
           
                if(GetYSXCtl(YSX_AI_MODE)){
						for(grp_id=0;grp_id<VIDEO_STREAM_NUMS-1;grp_id++){
                        for (i = 0; i < result ->count; i++) {
                                ulx=result->person[i].show_box.ul.x;
                                uly=result->person[i].show_box.ul.y;
                                brx=result->person[i].show_box.br.x;
                                bry=result->person[i].show_box.br.y;
                                //for(j=0;j<VIDEO_STREAM_NUMS-1;j++)
                                drawRect(grp_id,i,ulx,uly,brx,bry);
                        }
						}
                }

                lastNum=result->count;



                ret = IMP_IVS_ReleaseResult(IVS_CHANNEL_ID, (void *)result);
                if (ret < 0) {
                        printf("IMP_IVS_ReleaseResult(%d %d) failed\n", 0,1);
                        continue;
                }

                loop_count ++;

        }
        ivs_persondet_stop(1,&inteface);
        LOG("exit motion track thread...\n");
}


