/*
 *   Author:xuc@yingshixun.com
 *   Date:2016.7
 *   Function: an example for h264 capture stream data
 */

#ifndef __LOGO_BMP_H__
#define __LOGO_BMP_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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




#endif /*__LOGO_BMP_H__*/



