#include "log.h"


#include <ivs/ivs_common.h>

//#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_move.h>
#include "include/common_env.h"
#include "include/common_func.h"
#include <imp/imp_osd.h>



#define MAX_RECT 5
IMPRgnHandle rHanderRect[2];

struct osd_rect{
    uint8_t enable;
    uint16_t x0;
    uint16_t y0;
    uint16_t x1;
    uint16_t y1;
    uint16_t linewidth;  // pixel
};

struct osd_rect rect[2];

static int get_osd_switch(void){
//	LOG("%s : %d type: %d\n",__FUNCTION__,__LINE__, g_enviro_struct.mosd_en);
	return g_enviro_struct.mosd_en==0?0:1;
}

int set_osd_rect(uint8_t grp_id)
{
    int ret;
    int i;
    //osd_rect_extract();

    /*1.´´½¨OSDÇøÓò*/
	rHanderRect[grp_id] = IMP_OSD_CreateRgn(NULL);     //cover
    if (rHanderRect[grp_id] < 0) {
        printf("IMP_OSD_CreateRgn%d failed\n",rHanderRect[grp_id]);
        return -1;
    }

    /*2.×¢²áOSDÇøÓòµ½OSD×éÖÐ*/
	ret = IMP_OSD_RegisterRgn(rHanderRect[grp_id], grp_id, NULL);
	if (ret < 0) {
		printf("IVS IMP_OSD_RegisterRgn failed\n");
		return -1;
	}

    /*3. ÉèÖÃOSD×éÇøÓòÊôÐÔºÍÇøÓòÊôÐÔ*/
	IMPOSDRgnAttr rAttrRect;
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = rect[grp_id].x0;
	rAttrRect.rect.p0.y = rect[grp_id].y0;
	rAttrRect.rect.p1.x = rect[grp_id].x1;     //p0 is startï¼Œand p1 well be epual p0+width(or heigth)-1
	rAttrRect.rect.p1.y = rect[grp_id].y1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_BLACK;
	rAttrRect.data.lineRectData.linewidth = rect[grp_id].linewidth;
	ret = IMP_OSD_SetRgnAttr(rHanderRect[grp_id], &rAttrRect);
	if (ret < 0) {
		printf("IMP_OSD_SetRgnAttr Cover error !\n");
		return -1;
	}
	IMPOSDGrpRgnAttr grAttrRect;
	memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrRect.show = 1;


	grAttrRect.gAlphaEn = 1;
	grAttrRect.fgAlhpa = 0xff;
	grAttrRect.layer = 1+grp_id;
    grAttrRect.scalex = 1;
    grAttrRect.scaley = 1;
	if (IMP_OSD_SetGrpRgnAttr(rHanderRect[grp_id], grp_id, &grAttrRect) < 0) {
		LOG("IMP_OSD_SetGrpRgnAttr Cover error !\n");
		return -1;
	}

    ret = IMP_OSD_Start(grp_id);
    if (ret < 0) {
        LOG("IMP_OSD_SetGrpRgnAttr failed\n");
        return -1;
    }

    return 0;
}

int drawRect(int grp_id,int x0 , int y0, int x1 , int y1)
{
	int ret;
//	int x0, y0,x1,y1;
#if 1
	if(grp_id == 0)
	{
		x0 = 1920 * x0 / 640;
		y0 = 1080 * y0 / 360;
		x1 = 1920*x1 / 640;
		y1 = 1080*y1 / 360;
	}

#endif
	IMPRgnHandle handler = rHanderRect[grp_id];

	// rAttr.data.lineRectData.color = color;
	  /*3. è®¾ç½®OSDç»„åŒºåŸŸå±žæ€§å’ŒåŒºåŸŸå±žæ€§*/
	IMPOSDRgnAttr rAttrRect;
	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
	rAttrRect.type = OSD_REG_RECT;
	rAttrRect.rect.p0.x = x0;
	rAttrRect.rect.p0.y = y0;
	rAttrRect.rect.p1.x = x1;     //p0 is starté”›å®Žnd p1 well be epual p0+width(or heigth)-1
	rAttrRect.rect.p1.y = y1;
	rAttrRect.fmt = PIX_FMT_MONOWHITE;
	rAttrRect.data.lineRectData.color = OSD_RED;
	if(grp_id==0)
	{
		rAttrRect.data.lineRectData.linewidth = 3;//rect[i].linewidth;
	}
	else
	{
		rAttrRect.data.lineRectData.linewidth = 1;//rect[i].linewidth;
	}
	ret = IMP_OSD_SetRgnAttr(handler, &rAttrRect);
	if (ret < 0) {
		LOG("IMP_OSD_SetRgnAttr Cover error !\n");
		return -1;
	}

	if (IMP_OSD_ShowRgn(handler, grp_id, 1) < 0) {
		LOG("%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;

}

int resetRectshow(int grp_id)
{
	int ret,i;
	IMPOSDRgnAttr rAttrRect;

	IMPRgnHandle handler = rHanderRect[grp_id];
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


int ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface,int sense)
{
	int ret = 0;
	move_param_input_t param;
	printf("ivs_move_start  start\n");
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
	printf("ivs_move_start  start end\n");
	return 0;
}

void *motion_drawRect_thread(void *arg)
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
	printf("1+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	set_osd_rect(0);
	set_osd_rect(1);

	ivs_move_start(0, 1, &inteface,*sense);
	LOG("#### motion drawRect start ...\n");

	prctl(PR_SET_NAME,"motion_drwaRect_thread");

	while(1)
	{

		if(g_media_struct.resolution == YSX_RESOLUTION_HIGH)
		{
			grp_id = VIDEO_STREAM_MAIN;
		}
		else if(g_media_struct.resolution == YSX_RESOLUTION_MID)
		{
			grp_id = VIDEO_STREAM_SECOND;
		}
		else
			grp_id = VIDEO_STREAM_THREE;

		ret = IMP_IVS_PollingResult(chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			printf("IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			continue;
		}

		ret = IMP_IVS_GetResult(chn_num, (void **)&result);
		if (ret < 0) {
			printf( "IMP_IVS_GetResult(%d) failed\n", 0);
			continue;
		}

		if(result->count==0)
		{
			resetRectshow(grp_id);
			continue;
		}

		for(i=0;i<result->count;i++)
		{
			tmp_area = (result->rects[i].br.x - result->rects[i].ul.x) * (result->rects[i].br.y - result->rects[i].ul.y);
			if(tmp_area>max_area)
			{
				max_size_number = i;
				max_area = tmp_area;
			}
		}
		if (get_osd_switch() == 1){
			drawRect(grp_id,result->rects[max_size_number].ul.x,result->rects[max_size_number].ul.y,
				result->rects[max_size_number].br.x,result->rects[max_size_number].br.y);
		}
		else{
			resetRectshow(grp_id);
		}
		ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
		if (ret < 0) {
			printf("IMP_IVS_ReleaseResult(%d) failed\n", 0);
			continue;
		}

	}

}

