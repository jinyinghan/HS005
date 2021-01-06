#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ysx_audio.h"


FILE *fp = NULL;
void ysx_get_record(const struct timeval *tv, const void *pcm_buf, const int pcm_len)
{
//    printf("cb :tv = %d ,  len = %d\n",tv->tv_sec,pcm_len);

    QCamAudioOutputPlay((char *)pcm_buf,pcm_len);
    fwrite(pcm_buf,1,pcm_len,fp);
    fflush(fp);    
}


int record_print_usage()
{

    printf("***1. bitwidth = 16 , samplerate = 8K  *****\n");
    printf("***2. bitwidth = 16 , samplerate = 16K  ****\n");
    printf("***3. bitwidth = 16 , samplerate = 44.1K****\n");    
    printf("record format ,please input:\n");

}

int main()
{
    int ret;
    uint8_t ch; 
    uint8_t tmp[64];
    QCamAudioInputAttr attr;

    attr.sampleBit = 16;
    attr.sampleRate = 0;
    attr.volume = 75;
    attr.cb = ysx_get_record;
    QCamAudioOutputAttribute pb_attr;
    
     
     pb_attr.sampleBit = 16;
     pb_attr.sampleRate = 8000;
     pb_attr.volume = 65;

    while(1){
        record_print_usage();
        ch = '1';//getchar();
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
    
    snprintf(tmp,64,"sample-%dKHz-%dbit_Aec.pcm",attr.sampleRate,attr.sampleBit);
    printf("Save pcm file %s\n",tmp);
    fp = fopen(tmp,"wb");
    if(NULL == fp){
        printf("open file %s error!\n",tmp);
        return -1;
    }
    ret = QCamAudioOutputOpen(&pb_attr);
    
    QCamAudioInputOpen(&attr);
    if(ret < 0) 
    {
        QLOG(FATAL, "QCamAudioInputOpen Failed\n");
        return -1;
    }

    QCamAudioInputStart();
    QCamAudioAecEnable(1);

    getchar();
    QCamAudioInputStop();


    QCamAudioInputClose();
    QCamAudioOutputClose();

    return 0;
}
