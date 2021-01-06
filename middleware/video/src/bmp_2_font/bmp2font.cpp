#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#define OPT_ENUM_INPUTS			256
#define OPT_OUT_FILE			OPT_ENUM_INPUTS + 1
#define OPT_BG_COLOR			OPT_ENUM_INPUTS + 2
#define OPT_EDG_COLOR			OPT_ENUM_INPUTS + 3
#define OPT_FONT_COLOR			OPT_ENUM_INPUTS + 4




typedef struct tag_BITMAPINFOHEADER{
        unsigned short      biSize;
        unsigned int       biWidth;
        int       biHeight;
        unsigned short       biPlanes;
        unsigned short       biBitCount;
        unsigned int      biCompression;
        unsigned int      biSizeImage;
        unsigned int       biXPelsPerMeter;
        unsigned int       biYPelsPerMeter;
        unsigned int      biClrUsed;
        unsigned int      biClrImportant;
} BITMAPINFOHEADER;

typedef struct tag_BITMAPFILEHEADER {
        unsigned int   bfSize;
        unsigned short    bfReserved1;
        unsigned short    bfReserved2;
        unsigned int   bfOffBits;
}BITMAPFILEHEADER; 

typedef struct tag_RGBQUAD {
        unsigned char    rgbBlue;
        unsigned char    rgbGreen;
        unsigned char    rgbRed;
        unsigned char    rgbReserved;
} RGBQUAD;

typedef struct tag_BITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD                 bmiColors[1];
} BITMAPINFO;

typedef struct bmp_data_info_t
{
	char bmp_file[64];
	char out_file[64];
	FILE* out_fp;
	int bg_color;
	int edg_color;
	int font_color;
	unsigned char value;
}bmp_data_info;



static struct option opts[] = {
    {"out-file", 	1, 0, OPT_OUT_FILE},
    {"bg-color", 	1, 0, OPT_BG_COLOR},
    {"edg-color", 	1, 0, OPT_EDG_COLOR},
    {"font-color", 	1, 0, OPT_FONT_COLOR},
	{0, 0, 0, 0}
};
static void usage(const char *argv0)
{
	printf("Usage: %s [options] \n", argv0);
	printf("Supported options:\n");
	printf("     --out-file		The out put file \n");
	printf("     --bg-color		The background color \n");
	printf("     --edg-color	The edge color \n");
	printf("     --font-color	The font color \n");
	printf("\n");

}
int GetBmpInfo(const char *filename,     BITMAPFILEHEADER  *pBmpFileHeader,BITMAPINFO *pBmpInfo)
{
    FILE *pFile;

    unsigned short    bfType;

    if(NULL == filename)
    {
        printf("LoadBMP: filename=NULL\n");
        return -1;
    }

    if( (pFile = fopen((char *)filename, "rb")) == NULL)
    {
        printf("Open file faild:%s!\n", filename);
        return -1;
    }

    (void)fread(&bfType, 1, sizeof(bfType), pFile);
    if(bfType != 0x4d42)
    {
        printf("not bitmap file\n");
        fclose(pFile);
        return -1;
    }
    
    (void)fread(pBmpFileHeader, 1, sizeof(BITMAPFILEHEADER), pFile);
    (void)fread(pBmpInfo, 1, sizeof(BITMAPINFO), pFile);
    fclose(pFile);
    
    return 0;  
}
void write_head_file(FILE * fp)
{
	fprintf(fp,"\nstatic unsigned char bitMap = \n{\n");
}
void write_tail_file(FILE * fp)
{
	fprintf(fp,"\n};\n");
}
void write_head_node(FILE * fp,unsigned char value)
{
	fprintf(fp,"	/*%c start*/\n	",value);	
}
void write_tail_node(FILE * fp,unsigned char value)
{
	fprintf(fp,"\n	/*%c end*/\n",value);
}

int bmp_to_data(bmp_data_info*info)
{
	int ret;
	int c;
	FILE* bmp_fp = NULL;
	BITMAPFILEHEADER  BmpFileHeader = {};
	BITMAPINFO BmpInfo = {};
	unsigned char * buf = NULL;
	int buf_len = 0;
	int pixel;
	int i,j,k,l;
	unsigned char * p;
	unsigned char data[64]={};
	int bit;
	int bmpWidth;
	int bmpHeight;
	if(access(info->bmp_file, F_OK) != 0)
	{
		printf("bmp file:%s not exist\n",info->bmp_file);
		goto EXIT;
	}
	ret = GetBmpInfo(info->bmp_file,&BmpFileHeader,&BmpInfo);
	pixel = BmpInfo.bmiHeader.biBitCount/8;
	buf_len = BmpInfo.bmiHeader.biWidth *BmpInfo.bmiHeader.biHeight*pixel;
	buf = (unsigned char *)malloc(buf_len);
	if(NULL == buf)
	{
		printf("malloc error\n");
		goto EXIT;
	}
	
	if( (bmp_fp = fopen((char *)info->bmp_file, "rb")) == NULL)
    {
        printf("Open file faild:%s!\n", info->bmp_file);
        goto EXIT;
    }
	fseek(bmp_fp, BmpFileHeader.bfOffBits, 0);
	ret = fread(buf,sizeof(unsigned char),buf_len,bmp_fp);
	if(ret != buf_len)
	{
		printf("fread error ret:%d bur_len:%d\n",ret,buf_len);
		goto EXIT;
	}
	p = buf;
	memset(data,0,sizeof(data));
	bit = 0;
	write_head_node(info->out_fp,info->value);
	printf("\n");
	bmpWidth = BmpInfo.bmiHeader.biWidth;
	bmpHeight = BmpInfo.bmiHeader.biHeight;
	for(i= bmpHeight-1;i>=0;i--)
	{
		for(j=0;j<bmpWidth;j++)
		{
			p = buf + (i*bmpWidth+j)*pixel;
			if((p[0]== ((info->font_color&0xff0000)>>16)) && (p[1] == ((info->font_color&0x00ff00)>>8)) && (p[2] == ((info->font_color&0x0000ff))))
			{
				
				data[bit/8] |= 0x1<<(bit%8);
				printf(" & ");
			}
			else if((p[0]== ((info->edg_color&0xff0000)>>16)) && (p[1] == ((info->edg_color&0x00ff00)>>8)) && (p[2] == ((info->edg_color&0x0000ff))))
			{
				data[bit/8+(bmpWidth/8)] |= 0x1<<(bit%8);
				printf(" * ");
			}
			else
			{
				printf("   ");
			}
			if(bit == (bmpWidth-1))
			{
				for(k = 0;k<(bmpWidth/8*2);k++)
				{
					fprintf(info->out_fp,"0x%02x,",data[k]);
					data[k] = 0;
				}
			}
			bit ++;
			bit %= bmpWidth;
		}
		printf("\n");
	}
	write_tail_node(info->out_fp,info->value);
EXIT:
	if(buf != NULL)
	{
		free(buf);
		buf = NULL;
	}
	if(bmp_fp != NULL)
	{
		fclose(bmp_fp);
		bmp_fp = NULL;
	}
	return ret;
}
int main(int argc,char** argv)
{
	int ret;
	int c;
	char out_file[64] = "default.h";
	unsigned char value = 0;
	int bg_color = 0xffffff;
	int edg_color = 0x606060;
	int font_color = 0xf0f0f0;
	int i;
	bmp_data_info info = {};
	
	opterr = 0;
	while ((c = getopt_long(argc, argv, "h", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return 0;
			case OPT_OUT_FILE:
				memset(out_file, 0, 64);
				sscanf(optarg, "%s", out_file);
				break;
			case OPT_BG_COLOR:
				bg_color = strtol(optarg, NULL, 16);
				break;
			default:
				printf("Invalid option -%c\n", c);
				printf("Run %s -h for help.\n", argv[0]);
				return -1;
		}
	}
	
	sprintf(info.out_file,"%s",out_file);
	if( (info.out_fp = fopen((char *)info.out_file, "ab+")) == NULL)
    {
        printf("Open file faild:%s!\n", info.out_file);
       	return -1;
    }
	write_head_file(info.out_fp);
	info.bg_color=bg_color;
	info.edg_color = edg_color;
	info.font_color = font_color;
	for(i=0;i<10;i++)
	{
		sprintf(info.bmp_file,"%d.bmp",i);
		info.value = i+0x30;
		bmp_to_data(&info);
	}
	sprintf(info.bmp_file,"-.bmp");
	info.value = '-';
	bmp_to_data(&info);
	sprintf(info.bmp_file,"colon.bmp");
	info.value = ':';
	bmp_to_data(&info);
	sprintf(info.bmp_file,"division.bmp");
	info.value = '/';
	bmp_to_data(&info);
	write_tail_file(info.out_fp);
	if(info.out_fp != NULL)
	{
		fclose(info.out_fp);
		info.out_fp = NULL;
	}
}
