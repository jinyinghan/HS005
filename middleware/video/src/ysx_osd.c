#include <sys/queue.h>
#include "ysx_video.h"
#include "imp/imp_osd.h"
#include "logo_bmp.h"
#include "ysx_osd.h"
#include <imp/imp_log.h>

#define FONT_NUM 19


struct ysx_osd time_cfg[FS_CHN_NUM] ;
static struct ysx_osd pic_cfg[FS_CHN_NUM] ;


/*OSD的使用一般有以下几个步骤:
 *   创建OSD组
 *   绑定OSD组到系统中
 *   创建OSD区域
 *   注册OSD区域到OSD组中
 *   设置OSD组区域属性和区域属性
 *   设置OSD功能开关
*/

int system_osd_init()   
{
    int i;
    for (i = 0 ; i < FS_CHN_NUM; i++) {
        if (IMP_OSD_CreateGroup(i) != 0) {
            QLOG(FATAL,"IMP_OSD_CreateGroup %d failed.",i);
            return -1;
        }
    }

    memset(&time_cfg, 0, sizeof(time_cfg));
    memset(&pic_cfg, 0, sizeof(pic_cfg));
    return 0;
}

int system_osd_uninit(void)
{
//    int ret = 0;
    int index = 0;
    for (index = 0 ; index < FS_CHN_NUM; index++) {
        if (IMP_OSD_DestroyGroup(index) != 0) {
            QLOG(FATAL,"IMP_OSD_CreateGroup %d failed.", index);
            return -1;
        }
    }
    return 0;
}



uint8_t *ysx_osd_image(char *filepath, uint32_t *w, uint32_t *h)
{
    fileinfo_head file_st;
    bitmap_head bitmap_st;
    uint8_t *src,*dst;
    uint32_t size1 = 0,size2 = 0;
    uint32_t linebytes,i,j;
    uint8_t tmp;
    FILE *fp;

    fp = fopen(filepath,"rb");
    if(NULL == fp){
        QLOG(FATAL,"fopen file %s failed.\n",filepath);
        return NULL;
    }

	fread(&file_st,1,sizeof(fileinfo_head),fp);	
    fread(&bitmap_st,1,sizeof(bitmap_head),fp);
    if(bitmap_st.bitcount != 24){
        QLOG(FATAL,"bmp file %s need to be 24 bit.\n",filepath);
        return NULL;
    }

    *w = bitmap_st.width;
    *h = bitmap_st.height;
    size1 = bitmap_st.width * bitmap_st.height * 3;   // bgr 
    size2 = size1 * 4 / 3;                            // bgra
    src = (uint8_t *)malloc(sizeof(char)*size1);
    if(NULL == src){
        QLOG(FATAL,"malloc failed.\n");
        return NULL;
    }
    
    dst = (uint8_t *)malloc(sizeof(char)*size2);
    if(NULL == src){
        QLOG(FATAL,"malloc failed.\n");
        return NULL;
    }
    
    fseek(fp,file_st.offset,SEEK_SET);
    fread(src,1,size1,fp);
    fclose(fp);

    // 上下翻转
    linebytes = bitmap_st.width*3;
    for(i=0 ;i < linebytes; i++)  
        {
            for(j=0 ;j < bitmap_st.height/2; j++)
            {                
                tmp = *(src + linebytes* j + i);
                *(src + linebytes* j + i) = *(src + i + linebytes*(bitmap_st.height-j-1));  //此处不是(height - j) 而是(height - j - 1)
                
                *(src + i + linebytes*(bitmap_st.height-j-1)) = tmp;
            }
        }

    // bgr --> bgra
	for (i = 0; i < size1 / 3; i++)
	{
		for(j=0;j < 3;j++)
			*(dst + 4 * i + j) = *(src + 3 * i + j);

        if(*(dst + 4 * i + 0) == 255 &&     \
                *(dst + 4 * i + 1) == 255 &&    \
                *(dst + 4 * i + 2) == 255 )  
		    *(dst + 4 * i + 3) = 0x00;
        else
            *(dst + 4 * i + 3) = 0xff;
	}

    free(src);
    return dst;
}

IMPRgnHandle pic_osd_set(struct ysx_osd cfg, QCamVideoInputOSD *pOsdInfo)
{

    int ret;    
    uint8_t *image;
    uint32_t picw,pich;
    IMPRgnHandle handle_rgn;
    IMPOSDRgnAttr osd;
    IMPOSDGrpRgnAttr handle_grp;

    /*图片转换，获取width 和 height*/
    if(NULL == (image = ysx_osd_image(pOsdInfo->pic_path, &picw, &pich)))
        return -1;
    
    memset(&osd,0,sizeof(IMPOSDRgnAttr));
    /*1.创建OSD区域*/
    handle_rgn = IMP_OSD_CreateRgn(NULL);           
    if (handle_rgn < 0) 
	{
        QLOG(FATAL, "IMP_OSD_CreateRgn failed");
        return -1;
    }      

    /*2.注册OSD区域到OSD组中*/
    if (IMP_OSD_RegisterRgn(handle_rgn,cfg.grp,NULL) != 0) 
	{
        QLOG(FATAL, "IMP_OSD_RegisterRgn failed");
        return -1;
    } 
    
    /*3. 设置OSD组区域属性和区域属性*/
    memset(&handle_grp,0,sizeof(IMPOSDGrpRgnAttr));

    IMPOSDRgnAttr rgnAttr;
    memset(&rgnAttr,0,sizeof(IMPOSDRgnAttr));
    rgnAttr.type = OSD_REG_PIC;
    rgnAttr.rect.p0.x = pOsdInfo->pic_x;
    rgnAttr.rect.p0.y = pOsdInfo->pic_y;
    rgnAttr.rect.p1.x = rgnAttr.rect.p0.x+picw-1;     //p0 is start锛宎nd p1 well be epual p0+width(or heigth)-1
    rgnAttr.rect.p1.y = rgnAttr.rect.p0.y+pich-1;
    rgnAttr.fmt = PIX_FMT_BGRA;
    rgnAttr.data.picData.pData = image;
    ret = IMP_OSD_SetRgnAttr(handle_rgn, &rgnAttr);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_SetRgnAttr Logo error !");
        return -1;
    }

	handle_grp.show = 1;
    handle_grp.gAlphaEn = 1;
    handle_grp.fgAlhpa = 0xff;
    handle_grp.layer = 1;

    ret = IMP_OSD_SetGrpRgnAttr(handle_rgn,cfg.grp,&handle_grp);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_SetGrpRgnAttr failed");
        return -1;
    }  

    ret = IMP_OSD_Start(cfg.grp);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_SetGrpRgnAttr failed");
        return -1;
    }  

	if(image)    
		free(image);
	
    return handle_rgn;
}

int pic_osd_unset(struct ysx_osd cfg)
{    
    if (IMP_OSD_ShowRgn(cfg.handle,cfg.grp,0) != 0) 
	{
        QLOG(FATAL, "IMP_OSD_ShowRgn failed");
        return -1;
    }      

    if (IMP_OSD_UnRegisterRgn(cfg.handle,cfg.grp) != 0) 
	{
        QLOG(FATAL, "IMP_OSD_UnRegisterRgn failed");
        return -1;
    }   
	
    return 0;
}


IMPRgnHandle time_osd_set(struct ysx_osd cfg,QCamVideoInputOSD *pOsdInfo)
{
    int ret;    
    IMPOSDRgnAttr osd;
    IMPOSDGrpRgnAttr handle_grp;  
    IMPRgnHandle rHanderFont;
    
    memset(&osd,0,sizeof(IMPOSDRgnAttr));
    /*1.创建OSD区域*/
    rHanderFont = IMP_OSD_CreateRgn(NULL);           
    if (rHanderFont < 0) 
	{
        QLOG(FATAL, "IMP_OSD_CreateRgn failed");
        return -1;
    }      

    /*2.注册OSD区域到OSD组中*/
    ret = IMP_OSD_RegisterRgn(rHanderFont,cfg.grp,NULL);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_RegisterRgn failed");
        return -1;
    } 
    
    /*3. 设置OSD组区域属性和区域属性*/
    memset(&handle_grp,0,sizeof(IMPOSDGrpRgnAttr));

    IMPOSDRgnAttr rAttrFont;
    memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
    rAttrFont.type = OSD_REG_PIC;
    rAttrFont.rect.p0.x = pOsdInfo->time_x;
    rAttrFont.rect.p0.y = pOsdInfo->time_y;
    rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + FONT_NUM*cfg.font_w- 1;   //p0 is start锛宎nd p1 well be epual p0+width(or heigth)-1
    rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + cfg.font_h- 1;
    rAttrFont.fmt = PIX_FMT_BGRA;
    rAttrFont.data.picData.pData = NULL;

    VID_LOG("p0(%d,%d), p1(%d,%d)\n",rAttrFont.rect.p0.x,rAttrFont.rect.p0.y,rAttrFont.rect.p1.x,rAttrFont.rect.p1.y);

    ret = IMP_OSD_SetRgnAttr(rHanderFont, &rAttrFont);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_SetRgnAttr Logo error !");
        return -1;
    }
         
    handle_grp.show = 0;
    handle_grp.gAlphaEn = 1;
    handle_grp.fgAlhpa = 0xff;
    handle_grp.layer = 2;

    ret = IMP_OSD_SetGrpRgnAttr(rHanderFont,cfg.grp,&handle_grp);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_SetGrpRgnAttr failed");
        return -1;
    }  

    ret = IMP_OSD_Start(cfg.grp);
    if (ret < 0) 
	{
        QLOG(FATAL, "IMP_OSD_SetGrpRgnAttr failed");
        return -1;
    }
	
    return rHanderFont;
}


int sample_osd_exit(IMPRgnHandle prHander, int grpNum)
{
	int ret;
	ret = IMP_OSD_ShowRgn(prHander, grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
	}
    
	ret = IMP_OSD_UnRegisterRgn(prHander, grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
	}


	IMP_OSD_DestroyRgn(prHander);

#if 0
    // 因为将osd绑定到了encode Group,所以这里不能先释放osd group
    ret = IMP_OSD_DestroyGroup(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(0) error\n");
		return -1;
	}
#endif 


    return 0;


    
#if 0
	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[1], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Logo error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[2], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close cover error\n");
	}

	ret = IMP_OSD_ShowRgn(prHander[3], grpNum, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close Rect error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[0], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[1], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn logo error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[2], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Cover error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(prHander[3], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Rect error\n");
	}


	IMP_OSD_DestroyRgn(prHander[0]);
	IMP_OSD_DestroyRgn(prHander[1]);
	IMP_OSD_DestroyRgn(prHander[2]);
	IMP_OSD_DestroyRgn(prHander[3]);

	ret = IMP_OSD_DestroyGroup(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(0) error\n");
		return -1;
	}
	free(prHander);
	prHander = NULL;

#endif 

	return 0;
}





static void *stream_osd(void *p)
{
    unsigned char font_offset;
	/*generate time*/
	char DateStr[40];
	time_t currTime;
	struct tm *currDate;
	unsigned i = 0, j = 0;
	uint8_t *dateData = NULL;
    uint8_t *pdata = NULL;
	IMPOSDRgnAttrData rAttrData;
    int font_h,font_w,linebyte;

	struct ysx_osd *time_osd = (struct ysx_osd *)p;
    struct timeval tv;  
    printf("osd proc channel: %02d start...\n", time_osd ->grp);
    
    font_h = time_osd->font_h;
    font_w = time_osd->font_w;
    if(font_w == MAIN_OSD_WIDTH)   
		font_offset = 0;		//500W
    else if(font_w == SECOND_OSD_WIDTH)
		font_offset = 13;		//720P
	else
        font_offset = 26;   	//360P
        
    linebyte = font_w * 4;
    dateData = (uint8_t *)malloc(FONT_NUM * font_h * font_w * 4);
    if (NULL == dateData) {
        QLOG(FATAL, "mallo buffer for osd date error\n");
        return (void *)(-1);
    }

	QLOG(TRACE,"grp num %d , font w %d , h %d",time_osd->grp,font_w,font_h);
    if(IMP_OSD_ShowRgn(time_osd->handle,time_osd->grp,1) != 0 )
	{
        QLOG(FATAL, "IMP_OSD_ShowRgn error\n");
        return (void *)(-1);
    }
	
    QLOG(TRACE,"font width = %d, height = %d\n",font_w,font_h);
	char pr_name[64];
	memset(pr_name,0,sizeof(pr_name));
	sprintf(pr_name,"stream_osd[%d]",time_osd->grp);
	prctl(PR_SET_NAME,pr_name);

    while(time_osd->enable)
	{
        tv.tv_sec = 1;
        tv.tv_usec = 0;
       
        if(select(0, NULL, NULL, NULL, &tv) != 0)
            continue;

		time(&currTime);
		currDate = localtime(&currTime);
		memset(DateStr, 0, 40);
		strftime(DateStr, 40, "%Y/%m/%d %H:%M:%S", currDate); 		//2016/09/06 18:09:24

		for (i = 0; i < FONT_NUM; i++) 
		{
			switch(DateStr[i]) {
				case '0' ... '9':
					pdata = gTimeBgra[DateStr[i] - '0'+font_offset].data;
					break;
				case ':':
					pdata = gTimeBgra[10+font_offset].data;
					break;                    
				case '/':
					pdata = gTimeBgra[11+font_offset].data;
					break;
				case ' ':
					pdata = gTimeBgra[12+font_offset].data;
					break;
				default:
					break;
			}
                      
			for (j = 0; j < font_h ; j++) {
				memcpy(dateData + j * FONT_NUM * linebyte + i*linebyte,
						pdata + j*linebyte, linebyte);
			}
		}
      
		rAttrData.picData.pData = (void *)dateData;

        if(IMP_OSD_UpdateRgnAttrData(time_osd->handle, &rAttrData) != 0)
        {
            QLOG(TRACE,"IMP_OSD_UpdateRgnAttrData failed!");
        }

	}

    // 设置组区域是否显示
//    IMP_OSD_ShowRgn(time_osd->handle, time_osd->grp,0);
    sample_osd_exit(time_osd -> handle, time_osd -> grp);
    
	if (dateData) 
    	free(dateData);

    QLOG(TRACE, "ysx_update_thread exit!\n");
	pthread_exit(0);
}

extern struct encoder_channel gEncoderChns[FS_CHN_NUM];

int QCamVideoInput_SetOSD(int channel, QCamVideoInputOSD *pOsdInfo)
{
	VID_LOG("channel osd set  %d\n",channel);
	if (channel > FS_CHN_NUM || NULL== pOsdInfo) {
		QLOG(FATAL,"Invalid channel %d or osdinfo %p",channel,pOsdInfo);
		return -1;
	}

	if (gEncoderChns[channel].alive == false) {
		VID_LOG("channel %d is not added\n",channel);
		return -1;
	}    
	
    uint8_t grp = gEncoderChns[channel].enc_grp;

	// about picture OSD 
	
    if (pOsdInfo->pic_enable)
    {
        pic_cfg[grp].grp = grp;
        if(pic_cfg[grp].enable == 1) 
            pic_osd_unset(pic_cfg[grp]);
        
        pic_cfg[grp].handle = pic_osd_set(pic_cfg[grp],pOsdInfo);
        pic_cfg[grp].enable = 1;  
    }
    else
    {   
        if(pic_cfg[grp].enable == 1) 
            pic_osd_unset(pic_cfg[grp]);
    }
	
	//about timestamp OSD
    if (pOsdInfo->time_enable)
    {
        time_cfg[grp].grp = grp;
        if (time_cfg[grp].grp == 0)
        {
            time_cfg[grp].font_w = MAIN_OSD_WIDTH;
            time_cfg[grp].font_h = MAIN_OSD_HEIGHT;
        }
		else if(time_cfg[grp].grp == 1)
		{
			printf("=========================================osd   vga===========================================\n");
			//time_cfg[grp].font_w = THIRD_OSD_WIDTH;
            //time_cfg[grp].font_h = THIRD_OSD_HEIGHT;	
			time_cfg[grp].font_w = SECOND_OSD_WIDTH;
            time_cfg[grp].font_h = SECOND_OSD_HEIGHT;
		}
		else
		{
			time_cfg[grp].font_w = THIRD_OSD_WIDTH;
            time_cfg[grp].font_h = THIRD_OSD_HEIGHT;	
		}

        if(time_cfg[grp].enable == 0)
        {
            time_cfg[grp].handle = time_osd_set(time_cfg[grp],pOsdInfo);
            time_cfg[grp].enable = 1;	
			
            QLOG(TRACE,"grp = %d,handle=%d",time_cfg[grp].grp,time_cfg[grp].handle);
			
            if (pthread_create(&time_cfg[grp].osd_tid, NULL, stream_osd, &time_cfg[grp]) != 0) 
			{
				QLOG(FATAL,"pthread create Error : %s",strerror(errno));
                return -1;
            }
        }
    }
    else
    {
            time_cfg[grp].grp = grp;
            time_cfg[grp].enable = 0;
    }
    
    return 0;
}


int jpg_osd_enable = 0;

int QCamVideoInput_SetJpgOSD(int channel, QCamVideoInputOSD *pOsdInfo){
	if(jpg_osd_enable == 1){
		printf("jpg osd setup\n");
		return -1;
	}
	//if(channel != JPG_CHN){
	//	printf("not jpg chn\n");
	//	return -1;
	//}
	/*
	struct ysx_osd jpg_time_cfg;
	jpg_time_cfg.grp = channel;
	jpg_time_cfg.font_w = MAIN_OSD_WIDTH;
    jpg_time_cfg.font_h = MAIN_OSD_HEIGHT;
	jpg_time_cfg.enable = 1;
	
	jpg_time_cfg.handle = time_osd_set(jpg_time_cfg,pOsdInfo);
	//jpg_time_cfg.grp = 4;
	//pthread_t jpg_osd_tid;
	printf("jpg osd %d\n",jpg_time_cfg.handle);
	if (pthread_create(&jpg_time_cfg.osd_tid, NULL, stream_osd, &jpg_time_cfg) != 0) 
	{
		QLOG(FATAL,"pthread create Error : %s",strerror(errno));
        return -1;
    }
    */
    #if 1
    	struct ysx_osd jpg_time_cfg;
		memset(&jpg_time_cfg,0,sizeof(jpg_time_cfg));
		int grp = channel;
		//memcpy(&jpg_time_cfg,&time_cfg[0],sizeof(time_cfg[0]));
        jpg_time_cfg.grp = grp;
        if (jpg_time_cfg.grp == 0)
        {
            jpg_time_cfg.font_w = MAIN_OSD_WIDTH;
            jpg_time_cfg.font_h = MAIN_OSD_HEIGHT;
        }
		else if(jpg_time_cfg.grp == 1)
		{
			
			jpg_time_cfg.font_w = THIRD_OSD_WIDTH;
            jpg_time_cfg.font_h = THIRD_OSD_HEIGHT;	
			
		}
		else
		{
			jpg_time_cfg.font_w = THIRD_OSD_WIDTH;
            jpg_time_cfg.font_h = THIRD_OSD_HEIGHT;	
		}
		printf("jpg_time_cfg.enable %d\n",jpg_time_cfg.enable);
        if(jpg_time_cfg.enable == 0)
        {
            jpg_time_cfg.handle = time_osd_set(jpg_time_cfg,pOsdInfo);
            jpg_time_cfg.enable = 1;	
			
            QLOG(TRACE,"grp = %d,handle=%d",jpg_time_cfg.grp,jpg_time_cfg.handle);
			
            if (pthread_create(&jpg_time_cfg.osd_tid, NULL, stream_osd, &jpg_time_cfg) != 0) 
			{
				QLOG(FATAL,"pthread create Error : %s",strerror(errno));
                return -1;
            }
        }
	 #else
	 	int grp= channel;
		time_cfg[grp].grp = grp;
        if (time_cfg[grp].grp == 0)
        {
            time_cfg[grp].font_w = MAIN_OSD_WIDTH;
            time_cfg[grp].font_h = MAIN_OSD_HEIGHT;
        }
		else if(time_cfg[grp].grp == 1)
		{
			printf("=========================================osd   vga===========================================\n");
			//time_cfg[grp].font_w = THIRD_OSD_WIDTH;
            //time_cfg[grp].font_h = THIRD_OSD_HEIGHT;	
			time_cfg[grp].font_w = SECOND_OSD_WIDTH;
            time_cfg[grp].font_h = SECOND_OSD_HEIGHT;
		}
		else
		{
			time_cfg[grp].font_w = THIRD_OSD_WIDTH;
            time_cfg[grp].font_h = THIRD_OSD_HEIGHT;	
		}

        if(time_cfg[grp].enable == 0)
        {
            time_cfg[grp].handle = time_osd_set(time_cfg[grp],pOsdInfo);
            time_cfg[grp].enable = 1;	
			
            QLOG(TRACE,"grp = %d,handle=%d",time_cfg[grp].grp,time_cfg[grp].handle);
			
            if (pthread_create(&time_cfg[grp].osd_tid, NULL, stream_osd, &time_cfg[grp]) != 0) 
			{
				QLOG(FATAL,"pthread create Error : %s",strerror(errno));
                return -1;
            }
        }



	 #endif
	jpg_osd_enable = 1;
	return 0;
}




