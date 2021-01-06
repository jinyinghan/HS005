#include "ysx_audio.h"
#include<imp/imp_audio.h>
#include <signal.h>

FILE *fp = NULL;
static int cnt=0;
void ysx_get_record(const struct timeval *tv, const void *pcm_buf, const int pcm_len)
{
    printf("cb :tv = %d ,  len = %d\n",tv->tv_sec,pcm_len);
    
    fwrite(pcm_buf,1,pcm_len,fp);
    fflush(fp);
    return;
}


int record_print_usage()
{

    printf("***1. bitwidth = 16 , samplerate = 8K  *****\n");
    printf("***2. bitwidth = 16 , samplerate = 16K  ****\n");
    printf("***3. bitwidth = 16 , samplerate = 48K****\n");    
    printf("record format ,please input:\n");

}
void record_sig_fun(int sig)
{
	printf("********EXIT*******\n");

}

int main(int argc,char **argv)
{
    int ret;
    uint8_t ch; 
    uint8_t tmp[64];
    QCamAudioInputAttr attr;
    
//	signal(SIGTERM,record_sig_fun);
//	signal(SIGINT,record_sig_fun);

    attr.sampleBit = 16;
    attr.sampleRate = 0;
    attr.volume = 100;
    attr.cb = ysx_get_record;

    while(1){
        record_print_usage();
        ch ='2';// getchar();
        setbuf(stdin,NULL);
        switch(ch){
            case '1':
                attr.sampleRate = 8000; break;
            case '2':   
                attr.sampleRate = 16000;break;
            case '3':
                attr.sampleRate = 44100;break;
            default:
                printf("Invalid input\n");
        }
        if(attr.sampleRate)
            break;
    }
    
    snprintf(tmp,64,"%s/sample-%dKHz-%dbit.pcm",argv[1],attr.sampleRate,attr.sampleBit);
    printf("Save pcm file %s\n",tmp);
    fp = fopen(tmp,"wb");
    if(NULL == fp){
        printf("open file %s error!\n",tmp);
        return -1;
    }
    
    QCamAudioInputOpen(&attr);
    if(ret < 0) 
    {
        QLOG(FATAL, "QCamAudioInputOpen Failed\n");
        return -1;
    }

    QCamAudioInputStart();
    pause();

  //  QCamAudioInputStop();
    printf("**********\n");

    QCamAudioInputClose();
    system("date");
    fclose(fp);

    return 0;
}
