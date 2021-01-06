#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/statfs.h>

//#include "include/common_env.h"
//#include "include/common_func.h"
typedef struct _recordindex
{
    unsigned int iy;
    char im;
    char id;
    char ih;
    char imi;
    unsigned int timelen;
}RIndex;

void dump_index(RIndex temp)
{
	printf("iy = %d ",temp.iy);
	printf("im = %d ",temp.im);
	printf("id = %d ",temp.id);
	printf("ih = %d ",temp.ih);
	printf("imi = %d ",temp.imi);
	printf("timelen = %d ",temp.timelen);

	struct tm tb;

	memset(&tb,0,sizeof(struct tm));		
	tb.tm_year = temp.iy-1900;// = 1900+tb.tm_year;
	tb.tm_mon  = temp.im - 1;
	tb.tm_mday = temp.id - 1;
	tb.tm_hour = temp.ih;
	tb.tm_min  = temp.imi;
	tb.tm_sec  = 0; //60-rtotal[i].timelen;
	time_t stime = mktime(&tb);
	
	printf("time = %d\n",stime);
}

int main(int argc , char **argv)
{
	if(argc < 2){
		printf("arguments is too few !\n");
		return -1;
	}

	char *index_name = argv[1];
	FILE *fp = fopen(index_name,"rb");
	if(NULL == fp){
		printf("File Open error !\n");
		return -1;
	}

	RIndex tmp;
	int ret;
	
	while(1){
		ret = fread(&tmp,sizeof(RIndex),1,fp);
		if(ret <= 0 )
			break;
		dump_index(tmp);
	}

	fclose(fp);


	return 0;
}
