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
#include <imp/imp_osd.h>

#include "sample-common.h"
#include <pthread.h>
#include <sys/prctl.h>

#define TAG "SAMPLE-FIGURE"

#include <qcam_smart.h>
extern smart_init_t smt;
pthread_mutex_t frame_mutex;

struct osd_rect{
	uint8_t enable;
		uint16_t x0;
		uint16_t y0;
		uint16_t x1;
		uint16_t y1;
		uint16_t linewidth;  // pixel
		
};

#define rshu 5
IMPRgnHandle rHanderRect[2][rshu];
struct osd_rect rect[2][rshu];



int set_osd_rect(uint8_t grp_id)
{
        int ret;
        int i;
        //osd_rect_extract();

        /*1.����OSD����*/
        for(i=0;i<rshu;i++){
                rHanderRect[grp_id][i] = IMP_OSD_CreateRgn(NULL);     //cover
                if (rHanderRect[grp_id][i] < 0) {
                        printf("IMP_OSD_CreateRgn%d failed\n",rHanderRect[grp_id][i]);
                        return -1;
                }

                /*2.ע��OSD����OSD����*/
                ret = IMP_OSD_RegisterRgn(rHanderRect[grp_id][i], grp_id, NULL);
                if (ret < 0) {
                        printf("IVS IMP_OSD_RegisterRgn failed\n");
                        return -1;
                }

                /*3. ����OSD���������Ժ���������*/
                IMPOSDRgnAttr rAttrRect;
                memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
                rAttrRect.type = OSD_REG_RECT;
                rAttrRect.rect.p0.x = 0;//rect[grp_id][i].x0;
                rAttrRect.rect.p0.y = 0;//rect[grp_id][i].y0;
                rAttrRect.rect.p1.x = 0;//rect[grp_id][i].x1;     //p0 is start，and p1 well be epual p0+width(or heigth)-1
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
						
                        IMP_LOG_ERR(TAG,"IMP_OSD_SetGrpRgnAttr Cover error !\n");
                        return -1;
                }
        }
        ret = IMP_OSD_Start(grp_id);
        if (ret < 0) {
                IMP_LOG_ERR(TAG,"IMP_OSD_SetGrpRgnAttr failed\n");
                return -1;
        }

        return 0;
}
int resetRectshow(int grp_id,int personNum)
{
        int ret;//,i;
        IMPOSDRgnAttr rAttrRect;

        IMPRgnHandle handler = rHanderRect[grp_id][personNum];
        // rAttr.data.lineRectData.color = color;
        /*3. 设置OSD组区域属性和区域属性*/
        memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
        rAttrRect.type = OSD_REG_RECT;
        rAttrRect.rect.p0.x = 0;
        rAttrRect.rect.p0.y = 0;
        rAttrRect.rect.p1.x = 0;     //p0 is start锛宎nd p1 well be epual p0+width(or heigth)-1
        rAttrRect.rect.p1.y = 0;
        rAttrRect.fmt = PIX_FMT_MONOWHITE;
        rAttrRect.data.lineRectData.color = OSD_BLACK;
		if(grp_id == 0)
        rAttrRect.data.lineRectData.linewidth = 10;//rect[grp_id][personNum].linewidth;//3;//rect[i].linewidth;
        else
		rAttrRect.data.lineRectData.linewidth = 3;
        ret = IMP_OSD_SetRgnAttr(rHanderRect[grp_id][personNum], &rAttrRect);
        if (ret < 0) {
               IMP_LOG_ERR(TAG,"IMP_OSD_SetRgnAttr Cover error !\n");
                return -1;
        }

        if (IMP_OSD_ShowRgn(handler, grp_id, 0) < 0) {
                IMP_LOG_ERR(TAG,"%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
                return -1;
        }

        return 0;
}
int drawRect_(int grp_id, int personNum, int x0, int y0, int x1, int y1)
{
        int ret;
		static int cnt0 = 0;
        // printf("x0 %d y0 %d x1 %d y1 %d\n",x0,y0,x1,y1);
        if (0 == grp_id) {
                // x0 = 1920 * x0 / 640;
              // y0 = 1080 * y0 / 360;	
              //  x1 = 1920 * x1 / 640;
              //  y1 = 1080 * y1 / 360;
              
				x0 = 2304 * x0 / 640;
				y0 = 1296 * y0 / 360;
				x1 = 2304 * x1 / 640;
				y1 = 1296 * y1 / 360;
				
				/*		
				x0 = 1920 * x0 / 640;
				y0 = 1080 * y0 / 360;
				x1 = 1920 * x1 / 640;
				y1 = 1080 * y1 / 360;
				*/
				//if(abs(x0 -x1)%100 > 50){
				//printf("channel 0 %d %d %d %d\n",x0,y0,x1,y1);
				//return 0;
				//}
        }
        else if (1 == grp_id) {
				x0 = 2304 * x0 / 1280;
				y0 = 1296 * y0 / 720;
				x1 = 2304 * x1 / 1280;
				y1 = 1296 * y1 / 720;			
                /*  x0 = 1280 * x0 / 640;
                    y0 = 720 * y0 / 360;	
                    x1 = 1280 * x1 / 640;
                    y1 = 720 * y1 / 360;
                 */
               //  if(cnt0 % 20 ==0)
              //   printf("channel 1 %d %d %d %d\n",x0,y0,x1,y1);
        }
		else
		{
		}


        // ������
        IMPRgnHandle handler = rHanderRect[grp_id][personNum];
        // ����OSD���������Ժ���������
        // OSD������������
        IMPOSDRgnAttr rAttrRect;
        memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
        rAttrRect.type = OSD_REG_RECT;
        rAttrRect.rect.p0.x = x0;
        rAttrRect.rect.p0.y = y0;
        rAttrRect.rect.p1.x = x1;     //p0 is start锛宎nd p1 well be epual p0+width(or heigth)-1
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
                IMP_LOG_ERR(TAG,"IMP_OSD_SetRgnAttr Cover error !\n");
                return -1;
        }

        // �����������Ƿ���ʾ
        if (IMP_OSD_ShowRgn(handler, grp_id, 1) < 0) {
                IMP_LOG_ERR(TAG,"%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
                return -1;
        }

        return 0;
}
/*
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
*/
//extern IMPIVSInterface *intefacex;

int sample_ivs_figure_start(int width,int height,int grp_num, int chn_num, IMPIVSInterface **inteface)
{
	int ret = 0;
	figure_param_input_t param;

	memset(&param, 0, sizeof(figure_param_input_t));
	param.frameInfo.width = width;//640;
	param.frameInfo.height = height;//360;
	*inteface = FigureInterfaceInit(&param);
	if (*inteface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}
	if((*inteface)->init && ((ret = (*inteface)->init(*inteface)) < 0)) {
		IMP_LOG_ERR(TAG, "inteface->init failed, ret=%d\n", ret);
		return -1;
	}
	//intefacex = *inteface;
	/* 
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
	*/
	return 0;
}

int sample_ivs_figure_stop(int chn_num, IMPIVSInterface *inteface)
{
	//int ret = 0;
	/*
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
	*/
	if(inteface->exit != NULL){

		inteface->exit(inteface);
	}
	FigureInterfaceExit(inteface);

	return 0;
}
///extern int figure_exit ;

//#define FIGURE_TEST   1
#if 0
void * figure_thread(void *arg)
{
	figure_param_output_t *result = NULL;
	int i = 0,m = 0,ret = 0,grp_id;
	int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
	int lastNum=0;
	printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	prctl(PR_SET_NAME, "figure_thread");
	while(!smt.figure_exit){

		while(!smt.ivs_enable){
			sleep(1);
			printf("ivs sleep\n");
			continue;
		}

	
		ret = IMP_IVS_PollingResult(1, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 1, IMP_IVS_DEFAULT_TIMEOUTMS);
			//return -1;
			 continue;
		}
		
		ret = IMP_IVS_GetResult(1, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 1);
			//return -1;
			 continue;
		}
	
		if(access("/tmp/lzf_ok",F_OK)==0 && result->count>0)
        	printf("we have catch person %d\n",result ->count);


		pthread_mutex_lock(&smt.smart_lock);
		memset(&smt.ft,0,sizeof(smt.ft));
		smt.ft.pd_num = result->count;
		for(i=0;i<result->count;i++){
			smt.ft.pd_rect[i].x1 = result->rects[i].ul.x;
			smt.ft.pd_rect[i].y1 = result->rects[i].ul.y;
			smt.ft.pd_rect[i].x2 = result->rects[i].br.x;
			smt.ft.pd_rect[i].y2 = result->rects[i].br.y;


		}
		//smt.ft.score = *(result->score);
		pthread_mutex_unlock(&smt.smart_lock);
		
		#ifdef FIGURE_TEST
		if(lastNum != result ->count){
			for(grp_id=0;grp_id<2;grp_id++){
            	for(i=0;i<rshu;i++)
					resetRectshow(grp_id,i);
				     

                }
		}
		
		for(grp_id=0;grp_id<2;grp_id++){
        	for (i = 0; i < result ->count; i++) {
            	x0=result->rects[i].ul.x;
                y0=result->rects[i].ul.y;
                x1=result->rects[i].br.x;
                y1=result->rects[i].br.y;
                printf("%d %d %d %d\n",x0,y0,x1,y1);              
                drawRect_(grp_id,i,x0,y0,x1,y1);
			   
            }
		}
		#endif
		
		lastNum = result->count;
		ret = IMP_IVS_ReleaseResult(1, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 1);
			//return -1;
			 continue;
		}


	

	}
	for(grp_id=0;grp_id<2;grp_id++){
		for(i=0;i<rshu;i++){
			sample_osd_exit(rHanderRect[grp_id][i],grp_id);
		}
	}
	pthread_exit(0);

}
#else
int skip_x = 10;
int set_skip_yuvframe(int skip){
	if(skip < 5 || skip > 20)
	 	skip = 10;
	skip_x = skip;
	return 0;
}

void * figure_thread(void *arg)
{

	IMPIVSInterface *inteface = (IMPIVSInterface *)arg;
	//IMPIVSInterface *inteface = intefacex;
	figure_param_output_t *result = NULL;
	
	//int i = 0,m = 0,ret = 0,grp_id;
	int i = 0,ret = 0;
	//int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
//	int lastNum=0;
	//printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	prctl(PR_SET_NAME, "figure_thread");
	IMPFrameInfo sframe;
	//IMPFrameInfo* sframe
	memset(&sframe,0,sizeof(sframe));
	//unsigned char * g_sub_nv12_buf = malloc(640*360*3/2);
	char g_sub_nv12_buf[640*360*3/2];
	memset(g_sub_nv12_buf,0,sizeof(g_sub_nv12_buf));
	while(!smt.figure_exit){

		while(!smt.ivs_enable){
			sleep(1);
			//printf("ivs sleep\n");
			continue;
		}
		pthread_mutex_lock(&frame_mutex);
		//IMP_FrameSource_SetFrameDepth(1, 1);
		ret = IMP_FrameSource_SnapFrame(1, PIX_FMT_NV12, 640, 360, g_sub_nv12_buf, &sframe);
		sframe.virAddr = (unsigned int)g_sub_nv12_buf;
		//ret = IMP_FrameSource_GetFrame(1, &sframe);
		if(ret){
			IMP_LOG_ERR(TAG, "IMP_FrameSource_SnapFrame failed,ret=%d\n", ret);
			//printf("======================fault 1\n");
			pthread_mutex_unlock(&frame_mutex);
			goto loop;
		}
		
		if (inteface->preProcessSync && ((ret = inteface->preProcessSync(inteface, &sframe)) < 0)) {
			IMP_LOG_ERR(TAG, "inteface->preProcessSync failed,ret=%d\n", ret);
			//printf("======================fault 2\n");
			pthread_mutex_unlock(&frame_mutex);
			goto loop;
		}
		if (inteface->processAsync && ((ret = inteface->processAsync(inteface, &sframe)) < 0)) {
			IMP_LOG_ERR(TAG, "move_interface->processAsync failed,ret=%d\n", ret);
			//printf("======================fault 3\n");
			pthread_mutex_unlock(&frame_mutex);
			goto loop;
		}
		if (inteface->getResult && ((ret = inteface->getResult(inteface, (void **)&result)) < 0)) {
			IMP_LOG_ERR(TAG, "inteface->getResult failed,ret=%d\n", ret);
			//printf("======================fault 4\n");
			pthread_mutex_unlock(&frame_mutex);
			goto loop;
		}
		
		if (inteface->releaseResult && ((ret = inteface->releaseResult(inteface, (void *)result)) < 0)){
			IMP_LOG_ERR(TAG, "inteface->releaseResult failed ret=%d\n", ret);
			//printf("======================fault 5\n");
			pthread_mutex_unlock(&frame_mutex);
			goto loop;
		}
		//IMP_FrameSource_SetFrameDepth(1, 0);
		pthread_mutex_unlock(&frame_mutex);
		
		// = (figure_param_output_t*)result;
		//printf("figure_param_output_t count %d \n",result->count);
		
    	if(access("/tmp/lzf_ok",F_OK) == 0)
        	printf("we have catch person %d\n",result ->count);
		pthread_mutex_lock(&smt.smart_lock);
		memset(&smt.ft,0,sizeof(smt.ft));
		smt.ft.pd_num = result->count;
		for(i=0;i<result->count;i++){
			smt.ft.pd_rect[i].x1 = result->rects[i].ul.x;
			smt.ft.pd_rect[i].y1 = result->rects[i].ul.y;
			smt.ft.pd_rect[i].x2 = result->rects[i].br.x;
			smt.ft.pd_rect[i].y2 = result->rects[i].br.y;


		}
		pthread_mutex_unlock(&smt.smart_lock);
		

		loop:
			usleep(1000*10*skip_x);
}
	pthread_exit(0);
}
int yuv_exit = 0;
void* yuv_proc(void*arg){
	char buf[640*360*3/2] = {0};
	while(!yuv_exit){
		
		
		memset(buf,0,sizeof(buf));
		QCamVideoInput_CatchYUV(640, 360, buf, 640*360*3/2);



		usleep(200*1000);
	}

	pthread_exit(0);


}

int creat_yuv(){
	pthread_t yuv_id;
	int ret = pthread_create(&yuv_id, NULL,yuv_proc, NULL);
    if (ret < 0) {
    	printf("create thread failed!\n");
        return -1;
    }
	pthread_detach(yuv_id);
	return 0;
}
#endif


