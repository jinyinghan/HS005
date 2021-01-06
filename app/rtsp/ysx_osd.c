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
	short type;		//�ļ����� �� ��ֵ������0x4d 0x42 Ҳ�����ַ�'BM'
	int file_size;		//��ʾ��λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ��
	short reserved1;	//��������������Ϊ0
	short reserved2;	//��������������Ϊ0
	int offset;			//��ʾ���ļ�ͷ��ʼ��ʵ�ʵ�ͼ����֮ǰ���ֽڵ�ƫ������  λͼ��Ϣͷ�͵�ɫ��ĳ��Ȼ���ݲ�ͬ����仯�����Կ��������ƫ��ֵѸ�ٵĴ�ͷ�ļ��ж�ȡ��λ���ݡ�
	
}fileinfo_head;

typedef struct {
	int st_size;	//bitmap_head �ṹ����Ҫ���ֽ��� 
	uint32_t width;		// ͼ��Ŀ�ȣ���λ������
	uint32_t height;	//ͼ��ĸ߶ȣ���λ������
	short planes;	//ΪĿ���豸˵��λ��������ֵ���Ǳ���Ϊ1
	short bitcount;	// ��ʾ������/���� �� ֵΪ1,4,8,16,24,32
	int compression;	//��ʾͼ������ѹ�������ͣ� ��ʱ����û��ѹ��������BI_RGB    0--��ѹ��
	int sizeimage;		//��ʾͼ��Ĵ�С�����ֽ�Ϊ��λ������BI_RGB��ʽʱ��������Ϊ0
	uint32_t xpelspermeter;	//��ʾˮƽ�ֱ��ʣ�������/�ױ�ʾ
	uint32_t ypelspermeter;	//��ʾ��ֱ�ֱ��ʣ�������/�ױ�ʾ
	int crl_used;		//��ʾλͼʵ��ʹ�õĲ�ɫ���е���ɫ����������Ϊ0�Ļ�����˵��ʹ�����е�ɫ���
	int clr_important;	//��ʾ��ͼ����ʾ����ҪӰ�����ɫ��������Ŀ�������0����ʾ����Ҫ��
	
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
        /*1.����OSD����*/
    	rHanderRect[i] = IMP_OSD_CreateRgn(NULL);     //cover
        if (rHanderRect[i] < 0) {
            IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn%d failed\n",rHanderRect[i]);
            return -1;
        }  

        /*2.ע��OSD����OSD����*/    
    	ret = IMP_OSD_RegisterRgn(rHanderRect[i], grp_id, NULL);
    	if (ret < 0) {
    		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
    		return -1;
    	}
        
        /*3. ����OSD���������Ժ���������*/
    	IMPOSDRgnAttr rAttrRect;
    	memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));
    	rAttrRect.type = OSD_REG_RECT;
    	rAttrRect.rect.p0.x = rect[i].x0;
    	rAttrRect.rect.p0.y = rect[i].y0;
    	rAttrRect.rect.p1.x = rect[i].x1;     //p0 is start，and p1 well be epual p0+width(or heigth)-1
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


