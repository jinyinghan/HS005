#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "logo_bmp.h"

void help()
{
    printf("bmp2font sourcefile  destination flag\n");
    printf("if flag == 1 , add ',' to file\n");
    return;
}

int main(int argc, char **argv)
{
    fileinfo_head file_st;
    bitmap_head bitmap_st;
    uint8_t *src,*dst;
    uint32_t size1 = 0,size2 = 0;
    uint32_t linebytes,i,j;
    uint8_t tmp;
    FILE *fp,*fp2;
    int flag ;


    if(argc < 4)
    {
        printf("too few arguments \n");
        help();
        return -1;
    }
    flag = atoi(argv[3]);

    if(access(argv[1],F_OK) != 0)
    {
        printf("file %s not found\n",argv[1]);
        return -1;
    }
    
    fp2 = fopen(argv[2],"wb");
    if(NULL == fp2){
        printf("open file %s failed!\n",argv[2]);
        return -1;
    }

    fp = fopen(argv[1],"rb");
    if(NULL == fp){
        printf("fopen file %s failed.\n",argv[1]);
        return -1;
    }

	fread(&file_st,1,sizeof(fileinfo_head),fp);	
    fread(&bitmap_st,1,sizeof(bitmap_head),fp);
    if(bitmap_st.bitcount != 24){
        printf("bmp file %s need to be 24 bit.\n",argv[1]);
        return -1;
    }
	
	printf("width %d  height %d  xpelspermeter %d  ypelspermeter  %d\n",bitmap_st.width,bitmap_st.height,bitmap_st.xpelspermeter , bitmap_st.ypelspermeter);
	printf("file_st.offset %d\n", file_st.offset);
	
    size1 = bitmap_st.width * bitmap_st.height * 3;   // bgr 
    size2 = bitmap_st.width * bitmap_st.height * 4;                            // bgra
	
    linebytes = bitmap_st.width*3;
	//if(linebytes%2 == 1)
	//{
	//	linebytes = linebytes+1;
	//	size1 = linebytes*bitmap_st.height;
	//}
	
    src = (uint8_t *)malloc(sizeof(char)*size1);
    if(NULL == src){ 
        printf("malloc failed.\n");
        return -1;
    }
    
    dst = (uint8_t *)malloc(sizeof(char)*size2);
    if(NULL == dst){
        printf("malloc failed.\n");
        return -1;
    }
    

	
    fseek(fp,file_st.offset,SEEK_SET);
	
    //fread(src,1,size1,fp);
	for(i=0; i<bitmap_st.height;i++)
	{
		char tmp;
		fread(src+linebytes*i, bitmap_st.width,3,fp);
		fread(&tmp, 1,1,fp);
	}
	
    fclose(fp);

    // 上下翻转

	
	
	if(1)//for  test
	{	
		printf("\n");
		printf("\n");
		for(i=0;i< bitmap_st.height;i++)
		{
			for(j=0;j< bitmap_st.width; j++)
			{
				if( *(src+linebytes*i+j*3) != 0xff  || *(src+1+linebytes*i+j*3)!=0xff || *(src+2+linebytes*i+j*3)!=0xff ) 
					printf("*");
				else
					printf(" ");
			}
			printf("\n");
		}
		
	}
#if 1
    for(i=0 ;i < linebytes; i++)  
        {
            for(j=0 ;j < bitmap_st.height/2; j++)
            {                
                tmp = *(src + linebytes* j + i);
                *(src + linebytes* j + i) = *(src + i + linebytes*(bitmap_st.height-j-1));  //此处不是(height - j) 而是(height - j - 1)
                
                *(src + i + linebytes*(bitmap_st.height-j-1)) = tmp;
            }
        }
#endif
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
	
	if(0)//for  test
	{	
		printf("\n");
		printf("\n");
		for(i=0;i< bitmap_st.height;i++)
		{
			for(j=0;j< bitmap_st.width; j++)
			{
				if( *(src+linebytes*i+j*3) != 0xff  || *(src+1+linebytes*i+j*3)!=0xff || *(src+2+linebytes*i+j*3)!=0xff ) 
					printf("*");
				else
					printf(" ");
				
			}
			printf("\n");
		}
		
	}	

    if(flag)
    {
        for(i=0;i<size2;i++)
         {
            fprintf(fp2,"0x%x,",dst[i]);
        
        }
    }
    else
        fwrite(dst,size2,1,fp2);

    printf("size2=%d\n",size2);
    free(src);
    free(dst);
    
    fclose(fp2);

    return 0;
}

