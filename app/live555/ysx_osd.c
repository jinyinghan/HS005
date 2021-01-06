/*
 *   Author:xuc@yingshixun.com
 *   Date:2016.7
 *   Function: an example for h264 capture stream data
 */

#include <stdio.h>
#include <stdint.h>
#include "AudioInput.hh"
#include "imp/imp_osd.h"
#include "imp/imp_log.h"

#define TAG 						"sample-RTSPServer"
#define CLEAR(x) memset(x,0,sizeof(x))

#pragma pack(1)

typedef struct {
	short type;		//文件类型 ， 该值必须是0x4d 0x42 也就是字符'BM'
	int file_size;		//表示该位图文件的大小，用字节为单位。
	short reserved1;	//保留，必须设置为0
	short reserved2;	//保留，必须设置为0
	int offset;			//表示从文件头开始到实际的图像数之前的字节的偏移量。  位图信息头和调色板的长度会根据不同情况变化，所以可以用这个偏移值迅速的从头文件中读取到位数据。
	
}fileinfo_head;

typedef struct {
	int st_size;	//bitmap_head 结构所需要的字节数 
	uint32_t width;		// 图像的宽度，单位是像素
	uint32_t height;	//图像的高度，单位是像素
	short planes;	//为目标设备说明位面数，其值总是被设为1
	short bitcount;	// 表示比特数/像素 ， 值为1,4,8,16,24,32
	int compression;	//表示图像数据压缩的类型， 暂时讨论没有压缩的类型BI_RGB    0--不压缩
	int sizeimage;		//表示图像的大小，以字节为单位，当用BI_RGB格式时，可设置为0
	uint32_t xpelspermeter;	//表示水平分辨率，用像素/米表示
	uint32_t ypelspermeter;	//表示垂直分辨率，用像素/米表示
	int crl_used;		//表示位图实际使用的彩色标中的颜色索引数（设为0的话，则说明使用所有调色板项）
	int clr_important;	//表示对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。
	
}bitmap_head;

#pragma pack()

#define MAX_RECT 5
struct osd_rect{
    uint8_t enable;
    uint16_t x0;
    uint16_t y0;
    uint16_t x1;
    uint16_t y1;
    uint16_t linewidth;  // pixel
};

struct osd_rect rect[MAX_RECT];


int osd_rect_extract()
{
    char *pos;
    FILE *fp = NULL;   
    char fileBuf[1024],target[64];
    char tmp[64];
    int str_len;
    int i,j;
    
    CLEAR(rect);
    fp = fopen("rect.conf","r");
    if(fp == NULL){
        IMP_LOG_ERR(TAG,"open file rect.conf failed!\n");
        return -1;
    }
    memset(fileBuf,0,1024);
    fread(fileBuf,1,1024,fp);
    fclose(fp);

    for(i = 0;i < MAX_RECT;i++)
    {   
        //point left-top ,point right-bottom
        for(j=0;j<2;j++)
        {
            /*x*/
            CLEAR(target);
            sprintf(target,"rect%d_x%d=",i,j);
            pos = strstr(fileBuf,target);
            if(NULL == pos)
                continue;
            pos += strlen(target);
            str_len = strcspn(pos,",");
            if(str_len > 64)
                continue;
            CLEAR(tmp);
            memcpy(tmp,pos,str_len);
             if(j)
                rect[i].x1 = atoi(tmp);  
            else
                rect[i].x0 = atoi(tmp);  
            
            CLEAR(target);
            sprintf(target,"rect%d_y%d=",i,j);
            pos = strstr(fileBuf,target);
            if(NULL == pos)
                continue;
            pos += strlen(target);
            str_len = strcspn(pos,",");
            if(str_len > 64)
                continue;
            CLEAR(tmp);
            memcpy(tmp,pos,str_len);
            if(j)
                rect[i].y1 = atoi(tmp);  
            else
                rect[i].y0 = atoi(tmp);  
                
        }

        // line width
        CLEAR(target);
        sprintf(target,"rect%d_linewidth=",i);
        pos = strstr(fileBuf,target);
        if(NULL == pos)
            continue;

        pos += strlen(target);
        str_len = strcspn(pos,",");
        if(str_len > 64)
            continue;
        CLEAR(tmp);
        memcpy(tmp,pos,str_len);
        rect[i].linewidth = atoi(tmp);
    }

    for(i = 0;i < MAX_RECT;i++)
         printf("RECT[%d]: p0(%d,%d) , p1(%d,%d) width=%d\n",i,rect[i].x0,rect[i].y0,rect[i].x1,rect[i].y1,rect[i].linewidth);

    return 0;
}

/*Function : Create OSD region and register it to group*/
int set_osd_rect(uint8_t grp_id)
{
    int ret; 
    int i;
    IMPRgnHandle rHanderRect[MAX_RECT];

    osd_rect_extract();

    for(i=0; i < MAX_RECT; i++)
    {       
        /*1.创建OSD区域*/
    	rHanderRect[i] = IMP_OSD_CreateRgn(NULL);     //cover
        if (rHanderRect[i] < 0) {
            IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn%d failed\n",rHanderRect[i]);
            return -1;
        }  

        /*2.注册OSD区域到OSD组中*/    
    	ret = IMP_OSD_RegisterRgn(rHanderRect[i], grp_id, NULL);
    	if (ret < 0) {
    		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
    		return -1;
    	}
        
        /*3. 设置OSD组区域属性和区域属性*/
    	IMPOSDRgnAttr rAttrRect;
    	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
    	rAttrRect.type = OSD_REG_RECT;
    	rAttrRect.rect.p0.x = rect[i].x0;
    	rAttrRect.rect.p0.y = rect[i].y0;
    	rAttrRect.rect.p1.x = rect[i].x1;     //p0 is start锛宎nd p1 well be epual p0+width(or heigth)-1
    	rAttrRect.rect.p1.y = rect[i].y1;
    	rAttrRect.fmt = PIX_FMT_MONOWHITE;
    	rAttrRect.data.lineRectData.color = OSD_BLACK;
    	rAttrRect.data.lineRectData.linewidth = rect[i].linewidth;    
    	ret = IMP_OSD_SetRgnAttr(rHanderRect[i], &rAttrRect);
    	if (ret < 0) {
    		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Cover error !\n");
    		return -1;
    	}
    	IMPOSDGrpRgnAttr grAttrRect;
    	memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
    	grAttrRect.show = 1;


    	grAttrRect.gAlphaEn = 1;
    	grAttrRect.fgAlhpa = 0xff;
    	grAttrRect.layer = 1+i;
        grAttrRect.scalex = 1;
        grAttrRect.scaley = 1;
    	if (IMP_OSD_SetGrpRgnAttr(rHanderRect[i], grp_id, &grAttrRect) < 0) {
    		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
    		return -1;
    	}
        
    }
    
    ret = IMP_OSD_Start(grp_id);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr failed\n");
        return -1;
    }  
    

    return 0;
}


