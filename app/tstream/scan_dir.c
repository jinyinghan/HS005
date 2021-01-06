#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <dirent.h>  
#include <sys/statfs.h>
#include <dirent.h>
#include <errno.h>
#include<sys/wait.h>
#include <stdlib.h>
#include <time.h>

typedef struct _recordindex
{
    unsigned int iy;
    char im;
    char id;
    char ih;
    char imi;
    unsigned int timelen;
}RIndex;

void del_tmp_name(char *filename)
{
	char *start = strstr(filename,"tmp-");

	if(start)
	{
		
		char tmp[128];
		memset(tmp,0,sizeof(tmp));
		int len = start-filename;
		memcpy(tmp,filename,len);
		
		start += strlen("tmp-");
		strcat(tmp,start);

		if(rename(filename,tmp) != 0)
			perror("rename");
	}
}


int scan_dir(char *dirpath)  
{  
    struct dirent **namelist,*ptr;  
	struct stat s_buf;
    int n;  
	char nameTemp[128];
  
    n = scandir(dirpath, &namelist, NULL, alphasort);  
    if (n < 0) {
		return -1;
	}   

	int i;
	for(i=0;i<n;i++)
	{
		ptr = namelist[i];
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)	 ///current dir OR parrent dir
				continue;
		
		memset(nameTemp,0,sizeof(nameTemp));
		strcpy(nameTemp,dirpath);
		strcat(nameTemp,"/");
		strcat(nameTemp,ptr->d_name);

		stat(nameTemp,&s_buf);
		if(S_ISDIR(s_buf.st_mode))
		{			
//			printf("### %s\n",ptr->d_name);
			scan_dir(nameTemp);
		}
		else if(S_ISREG(s_buf.st_mode)) 
		{
			printf("%s\n", nameTemp); 
			del_tmp_name(nameTemp);
		}
		
		free(ptr);  
	}  
    free(namelist);      
}  

void dump_index(RIndex temp)
{
	printf("iy = %d ",temp.iy);
	printf("im = %d ",temp.im);
	printf("id = %d ",temp.id);
	printf("ih = %d ",temp.ih);
	printf("imi = %d ",temp.imi);
	printf("timelen = %d ",temp.timelen);
	printf("\n");
	return;
}

int CreateRecordIndex(char *basePath, FILE *fIndex, unsigned int iStart)
{
	DIR *dir;
        struct dirent *ptr;

        char nameTemp[50];
       // nameTemp[0] = '\0';
	//const char *basePath = "/media/mmcblk0p1/easyn/";
        if ((dir=opendir(basePath)) == NULL)
        {
                printf("Open dir error... %s\n", basePath);
                return 0;
        }

        while ((ptr=readdir(dir)) != NULL)
        {
                if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
                        continue;
				if (ptr->d_type == 4)
				{
					//printf("dir name %s\n", ptr->d_name);
					strcpy(nameTemp,basePath);
                    strcat(nameTemp,"/");
                    strcat(nameTemp,ptr->d_name);
					CreateRecordIndex(nameTemp, fIndex,iStart);
				}
				else if (ptr->d_type == 8)
				{	
		
					//if(strstr(ptr->d_name, "Record"))				
						RIndex rtmp;
						struct tm tb;
					
						char fmt[] = "%Y_%m_%d_%H_%M_%S";
						//char buf[] = "2000_02_01_00_09_33.avi";
						if (strptime(ptr->d_name, fmt, &tb) != 0) 
						{
//							if (tb.tm_min != last_min)
							{
								rtmp.iy = 1900+tb.tm_year;
								rtmp.im = tb.tm_mon+1;
								rtmp.id = tb.tm_mday;
								rtmp.ih = tb.tm_hour;
								rtmp.imi = tb.tm_min;
								rtmp.timelen = 60- tb.tm_sec;
								dump_index(rtmp);
						//	printf("file name %s\n", ptr->d_name);
						//	printf("%d y %d timelen %d\n", iStart, rtotal[iStart].iy, rtotal[iStart].timelen);
//								fwrite(&rtmp, sizeof(RIndex), 1, fIndex);
//								iStart++;
//								last_min = tb.tm_min;
							}


							//rtotal[iStart] = rtmp;
					
						}
					//	char buf[] = "2000-01-01-00";
							
				}
   
        }

    closedir(dir);
	return 1;
}

int main(int argc, char **argv)
{
	scan_dir(argv[1]);
//	CreateRecordIndex(argv[1],NULL,0);

	return 0;
}
