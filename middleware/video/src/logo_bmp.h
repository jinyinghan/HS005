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




#endif /*__LOGO_BMP_H__*/



