#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "Options.hh"



#define HZK_FONT_PATH	"/lib/font/hzk16"
#define ASC_FONT_PATH	"/lib//font/asc16"

static FILE *hzkfd = NULL;
static FILE *ascfd = NULL;
static char *hzkptr = NULL;
static char *ascptr = NULL;

//=============================================
//auth:zxh
//date:2013-4-11
//function:video osd init
//param:
//=============================================
int VideoOsdReadInit(void){
	int		retval = 0;
	int 	lSize = 0;

	hzkfd = NULL;
	ascfd = NULL;
	hzkptr = NULL;
	ascptr = NULL;

	if((hzkfd = fopen(HZK_FONT_PATH, "rb")) == NULL)
	{
		printf("Front osd Open font lib hzk16 failed !\n");
		return -1;
	}

	//*获取文件大小.
	fseek(hzkfd, 0, SEEK_END);
	lSize = ftell(hzkfd);
	printf("File size = %d\n", lSize);
	rewind(hzkfd);

	//*分配内存以包含整个文件
	hzkptr = (char*)malloc(lSize);
	if(hzkptr == NULL)
	{
		fclose(hzkfd);
		perror("Couldn't allocate memory\n");
		return -1;
	}
	retval = fread(hzkptr,1,lSize,hzkfd);
	fclose(hzkfd);
	//printf("Read size = %d\n", retval);

	if((ascfd = fopen(ASC_FONT_PATH, "rb")) == NULL)
	{
		printf("Front osd Open font lib asc16 failed !\n");
		return -1;
	}

	fseek(ascfd, 0, SEEK_END);
	lSize = ftell(ascfd);
	//printf("File size = %d\n", lSize);
	rewind(ascfd);

	//*分配内存以包含整个文件
	ascptr = (char*)malloc(lSize);
	if(ascptr == NULL)
	{
		if(hzkptr != NULL)
		{
			free(hzkptr);
			hzkptr = NULL;
		}
		fclose(ascfd);
		perror("Couldn't allocate memory\n");
		return -2;
	}
	retval = fread(ascptr,1,lSize,ascfd);
	fclose(ascfd);
}

static int rgbfd = -1;
void osd_to_RGB(unsigned int zcolor, unsigned int bcolor, int len, unsigned char * s, unsigned char *d, int encode_flag,unsigned char *text_flag)
{
	int m,n,i,j,k;
	int textlen;
	textlen = len;
	unsigned char *dorg = d;
#if 0
	unsigned char R = zcolor&0x1f;
	unsigned char G = zcolor>>5&0x1f;
	unsigned char B = zcolor>>10&0x1f;
#else
	unsigned char R = zcolor&0xff;
	unsigned char G = zcolor>>8 &0xff;
	unsigned char B = zcolor>>16&0xff;
	unsigned char A = 0xff;
#endif

	unsigned char *flag=text_flag;//记录每行中英文处理标志的位置

	if((encode_flag == RES_D1)||(encode_flag == RES_VGA)||(encode_flag == RES_720P))
	{
		for(m = 0;m<16;m++)//行
		{
			for(n= 0;n<2;n++)
			{
				for(i=0; i<textlen; i++)
				{
					if(*flag++ != 0)//处理中文字符
					{
						for(j=0;j<2;j++)
						{
							for(k=0;k<8;k++) //象素
							{
								if(((s[m*2+j+i*16]>>(7-k))&0x1)!=0) //字色
								{
									*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
									d += 4;
								}
								else
								{
									*d++ = bcolor;//背景色(0x03:绿)
									*d++ = bcolor;
									*d++ = bcolor;
									*d++ = bcolor;
								}
							}
						}
						/************************
						  一个中文字符占两个字节,因此中英文区分标志位和
						  字符长度控制变量都要做相应的增加
						 *************************/
						flag++;
						i++;
					}
					else//处理英文字符
					{
						for(j=0;j<8;j++)
						{
							if(((s[m+i*16]>>(7-j))&0x1)!=0)
							{
								*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
								//*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
								d += 4;
							}
							else
							{
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;
							}
						}
					}
				}
				//处理下一行开始前，恢复中英文标志flag
				flag=text_flag;
			}
		}
	}

	else if(encode_flag == RES_HD1)
	{
		for(m = 0;m<16;m++)
		{
			for(i=0; i<textlen; i++)
			{
				if(*flag++ != 0)
				{
					for(j=0;j<2;j++)
					{
						for(k=0;k<8;k++)
						{
							if(((s[m*2+j+i*16]>>(7-k))&0x1)!=0)
							{
								*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
								//*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
								d += 4;

								*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
								d += 4;
							}
							else
							{
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;

								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;

							}
						}
					}
					//一个中文字符占两个字节
					flag++;
					i++;
				}
				else
				{
					//处理英文字符
					for(j=0;j<8;j++)
					{
						if(((s[m+i*16]>>(7-j))&0x1)!=0)
						{
							*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
							//*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
							d += 4;

							*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
							//*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
							d += 4;
						}
						else
						{
							*d++ = bcolor;
							*d++ = bcolor;
							*d++ = bcolor;
							*d++ = bcolor;

							*d++ = bcolor;
							*d++ = bcolor;
							*d++ = bcolor;
							*d++ = bcolor;

						}
					}
				}
			}
			//处理下一行开始前，恢复中英文标志flag
			flag=text_flag;
		}
	}

	else if((encode_flag == RES_CIF)||(encode_flag == RES_QVGA))
	{
		for(m = 0;m<16;m++)
		{
			for(i=0; i<textlen; i++)
			{
				if(*flag++ != 0)
				{
					for(j=0;j<2;j++)
					{
						for(k=0;k<8;k++)
						{
							if(((s[m*2+i*16+j]>>(7-k))&0x1)!=0)
							{
								*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
								//*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
								d += 4;
							}
							else
							{
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;
								*d++ = bcolor;
							}
						}
					}
					//一个中文字符占两个字节
					flag++;
					i++;
				}
				else//处理英文字符
				{
					for(j=0;j<8;j++)
					{
						if(((s[m+i*16]>>(7-j))&0x1)!=0)
						{
							*(unsigned int *)d = (A << 24) | (R << 16) | (G << 8) | B;
							//*(unsigned int *)d = A | (R << 24) | (G << 16) | (B << 8);
							d += 4;
						}
						else
						{
							*d++ = bcolor;
							*d++ = bcolor;
							*d++ = bcolor;
							*d++ = bcolor;
						}
					}
				}
			}
			//处理下一行开始前，恢复中英文标志flag
			flag=text_flag;
		}
	}

#if 1
	if (rgbfd < 0) {
		int rgbret = 0;
		rgbfd = open("/tmp/bgra", O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (rgbfd > 0) {
			if (write(rgbfd, dorg, d - dorg) != d - dorg) {
				printf("write /tmp/bgra d - dorg = %d failed\n", d - dorg);
			}
			close(rgbfd);
		}
	}
#endif
}

//=============================================
//auth:zxh
//date:2013-4-11
//function:video note
//param:
//=============================================
int osd_find_asc_from_lib(char * tx, unsigned char *buffer)
{
	int chrlen = strlen(tx);
	char * t = ( char *)tx;
	int i;

	if (buffer == NULL){
		printf("===osd_find_asc_from_lib0 is failed===\n");
	}

	if (tx == NULL){
		printf("===osd_find_asc_from_lib1 is failed===\n");
	}
	//	ASSERT(buffer != NULL);
	//	ASSERT(tx != NULL);
	if(ascptr == NULL)
	{
		return -1;
	}
	for(i=0; i<chrlen; i++)
	{
		//fseek(ascfd, (*t)*16,0);
		//fread(&buffer[i*16],16,1,ascfd);
		memcpy(&buffer[i*16],&ascptr[(*t)*16],16);
		t++;
	}

	return 0;
}

//=============================================
//auth:zxh
//date:2013-4-11
//function:video note
//param:
//=============================================
int osd_find_hzk_from_lib(char * tx, unsigned char *buffer)
{
	int chrlen = strlen(tx);
	char * t = ( char *)tx;
	unsigned char qh,wh;
	unsigned long location;
	int i;

	if (buffer == NULL){
		printf("===osd_find_hzk_from_lib0 is failed===\n");
	}

	if (tx == NULL){
		printf("===osd_find_hzk_from_lib1 is failed===\n");
	}
	//	ASSERT(buffer != NULL);
	//	ASSERT(tx != NULL);
	if(hzkptr == NULL)
	{
		return -1;
	}
	chrlen /= 2;
	for(i = 0;i<chrlen;i++)
	{
		qh=*t-0xa0;
		wh=*(t+1)-0xa0;
		location=(94*(qh-1)+(wh-1))*32;
		// 	fseek(hzkfd,location,SEEK_SET);
		// 	fread(&buffer[i*32],32,1,hzkfd);
		memcpy(&buffer[i*32],&hzkptr[location],32);
		t+=2;
	}

	return 0;
}

//=============================================
//auth:zxh
//date:2013-4-11
//function:video note
//param:
//=============================================
int Osd_show_fixed_text(int hand, char * text, int encode_flag,unsigned int color,unsigned char *pdata)
{
#if 1
	int 			chrlen;
	int  			iRet = -1;
	unsigned char  	lib[512];
	unsigned char	temp;

	if (text == NULL){
		printf("===Osd_show_fixed_text failed===\n");
	}
	//	ASSERT(text != NULL);
	chrlen = strlen(text);
	memset(lib,0,512);//*读取字库文件
	/**********
	  此处开始分别处理中英文差异问题，要让中英文混合显示处理，从此处开始。
	 ************/

	char tmptext[3];//*该变量用于存储当前要处理的字符串，英文的长度为1，中文的长度为2。
	unsigned char text_language_flag[30]; //*记录lib中的中英文标志
	unsigned int text_current=0; //*记录当前中英文标志位置
	unsigned int lib_current=0;	//*记录当前lib位置

	//	printf("text:%s len=%d\n",text,strlen(text));
	memset(text_language_flag,0,30);
	while(*text)
	{
		//		printf("lib_current %d text %x\n",lib_current,*text);
		temp = *text;
		if(temp < 0xa0)
		{
			printf("");
			tmptext[0]=*text++;
			tmptext[1]='\0';
			osd_find_asc_from_lib(tmptext, &lib[lib_current]);//*获取英文字库
			lib_current += 16;
			text_language_flag[text_current++]=0;
		}
		else
		{
			tmptext[0]=*text++;
			tmptext[1]=*text++;
			tmptext[2]='\0';
			if(tmptext[1] == 0)
				break;
			osd_find_hzk_from_lib(tmptext, &lib[lib_current]);//*获取中文字库
			lib_current +=32;
			text_language_flag[text_current++]=1;
			text_language_flag[text_current++]=1;
		}

		//printf("tmptext is :%s\n",tmptext);//*打印提取中英文字符串信息
	}
#if 0
	int k = 0, h = 0;

	for (h = 0; h < chrlen; h++) {
		for (k = 0; k < 16; k++) {
			if (lib[h * 16 + k] > 0) {
				putchar('1');
			} else {
				putchar('0');
			}
		}
		putchar('\n');
	}

#endif

	//	printf("lib_current end\n");
#if 1
	//*将中英文点阵转换为RGB数据
	if(color == 0)
	{
		osd_to_RGB(0xffffffff,0x0,chrlen,lib,pdata,encode_flag,text_language_flag);
	}
	else
	{
		osd_to_RGB(color,0x0,chrlen,lib,pdata,encode_flag,text_language_flag);
	}
#endif
	//	printf("===osd_to_RGB===%d\n",color);
#endif
	return 0;
}
