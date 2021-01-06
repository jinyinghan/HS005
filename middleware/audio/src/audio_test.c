#include "ysx_audio.h"
#include <stdint.h>

int play_print_usage()
{

    printf("***1. bitwidth = 16 , samplerate = 8K  *****\n");
    printf("***2. bitwidth = 16 , samplerate = 16K  ****\n");
    printf("***3. bitwidth = 16 , samplerate = 44.1K****\n");    
    printf("play pcm file format,please input:\n");
    return 0;
}
FILE *fp1 = NULL;
static int cnt=0;
void ysx_get_record(const struct timeval *tv, const void *pcm_buf, const int pcm_len)
{
 //   printf("cb :tv = %d ,  len = %d\n",tv->tv_sec,pcm_len);
    
    fwrite(pcm_buf,1,pcm_len,fp1);
    fflush(fp1);

}

int main(int argc,char **argv)
{
    int ret;
    char ch;
    char tmp[64];
    FILE *fp;






    fp1 = fopen("audio.pcm","w");
    QCamAudioOutputAttribute pb_attr;
    QCamAudioOutputBufferStatus status;
	QCamAudioInputAttr micAttr;
	memset(&micAttr, 0, sizeof(micAttr));
	micAttr.cb = ysx_get_record;
	micAttr.volume = 60;
	micAttr.sampleRate = 16000;
	micAttr.sampleBit = 16;
	ret = QCamAudioInputOpen(&micAttr);//475ms
	if(ret)
	{
		printf("QCamAudioInputOpen error:%d\n", ret);
        return -1;
	}
	ret = QCamAudioInputStart();
	if(ret)
	{
		printf("QCamAudioStart mic error:%d\n",ret);
	}


    printf("########## init audio output ##########\n");
    QCamAudioOutputAttribute outAttr;
    memset(&outAttr, 0, sizeof(outAttr));
    outAttr.volume = 66;
    outAttr.sampleRate = 16000;
    outAttr.sampleBit = 16;
    ret = QCamAudioOutputOpen(&outAttr);
    if(ret)
    {
        printf("QCamAudioOutputOpen error:%d\n",ret);
        return -1;
    }   

#if 0
    fp = fopen("1-sample-16000KHz-16bit.pcm","rb");
       if(NULL == fp){
           printf("open file  failed !\n");
           return -1;
       }
       fseek(fp,0,SEEK_END);
       int len = ftell(fp);
       fseek(fp,0,SEEK_SET);


#if 0        
       char  *data = (char   *)malloc(len);
      
       QCamAudioOutputQueryBuffer(&status);
       printf("total:%d, busy:%d\n",status.total,status.busy);
       printf("start play !\n");    
       int i;
       for(i=0;i<=len/1920;i++)
       {
           usleep(100*1000);
           ret =fread(data,1,1920,fp);
           if(ret >= 0)
               QCamAudioOutputPlay(data,ret);
       }
       printf("play done !\n");
       fclose(fp);
#endif       
#endif 
       sleep(3);
    
     //  goto wait;
       QCamAudioOutputClose();

       QCamAudioInputClose();

 

    return 0;

}

