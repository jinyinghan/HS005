#include "ysx_audio.h"
#include <stdint.h>

FILE *fp_rec=NULL;
int play_print_usage()
{

    printf("***1. bitwidth = 16 , samplerate = 8K  *****\n");
    printf("***2. bitwidth = 16 , samplerate = 16K  ****\n");
    printf("***3. bitwidth = 16 , samplerate = 44.1K****\n");    
    printf("play pcm file format,please input:\n");
    return 0;
}
void ysx_get_record(const struct timeval *tv, const void *pcm_buf, const int pcm_len)
{
//    printf("cb :tv = %d ,  len = %d\n",tv->tv_sec,pcm_len);
    
    fwrite(pcm_buf,1,pcm_len,fp_rec);
    fflush(fp_rec);
    return;
}

int main(int argc,char **argv)
{
    int ret;
    FILE *fp;
    char ch;
    char tmp[64];

    
    QCamAudioOutputAttribute pb_attr;
    QCamAudioOutputBufferStatus status;
    if(argc < 3){
        printf("too few arguments\n");
        printf("example: sample_play samplerate sample-8000hz-16bit.pcm\n");
        return -1;
    }

    
    pb_attr.sampleBit = 16;
    if(argv[3] == NULL)
    pb_attr.volume = 120;
    else
    pb_attr.volume = atoi(argv[3]);
	pb_attr.sampleRate = atoi(argv[1]);

	printf("volume: %d\n", pb_attr.volume);

wait:    
//    while(1){
//        play_print_usage();
//        pb_attr.sampleRate = 0;        
//        ch = atoi(argv[1]);//getchar();
//        setbuf(stdin, NULL);
//        switch(ch){
//            case '1':
//                pb_attr.sampleRate = 8000; break;
//            case '2':   
//                pb_attr.sampleRate = 16000;break;
//            case '3':
//                pb_attr.sampleRate = 44100;break;
//            default:
//                printf("Invalid input\n");
//        }
//        if(pb_attr.sampleRate)
//            break;
//    }
//    snprintf(tmp,64,"sample-%dKHz-%dbit.pcm",pb_attr.sampleRate,pb_attr.sampleBit);
    strcpy(tmp,argv[2]);
    printf("play pcm file %s ..\n",tmp);  
    ret = QCamAudioOutputOpen(&pb_attr);
    if(ret < 0) 
    {
        QLOG(TRACE,"QCamAudioOutputOpen Failed \n");
        return -1;
    }

    fp = fopen(tmp,"rb");
    if(NULL == fp){
        printf("open file %s failed !\n",tmp);
        return -1;
    }
    fseek(fp,0,SEEK_END);
    int len = ftell(fp);
    fseek(fp,0,SEEK_SET);

    
    char  *data = (char   *)malloc(len);
    
       fread(data,1,len,fp);
       fclose(fp);
   
//    while(1)
         QCamAudioOutputPlay_ysx(data,len);

  //  goto wait;
    QCamAudioOutputClose();
 

    free(data);

   // sleep(3);
    return 0;

}

