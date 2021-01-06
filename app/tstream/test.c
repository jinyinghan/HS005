#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>   
#include <sys/stat.h>   
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#define CLEAR(x) memset(x,0,sizeof(x))

FILE *stdout_fp;

void std2file()
{       
   if(access("/tmp/sd_ok",F_OK) == 0 )
    {
        printf("### Change LOG destination\n");
        char buf[128];
        FILE *fp = fopen("/tmp/sd_ok", "r");
        CLEAR(buf);
        int ret = fread(buf, 1, sizeof(buf), fp);
        buf[ret-1]='\0';
        fclose(fp);
      
        strcat(buf,"/camera_log.txt");
        stdout_fp = freopen(buf, "a+", stdout);  
		setvbuf(stdout_fp,NULL,_IOLBF ,1024);           
    }
}
void file2std()
{
 
	stdout_fp = freopen("/dev/console", "w", stdout);
  
}

int main()
{

	stdout_fp = stdout;
    int cnt = 0;
    
    printf("### Before stdout to file \n");
    std2file();
	int status = 0;
	
    while(1){

        usleep(100*1000);
        printf("### LOG count = %d\n",cnt++);
		if(access("/tmp/sd_ok",F_OK) == 0)
		{		
			if(status != 0)
			{
				std2file();
				status = 0;
			}	
		}	
		if(access("/tmp/sd_ok",F_OK) != 0)
		{
			if(status != 1)
			{
				file2std();
				status = 1;
			}	
		}			
    }
 

}
