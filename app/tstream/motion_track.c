/*
OSD 使用方法:
    1. 创建OSD组
    2. 绑定OSD组到系统中
    3. 创建OSD区域
    4. 注册OSD区域到OSD组中
    5. 设置OSD组区域属性和区域属性
    6. 设置OSD功能开关
*/
#include <sys/prctl.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_move.h>
#include <imp/imp_osd.h>
//#include "include/common_env.h"
//#include "include/common_func.h"
//#include "include/common.h"
#include "include/common_env.h"
#include "motion_track.h"
#include "log.h"

#define MAX_RECT 5
IMPRgnHandle rHanderRect[VIDEO_STREAM_NUM];
struct osd_rect rect[VIDEO_STREAM_NUM];

extern G_StructEnviroment g_enviro_struct;
extern 	G_StructMedia g_media_struct;

static int get_osd_switch(void){
	//	LOG("%s : %d type: %d\n",__FUNCTION__,__LINE__, g_enviro_struct.mosd_en);
	return g_enviro_struct.mosd_en==0?0:1;
}

static int set_osd_rect(uint8_t grp_id)
{
	int ret;
	int i;

	rHanderRect[grp_id] = IMP_OSD_CreateRgn(NULL);  
	if (rHanderRect[grp_id] < 0) {
		printf("IMP_OSD_CreateRgn%d failed\n",rHanderRect[grp_id]);
		return -1;
	}

    // 注册OSD区域,handle 区域句柄,grpNum OSD组号,pgrAttr OSD组显示属性
	ret = IMP_OSD_RegisterRgn(rHanderRect[grp_id], grp_id, NULL);
	if (ret < 0) {
		printf("IVS IMP_OSD_RegisterRgn failed\n");
		return -1;
	}

	IMPOSDRgnAttr rAttrRect;
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
	rAttrRect.type = OSD_REG_RECT;  // OSD区域类型
	rAttrRect.rect.p0.x = rect[grp_id].x0;  // 矩形数据
	rAttrRect.rect.p0.y = rect[grp_id].y0;
	rAttrRect.rect.p1.x = rect[grp_id].x1;    
	rAttrRect.rect.p1.y = rect[grp_id].y1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;  // 点格式
	rAttrRect.data.lineRectData.color = OSD_BLACK;  // OSD区域属性数据,线、矩形数据
	rAttrRect.data.lineRectData.linewidth = rect[grp_id].linewidth;

	// 设置区域属性
	ret = IMP_OSD_SetRgnAttr(rHanderRect[grp_id], &rAttrRect);
	if (ret < 0) {
		printf("IMP_OSD_SetRgnAttr Cover error !\n");
		return -1;
	}

	// OSD组区域属性
	IMPOSDGrpRgnAttr grAttrRect;
	memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrRect.show = 1;            //是否显示
	grAttrRect.gAlphaEn = 1;        //Alpha开关
	grAttrRect.fgAlhpa = 0xff;      //前景Alpha
	grAttrRect.layer = 1 + grp_id;  //显示层
	grAttrRect.scalex = 1;          //缩放x参数
	grAttrRect.scaley = 1;          //缩放y参数

    // 设置OSD组区域属性
	if (IMP_OSD_SetGrpRgnAttr(rHanderRect[grp_id], grp_id, &grAttrRect) < 0) {
		LOG("IMP_OSD_SetGrpRgnAttr Cover error !\n");
		return -1;
	}

    // 设置开始OSD组的显示
	ret = IMP_OSD_Start(grp_id);
	if (ret < 0) {
		LOG("IMP_OSD_SetGrpRgnAttr failed\n");
		return -1;
	}

	return 0;
}

static int drawRect(int grp_id, int x0, int y0, int x1, int y1)
{
	int ret;

	if (0 == grp_id) {
		x0 = 1920 * x0 / 640;
		y0 = 1080 * y0 / 360;	
		x1 = 1920 * x1 / 640;
		y1 = 1080 * y1 / 360;
	}
	else if (1 == grp_id) {
    	/*x0 = 1280 * x0 / 640;
		y0 = 720 * y0 / 360;	
		x1 = 1280 * x1 / 640;
		y1 = 720 * y1 / 360;*/
		x0 = 640 * x0 / 640;
		y0 = 360 * y0 / 360;	
		x1 = 640 * x1 / 640;
		y1 = 360 * y1 / 360;
	}


    // 区域句柄
	IMPRgnHandle handler = rHanderRect[grp_id];
	// 设置OSD组区域属性和区域属性
	// OSD区域属性数据
	IMPOSDRgnAttr rAttrRect;
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = x0;
	rAttrRect.rect.p0.y = y0;
	rAttrRect.rect.p1.x = x1;     //p0 is start閿涘畮nd p1 well be epual p0+width(or heigth)-1
	rAttrRect.rect.p1.y = y1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_RED;
	if (grp_id == 0) {
		rAttrRect.data.lineRectData.linewidth = 3;//rect[i].linewidth;
	}
	else {
		rAttrRect.data.lineRectData.linewidth = 1;//rect[i].linewidth;
	}
	
	ret = IMP_OSD_SetRgnAttr(handler, &rAttrRect);
	if (ret < 0) {
		LOG("IMP_OSD_SetRgnAttr Cover error !\n");
		return -1;
	}

    // 设置组区域是否显示
	if (IMP_OSD_ShowRgn(handler, grp_id, 1) < 0) {
		LOG("%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

static int resetRectshow(int grp_id)
{
	int ret,i;
	IMPOSDRgnAttr rAttrRect;
	IMPRgnHandle handler = rHanderRect[grp_id];
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = 0;
	rAttrRect.rect.p0.y = 0;
	rAttrRect.rect.p1.x = 0;     //p0 is start閿涘畮nd p1 well be epual p0+width(or heigth)-1
	rAttrRect.rect.p1.y = 0;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_BLACK;
	rAttrRect.data.lineRectData.linewidth = 3;//rect[i].linewidth;
	ret = IMP_OSD_SetRgnAttr(rHanderRect[grp_id], &rAttrRect);
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


static int ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface, int sense)
{
	int ret = 0;
	move_param_input_t param;

	memset(&param, 0, sizeof(move_param_input_t));
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	param.sense = sense;                      /*sensitivity 0-4 , 0-off , 4 -- The most sensitive*/
	LOG("move track sense: %d\n",param.sense);
	*interface = MoveInterfaceInit(&param);
	if (*interface == NULL) {
		LOG( "IMP_IVS_CreateGroup(%d) failed\n",grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		LOG("IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		LOG("IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		LOG("IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

void * motion_drawRect_thread(void *arg)
{
	IMPIVSInterface *inteface = NULL;
	move_param_output_t *result = NULL;

	int ret;
	int chn_num = 1;
	int i;
	int grp_id = 0;
	long max_area = 0;
	long tmp_area = 0;
	int max_size_number;
	int *sense = (int*)arg;
    int index = 0;
    int pre_res = 0;
    unsigned long loop_count = 0;
      
    for (index; index < VIDEO_STREAM_NUM; index ++) {
	    set_osd_rect(index);
	}
    	
	ivs_move_start(IVS_GROUP_ID, IVS_CHANNEL_ID, &inteface, *sense);
	//ivs_move_start(0, 0, &inteface, *sense);
	LOG("#### motion tracker start ...\n");
    g_enviro_struct.motion_track.started = 1;
    
	prctl(PR_SET_NAME, "motion_tracker_thread");

	while(1)
	{   
        if(g_media_struct.resolution == YSX_RESOLUTION_HIGH){
            grp_id = VIDEO_STREAM_MAIN;
        }
        else if (g_media_struct.resolution == YSX_RESOLUTION_MID) {
            grp_id = VIDEO_STREAM_SECOND;
        }
        else if (g_media_struct.resolution == YSX_RESOLUTION_LOW) {
            grp_id = VIDEO_STREAM_SECOND;
        }                                   
        
        if (g_enviro_struct.motion_track.destroyed_thread) {
            break;
        }  

        //
		ret = IMP_IVS_PollingResult(IVS_CHANNEL_ID, 100000);
		if (ret < 0) {
			printf("IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			continue;
		}

		ret = IMP_IVS_GetResult(IVS_CHANNEL_ID, (void **)&result);
		if (ret < 0) {
			printf( "IMP_IVS_GetResult(%d) failed\n", 0);
			continue;
		}

		if (result -> count == 0) {
			resetRectshow(grp_id);
			continue;
		}

		for (i = 0; i < result -> count; i++) {
			tmp_area = (result -> rects[i].br.x - result->rects[i].ul.x) * (result->rects[i].br.y - result->rects[i].ul.y);
			if (tmp_area > max_area) {
				max_size_number = i;
				max_area = tmp_area;
			}
		}
		//printf("drawRect++++++++++++++++++++++++++++++++ %d\n",get_osd_switch());
		if (get_osd_switch() == 1){
			drawRect(grp_id,
			    result -> rects[max_size_number].ul.x,
			    result -> rects[max_size_number].ul.y,
				result -> rects[max_size_number].br.x,
				result -> rects[max_size_number].br.y);
		}
		else{
			resetRectshow(grp_id);
		}
		
		ret = IMP_IVS_ReleaseResult(IVS_CHANNEL_ID, (void *)result);
		if (ret < 0) {
			printf("IMP_IVS_ReleaseResult(%d) failed\n", 0);
			continue;
		}
		
		loop_count ++;
   
	}

    LOG("exit motion track thread...\n");
}

