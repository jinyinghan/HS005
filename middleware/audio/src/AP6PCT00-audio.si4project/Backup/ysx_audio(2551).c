#include "ysx_audio.h"
#include "ysx_sys.h"
#include <time.h>
#include <signal.h>
#include <imp/imp_log.h>
#include <imp/imp_audio.h>
#include "audio_shm.h"
#define TAG "ysx-audio"


static pthread_t record_tid = -1 ,pb_tid;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int play_done = 1 ;
time_t lst_pb_time = 0;

unsigned char last_spken_stat;
struct timeval last_spken_time;
unsigned char running;

static struct ysx_audio_record record_attr;
static struct ysx_audio_pb pb_attr;
#define VOLUME_360
int IMP_AO_FlushChnBuf(int,int);

/**********************AUDIO  INPUT******************************/
void ysx_get_gain(int *val,char *symbol)
{

    char line_buf[128];
    char *match;
    FILE *fp;
    fp = fopen(CAMERA_CONF,"r");
    if(NULL == fp)
    { 
        goto end;
    }


     while(fgets(line_buf,128,fp) != NULL){
        if((match = strstr(line_buf,symbol)) != NULL)
         {
            match += strlen(symbol);
            *val = atoi(match);
            break;
        }              
    }
    fclose(fp);

end:    
    QLOG(TRACE,"Set %s value(%d)\n",symbol,*val);    
    return ;    
}



int QCamAudioInputOpen(QCamAudioInputAttr *pAttr) 
{
    int ret;
    uint8_t devID;
	IMPAudioIOAttr attr;

        
    memset(&record_attr,0,sizeof(struct ysx_audio_record ));
    
    if( pAttr->sampleBit != 8 && pAttr->sampleBit != 16){
		QLOG(FATAL, "sampleBit do not support %d\n",pAttr->sampleBit);
        return -1;
    }
    
    if( pAttr->sampleRate != AUDIO_SAMPLE_RATE_8000 && \
        pAttr->sampleRate != AUDIO_SAMPLE_RATE_16000 && \
        pAttr->sampleRate != AUDIO_SAMPLE_RATE_44100 && \
         pAttr->sampleRate != AUDIO_SAMPLE_RATE_48000 )
    {
		QLOG(FATAL, "sampleRate do not support %d\n",pAttr->sampleRate);
        return -1;
    }    
    
    /*Save audio input attribute*/
    memset(&record_attr, 0 , sizeof(record_attr));
    memcpy(&record_attr.attr , pAttr , sizeof(QCamAudioInputAttr));
    record_attr.enable = 0;
    record_attr.start  = 0;


 	/* Step 1: set public attribute of AI device. */
    devID = 1;
	attr.samplerate = pAttr->sampleRate;
	attr.bitwidth = pAttr->sampleBit;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 40;
	attr.numPerFrm = (40*attr.samplerate / 1000);
	attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(devID, &attr);
	if(ret != 0) {
		QLOG(FATAL, "set ai %d attr err: %d\n", devID, ret);
		return -1;
	}



	/* Step 2: enable AI device. */
	ret = IMP_AI_Enable(devID);
	if(ret != 0) {
		QLOG(FATAL, "enable ai %d err\n", devID);
		return -1;
	}

	/* Step 3: set audio channel attribute of AI device. */
	int chnID = 0;
	IMPAudioIChnParam chnParam;
	chnParam.usrFrmDepth = 40;
	ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		QLOG(FATAL, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return -1;
	}

	memset(&chnParam, 0x0, sizeof(chnParam));
	ret = IMP_AI_GetChnParam(devID, chnID, &chnParam);
	if(ret != 0) {
		QLOG(FATAL, "get ai %d channel %d attr err: %d\n", devID, chnID, ret);
		return -1;
	}

	QLOG(TRACE, "Audio In GetChnParam usrFrmDepth : %d\n", chnParam.usrFrmDepth);

    ret = QCamAudioInputSetVolume(pAttr->volume);
	if(ret != 0) {
		QLOG(FATAL, "Set Volume %d Failed .\n",pAttr->volume);
		return -1;
	}

    record_attr.mic_gain = 20;  // default value

    ysx_get_gain(&record_attr.mic_gain,SYMBOL_MIC_GAIN);
    
    ret = IMP_AI_SetGain(devID,chnID,record_attr.mic_gain);
	if(ret != 0) {
		QLOG(FATAL, "IMP_AI_SetGain Failed .\n");
		return -1;
	}

#if 0    
    ret = IMP_AI_EnableHpf(&attr);
    if(ret != 0) {
        printf("enable audio hpf error.\n");
        QLOG(FATAL, "enable audio hpf error.\n");
        return -1;
    }

    ret = IMP_AI_EnableNs(&attr, NS_VERYHIGH);
    if(ret != 0) {
        printf("enable audio ns error.\n");
        QLOG(FATAL, "enable audio ns error.\n");
        return -1;
    }
#endif
     ret = IMP_AI_EnableHpf(&attr);
     if(ret != 0) {
        printf("enable audio hpf error.\n");
        QLOG(FATAL, "enable audio hpf error.\n");
        return -1;
    }
 	IMPAudioAgcConfig agcConfig = {
		.TargetLevelDbfs = 0,
		.CompressionGaindB = 10
	};

	if(pAttr->sampleRate != AUDIO_SAMPLE_RATE_44100)
	{
	    ret = IMP_AI_EnableNs(&attr, NS_VERYHIGH);//降噪  
	    if(ret != 0) {
	        printf("enable audio ns error.\n");
	        QLOG(FATAL, "enable audio ns error.\n");
	        return -1;
	    }
        /*
		ret = IMP_AI_EnableAgc(&attr, agcConfig);//控制音量
		if(ret != 0) {
			QLOG(FATAL, "enable audio agc error.\n");
			return -1;
        }
        */
	}

    int gain;
    IMP_AI_GetGain(devID,chnID,&gain);
    printf("mic gain = %d\n",gain);

    record_attr.ChnId  = chnID;
    record_attr.DevId  = devID;      // 0--digital mic ; 1 --simulative mic 
	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AI_GetPubAttr(devID, &attr);
	if(ret != 0) {
		QLOG(FATAL, "get ai %d attr err: %d\n", devID, ret);
		return -1;
	}
	QLOG(TRACE, "Audio In  samplerate : %d", attr.samplerate);
	QLOG(TRACE, "Audio In    bitwidth : %d", attr.bitwidth);
	QLOG(TRACE, "Audio In   soundmode : %d", attr.soundmode);
	QLOG(TRACE, "Audio In      frmNum : %d", attr.frmNum);
	QLOG(TRACE, "Audio In   numPerFrm : %d", attr.numPerFrm);
	QLOG(TRACE, "Audio In      chnCnt : %d", attr.chnCnt);

    return 0;
}

void QCamAudioInputSetGain(int gain)
{
	int ret;
    uint8_t devID,chnID;
    
    devID = record_attr.DevId;
    chnID = record_attr.ChnId;    
	
    ret = IMP_AI_SetGain(devID,chnID,gain);
	if(ret != 0) {
		QLOG(FATAL, "IMP_AI_SetGain Failed .\n");
		return -1;
	}
	
    int val;
    IMP_AI_GetGain(devID,chnID,&val);
    printf("re set mic gain = %d\n",val);
	return ;
}


int QCamAudioInputClose()
{
    int ret;
    uint8_t devID,chnID;
    
    devID = record_attr.DevId;
    chnID = record_attr.ChnId;    

    record_attr.enable = 0;    
  //  sleep(1);
   if(record_tid && record_attr.start)
       pthread_join(record_tid, NULL); 

    if(devID)
    {
#if 0        
    	ret = IMP_AI_DisableHpf();
    	if(ret != 0) {
    		QLOG(FATAL, "enable audio hpf error.\n");
    		return -1;
    	}

    	ret = IMP_AI_DisableNs();
    	if(ret != 0) {
    		QLOG(FATAL, "disable audio ns error.\n");
    		return -1;
    	}

    	ret = IMP_AI_DisableAgc();
    	if(ret != 0) {
    		QLOG(FATAL, "disable audio ns error.\n");
    		return -1;
    	}      
#endif        
    	ret = IMP_AI_DisableAec(devID, chnID);
    	if(ret != 0) {
    		QLOG(-1, "Audio enable aec failed\n");
    	}        

        QLOG(TRACE,"disable audio input\n");
    	/* Step 9: disable the audio channel. */
    	ret = IMP_AI_DisableAecRefFrame(devID, chnID, 0, 0);
    	if(ret != 0) {
    		QLOG(FATAL, "Audio channel disable error\n");
    		return -1;
    	}
		
    	ret = IMP_AI_DisableChn(devID, chnID);
    	if(ret != 0) {
    		QLOG(FATAL, "Audio channel disable error\n");
    		return -1;
    	}

    	/* Step 10: disable the audio devices. */
    	ret = IMP_AI_Disable(devID);
    	if(ret != 0) {
    		QLOG(FATAL, "Audio device disable error\n");
    		return -1;
    	}
    }

    QLOG(TRACE,"AUDIO Input close!\n");
    return 0;
}

void *thr_ysx_audio_record()
{
    int ret;    
    struct timeval tv;
    uint8_t devID,chnID;
    IMPAudioFrame frm, ref; 
    
    prctl(PR_SET_NAME,"thr_ysx_audio_record");
    devID = record_attr.DevId;
    chnID = record_attr.ChnId;
    static int num;
    while(record_attr.enable)
    {
        if(record_attr.start)
        {
            /* Step 6: get audio record frame. */
            ret = IMP_AI_PollingFrame(devID, chnID, 1000);//
            if (ret != 0 ) {
                IMP_LOG_ERR(TAG, "Audio Polling Frame Data error\n");
                continue;
            }

            ret = IMP_AI_GetFrameAndRef(devID, chnID, &frm, &ref, BLOCK);
            if(ret != 0) {
                IMP_LOG_ERR(TAG, "Audio Get Frame Data error\n");
                break;
            }
			//if(num++%150==0)
			//printf("encode audio timestamp:%llu\n",frm.timeStamp);
            gettimeofday(&tv, NULL);  
            if (record_attr.attr.cb) {
                record_attr.attr.cb(&tv, frm.virAddr, frm.len, ref.virAddr);
            }
           /* Step 7: release the audio record frame. */
            ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
            if(ret != 0) {
                QLOG(FATAL, "Audio release frame data error\n");
                break;
            }       
            /* Step 8: Save the recording data to a file. */
            
        }
        else 
            sleep(1);

    }

    QLOG(TRACE ,"Stop Audio Record \n");
    pthread_exit(0);
}


// start/stop 用于控制是否采集callback发生
int QCamAudioInputStart()
{
    uint8_t devID,chnID;
    
    int ret;

    if(record_attr.start)
    {
		QLOG(FATAL, "Audio Record has already been started\n");
		return -1;
    }
    
    devID = record_attr.DevId;
    chnID = record_attr.ChnId;
	/* Step 4: enable AI channel. */
	ret = IMP_AI_EnableChn(devID, chnID);
	if(ret != 0) {
		QLOG(FATAL, "IMP_AI_EnableChn dev%d channel%d failed\n",devID,chnID);
		return -1;
	}    
	ret = IMP_AI_EnableAecRefFrame(devID, chnID, 0, 0);
	if(ret != 0) {
		QLOG(FATAL, "IMP_AI_EnableAecRefFrame dev%d channel%d failed\n",devID,chnID);
		return -1;
	}    

    record_attr.enable = 1; 
    record_attr.start  = 1;
    ret = pthread_create(&record_tid, NULL, thr_ysx_audio_record, NULL);
    if (ret < 0) 
    {
        QLOG(FATAL, "Failed Create pthread to get_h264_stream with result %d\n",ret);
        return -1;
    }         
    
    QLOG(TRACE, "Audio Start Record.\n");
    
    return 0;  
}

int QCamAudioInputStop()
{
 
    record_attr.start  = 0;
    
    return 0;
}

/*360接口:袅縖0~100]， -1 表示使用系统默认，不进行修改
 *Ingenic ;[-30 ~ 120]. -30代表静音,120表示将声音放大30dB,步长0.5dB. 60是音量设置的一个临界点，
 */
int QCamAudioInputSetVolume(int vol)
{
    int ret;
    int value;
    
#ifdef VOLUME_360
        if(vol == -1)
            vol = 90;
        value = vol-30;
    
#endif


    printf("set volume = %d\n",value);
	ret = IMP_AI_SetVol(record_attr.DevId, record_attr.ChnId, value);
	if(ret != 0) {
		QLOG(FATAL, "Audio Record set volume failed\n");
		return -1;
	}
    return 0;
}


int QCamAudioAecEnable(int enable)//回音消除功能
{
    int ret;
#ifdef NO_SPK
	return 0;
#else
    if(record_attr.enable != 1){
        QLOG(WARN,"record enable = %d, pb enable = %d\n",record_attr.enable);
        return -1;
    }

    if(enable){
        printf("enable aec\n");
        ret = IMP_AI_EnableAec(record_attr.DevId,record_attr.ChnId,pb_attr.DevId,pb_attr.ChnId);
        if(ret < 0)
            QLOG(TRACE, "Audio enable aec failed\n");            
    }else{

		return 0;
        ret = IMP_AI_DisableAec(record_attr.DevId,record_attr.ChnId);
        if(ret < 0)
            QLOG(TRACE, "Audio enable aec failed\n");                    
    }
#endif
    return ret;
}

int QCamAudioDenoiseEnable(int enable)//降噪
{
	int ret;
	IMPAudioIOAttr attr ;

	if(enable){
		ret = IMP_AI_GetPubAttr(record_attr.DevId , &attr);
		if(ret < 0 ){
	        QLOG(FATAL, "audio ai get attr error.\n");
	        return -1;
		}
		
	    ret = IMP_AI_EnableNs(&attr, NS_VERYHIGH);
	    if(ret != 0) {
	        QLOG(FATAL, "enable audio ns error.\n");
	        return -1;
	    }
	}
	else{
		ret = IMP_AI_DisableNs();
    	if(ret != 0) {
    		QLOG(FATAL, "disable audio ns error.\n");
    		return -1;
    	}
	}

	return ret;
}

/**********************AUDIO  INPUT******************************/



#define AUTO_PA
/**********************AUDIO  OUTPUT******************************/
int ao_shm_fd = 0;
audio_deamon_fmt *shm_buffer = NULL;
sem_t *deamon_sem_global;

static int util_time_clock(clockid_t clk_id, struct timeval* pTime)
{
    struct timespec stTime;
    memset(&stTime, 0, sizeof(struct timespec));
    memset(pTime, 0, sizeof(struct timeval));
    if (!clock_gettime(clk_id, &stTime))
    {
        pTime->tv_sec = stTime.tv_sec;
        pTime->tv_usec = stTime.tv_nsec/1000;
        return 0;
    }

    return -1;
}

int util_time_abs(struct timeval* pTime)
{
    return util_time_clock(CLOCK_MONOTONIC, pTime);
}

int util_time_local(struct timeval* pTime)
{
    return util_time_clock(CLOCK_REALTIME, pTime);
}

int util_time_sub(struct timeval* pStart, struct timeval* pEnd)
{
    int IntervalTime = (pEnd->tv_sec -pStart->tv_sec)*1000 + (pEnd->tv_usec - pStart->tv_usec)/1000;

    return IntervalTime;
}

int util_time_pass(struct timeval* previous)
{

    struct timeval cur = {0, 0};
    util_time_abs(&cur);

    return util_time_sub(previous, &cur);
}


void set_pa_status(int mode)
{    
	char sys_cmd[64];
	CLEAR(sys_cmd);

	QLOG(TRACE,"###########@@@@%d@@@@########",mode);
	if(shm_buffer == NULL)
	{
		QLOG(FATAL,"shm buffer is NULL !");
		return;
    }
	snprintf(sys_cmd,64,"echo %d > /proc/jz/aw8733a/mode", mode);
	system(sys_cmd);
    
	if(mode == 0 )
	{
		shm_buffer->status = AUDIO_OUT_CLOSE;
	}
	else
    {
		shm_buffer->status = AUDIO_OUT_OPEN;
	}
}

int get_pa_status()
{
	if(!shm_buffer)
	{
		printf("shm_buffer is NULL !\n");
        return -1;
    }    
	return shm_buffer->status;
}
    /*Save audio input attribute*/
int sem_p()
{
	/* Step 1: set public attribute of AO device. */
	int ret;
	ret = sem_wait(deamon_sem_global);
        
	if (0 != ret) {
		QLOG(FATAL,"err: sem_wait %s", strerror(errno));
        return -1;
    }
	return 0;
}

int sem_v()
{
	int ret ;
	ret = sem_post(deamon_sem_global);
	if (0 != ret) {
		QLOG(FATAL,"err: sem_post %s\n", strerror(errno));
        return -1;
    }
	return 0;
}

void ysx_audio_shm_deinit()
{
	int ret;
    /* Step 2: enable AO device. */
	ret = munmap(shm_buffer, SHM_SIZE);
	if(-1 == ret)
	{
		QLOG(FATAL, "munmap failed");
		return;
    }
}

int ysx_audio_shm_init()
{
	int ret = -1;
	ao_shm_fd = shm_open(SHM_NAME, SHM_OPEN_FLAG, SHM_OPEN_MODE);
	if (-1 == ao_shm_fd) {
		DP("err: shm_open err %s, %s\n", strerror(errno), SHM_NAME);
		goto err;
	}
	ret = ftruncate(ao_shm_fd, SHM_SIZE);
	if (-1 == ret) {
		DP("err: ftruncate err %s\n", strerror(errno));
		goto err;
    }
	shm_buffer =(audio_deamon_fmt *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ao_shm_fd, 0);
	if (MAP_FAILED == shm_buffer) {
		DP("err: mmap err %s\n", strerror(errno));
		goto err;
	}


	deamon_sem_global = sem_open(SEM_GLOBAL_NAME, O_CREAT, 0666, 1);
	if (SEM_FAILED == deamon_sem_global) {
#if 0
		if (EEXIST == errno) {
			sem_unlink(SEM_GLOBAL_NAME);
			DP("info: sem_unlink %s\n", SEM_GLOBAL_NAME);
			deamon_sem_global = sem_open(SEM_GLOBAL_NAME, O_CREAT | O_EXCL, 0666, 1);
			if (SEM_FAILED == deamon_sem_global) {
				DP("err: sem_open global err %s\n", strerror(errno));
				goto err;
}
		} else {
			DP("err: sem_open global err %s\n", strerror(errno));
			goto err;
		}
#else
		DP("err: sem_open global err %s\n", strerror(errno));
				goto err;
#endif				
	}
	int val;
	sem_getvalue(deamon_sem_global, &val);
	QLOG(TRACE,"shm val = %d\n",val);
//	if(shm_buffer->status == AUDIO_OUT_CLOSE )
		set_pa_status(0);
	return 0;
err:
	return -1;

}

static int spken_gpio_write(int pin , int value)
{
    int fd;
	char buf[128];
	
    memset(buf, 0, sizeof(buf));
    snprintf(buf, 128,"/sys/class/gpio/gpio%d/value",pin);
    fd = open(buf,O_WRONLY);
	if(fd < 0){
		QLOG(FATAL, "set gpio %d direction error %s!\n", pin, strerror(errno));
		return -1;
	}

    if (value == 0)
        write(fd,"0",sizeof("0"));
    else
        write(fd,"1",sizeof("1"));
    close(fd);
   
    return 0;
}

static int spken_gpio_open(int pin)
{
	char buf[128];

	if(pin < 0 || pin > 96){
		printf("illegal pin number!\n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf,128,"echo %d > /sys/class/gpio/export",pin);
	system(buf);    

	return 0;
}

static int spken_gpio_output(int pin)
{
	char buf[128];
	int fd;
	memset(buf, 0, sizeof(buf));	
	snprintf(buf,128,"/sys/class/gpio/gpio%d/direction",pin);
	fd = open(buf,O_RDWR);
	if(fd < 0){
		QLOG(FATAL, "set gpio %d output error %s!\n", pin, strerror(errno));
		return -1;
	}

	write(fd,"out",sizeof("out"));
	close(fd);

	return 0;
}

static int spken_gpio_init()
{
	int ret;
	ret = spken_gpio_open(SPK_EN_GPIO);
	if(ret < 0){
		QLOG(FATAL, "open SPK_EN_GPIO %d faield\n",SPK_EN_GPIO);
		return -1;
	}

	ret = spken_gpio_output(SPK_EN_GPIO);
	if(ret < 0){
		QLOG(FATAL, "gpio_output SPK_EN_GPIO %d faield\n",SPK_EN_GPIO);
		return -1;
	}

    spken_gpio_write(SPK_EN_GPIO, 0);  
	return 0;
}

static int audio_set_spken(int arg)
{
	pthread_mutex_lock(&mut);
	spken_gpio_write(SPK_EN_GPIO, arg);
	last_spken_stat = arg;
	pthread_mutex_unlock(&mut);

    return 0;
}

static void *audio_play_proc(void *args)
{
	int ret;

	char pr_name[64];
    memset(pr_name,0,sizeof(pr_name));
    sprintf(pr_name,"audio_play_proc");
    prctl(PR_SET_NAME,pr_name);
	
	while(running) {	
		int pass_time = util_time_pass(&last_spken_time);
        if (pass_time > 5*1000){
            if (last_spken_stat != SPK_EN_CLOSE)
				audio_set_spken(SPK_EN_CLOSE);
        }

        usleep(100*1000);
	}

	QLOG(TRACE, "audio_play_proc exit OK\n");
	pthread_exit(0);
}

void *thr_ysx_pa_control()
{
	int ret;
	prctl(PR_SET_NAME,"thr_ysx_pa_control");

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	while(1)
    {      
		sleep(1);
		pthread_mutex_lock(&mut);
		if(play_done == 0 && abs(time(NULL)-shm_buffer->lst_time) > 5)
		{		
			QLOG(TRACE,"Time Out 5s !");
			shm_buffer->lst_time = time(NULL);
			play_done = 1;
			sem_v();
    }
		pthread_mutex_unlock(&mut);		
	}
}

int QCamAudioOutputOpen(QCamAudioOutputAttribute *pAttr)
{
	int ret;
#ifdef NO_SPK
	return 0;
#else
    memset(&pb_attr,0,sizeof(struct ysx_audio_pb));

	/* Step 1: set public attribute of AO device. */
	int devID = 0;
	IMPAudioIOAttr attr;
	attr.samplerate = pAttr->sampleRate;
	attr.bitwidth = pAttr->sampleBit;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 10;
	attr.numPerFrm = (40*attr.samplerate / 1000);
	attr.chnCnt = 1;
    
    
	ret = IMP_AO_SetPubAttr(devID, &attr);
	if(ret != 0) {
		QLOG(FATAL, "set ao %d attr err: %d\n", devID, ret);
		return -1;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "get ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr samplerate:%d\n", attr.samplerate);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr   bitwidth:%d\n", attr.bitwidth);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  soundmode:%d\n", attr.soundmode);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     frmNum:%d\n", attr.frmNum);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  numPerFrm:%d\n", attr.numPerFrm);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     chnCnt:%d\n", attr.chnCnt);

	
	/* Step 2: enable AO device. */
	ret = IMP_AO_Enable(devID);
	if(ret != 0) {
		QLOG(FATAL, "enable ao %d err\n", devID);
		return -1;
	}

	/* Step 3: enable AI channel. */
	int chnID = 0;
	ret = IMP_AO_EnableChn(devID, chnID);
	if(ret != 0) {
		QLOG(FATAL, "Audio play enable channel failed\n");
		return -1;
	}
	
	
	
	if(D603_2){
    pb_attr.spk_gain = 24;      //default value
	}else{
		pb_attr.spk_gain = 23;		//default value
	}
    ysx_get_gain(&pb_attr.spk_gain,SYMBOL_SPK_GAIN);
    
    ret = IMP_AO_SetGain(devID,chnID,pb_attr.spk_gain);
	if(ret != 0) {
		QLOG(FATAL, "IMP_AO_SetGain Failed .\n");
		return -1;
	} 
    int gain;
    IMP_AO_GetGain(devID,chnID,&gain);
    printf("speaker gain = %d",gain);

	ret = IMP_AO_EnableHpf(&attr);
	if(ret != 0) {
		printf("enable audio hpf error.\n");
		QLOG(FATAL, "enable audio hpf error.\n");
		return -1;
	}

	IMPAudioAgcConfig agcConfig;
	agcConfig.TargetLevelDbfs = 6;
	agcConfig.CompressionGaindB = 10;
    
    QLOG(TRACE,"AO Agc config : TargetLevelDbfs=%d ,CompressionGaindB=%d",agcConfig.TargetLevelDbfs,agcConfig.CompressionGaindB);
	ret = IMP_AO_EnableAgc(&attr, agcConfig);
	if(ret != 0) {
		printf("enable audio agc error.\n");
		QLOG(FATAL, "enable audio agc error.\n");
		return -1;
	}
#if 0
	if(ysx_audio_shm_init() != 0)
	{
		QLOG(FATAL,"shm init error !");
		return -1;
	}
	QLOG(TRACE,"##########shm init ok !\n");
    ret = pthread_create(&pb_tid, NULL, thr_ysx_pa_control, NULL);
    if (ret < 0) 
    {
        QLOG(FATAL, "Failed Create pthread to thr_ysx_pa_control with result %d\n",ret);
        return -1;
    }         
#endif
#if 0
	if(spken_gpio_init() != 0){
		QLOG(FATAL,"spken_gpio_init error !");
		return -1;
	}
	running = 1;
	audio_set_spken(SPK_EN_CLOSE);
    util_time_abs(&last_spken_time);
	QLOG(TRACE,"##########spken_gpio_init ok !\n");
    ret = pthread_create(&pb_tid, NULL, audio_play_proc, NULL);
    if (ret < 0) {
        QLOG(FATAL, "Failed Create pthread to audio_play_proc with result %d\n",ret);
        return -1;
    } 
#endif
    pb_attr.buf_size = 40*attr.samplerate * attr.bitwidth / 8 / 1000;
    pb_attr.buf = (unsigned char *)malloc(pb_attr.buf_size);
    if(pb_attr.buf == NULL) {
		QLOG(FATAL, "[ERROR] %s: malloc audio buf error\n", __FUNCTION__);
		return -1;
	}
	QLOG(TRACE, "malloc audio buf size:%d", pb_attr.buf_size);
	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(devID, &attr);
	if(ret != 0) {
		QLOG(FATAL, "get ao %d attr err: %d\n", devID, ret);
		return -1;
	}
	QLOG(TRACE, "Audio Out GetPubAttr samplerate:%d", attr.samplerate);
	QLOG(TRACE, "Audio Out GetPubAttr   bitwidth:%d", attr.bitwidth);
	QLOG(TRACE, "Audio Out GetPubAttr  soundmode:%d", attr.soundmode);
	QLOG(TRACE, "Audio Out GetPubAttr     frmNum:%d", attr.frmNum);
	QLOG(TRACE, "Audio Out GetPubAttr  numPerFrm:%d", attr.numPerFrm);
	QLOG(TRACE, "Audio Out GetPubAttr     chnCnt:%d", attr.chnCnt);
    QCamAudioOutputSetVolume(pAttr->volume);
    pb_attr.ChnId  = chnID;
    pb_attr.DevId  = devID;      // 0--default speaker
    return 0;
#endif	
}


int QCamAudioOutputClose()
{    
    int ret;
#ifdef NO_SPK
	return 0;
#else	
    int devID,chnID;
	IMPAudioOChnState status;
    devID = pb_attr.DevId;
    chnID = pb_attr.ChnId;

    if (running){
        running = 0;
        pthread_join(pb_tid, NULL);
    }	

    if(pb_attr.buf_size != 0)
    {
		IMP_AO_FlushChnBuf(devID, chnID);

		do{
			memset(&status,0,sizeof(IMPAudioOChnState));
			ret = IMP_AO_QueryChnStat(devID,chnID,&status);
			if(ret < 0)
			{
				QLOG(FATAL, "IMP_AO_QueryChnStat failed\n");
				break;
			}
		}while(status.chnBusyNum);
		sleep(1);
        IMP_AO_ClearChnBuf(devID,chnID);
#if 0		
		sem_v();
		pthread_cancel(pb_tid);
		ysx_audio_shm_deinit();
#endif
    	/* Step 9: disable the audio channel. */
    	ret = IMP_AO_DisableChn(devID, chnID);
    	if(ret != 0) {
    		QLOG(FATAL, "Audio channel disable error\n");
    		return -1;
    	}

    	/* Step 10: disable the audio devices. */
    	ret = IMP_AO_Disable(devID);
    	if(ret != 0) {
    		QLOG(FATAL, "Audio device disable error\n");
    		return -1;
    	}
        

        if(NULL != pb_attr.buf)
            free(pb_attr.buf);

    }

    QLOG(TRACE,"AUDIO Output close!\n");
    
    return 0;
#endif	
}


int QCamAudioOutputPlay(char *pcm_data, int len)
{
#ifdef NO_SPK
	return 0;
#else	
    int ret;
    uint8_t *buf,*start;
    uint32_t has_left,totel;
    uint8_t devID,chnID;
    IMPAudioFrame frm;
    
    devID = pb_attr.DevId;
    chnID = pb_attr.ChnId;
    buf = pb_attr.buf;
    
    start = (uint8_t *)pcm_data;
    has_left = totel = len;
#if 0
	if(access("/tmp/audio_on",F_OK) == 0 )
	{
		if(!shm_buffer)
			QLOG(TRACE,"shm buffer is null");
		QLOG(TRACE,"PA status = %d , PID = %d",get_pa_status(),getpid());
	}

	pthread_mutex_lock(&mut);
	if(shm_buffer->status == AUDIO_OUT_CLOSE)
	{
		set_pa_status(2);	
	}
	pthread_mutex_unlock(&mut);
#endif
#if 0
	if (last_spken_stat != SPK_EN_OPEN)
		audio_set_spken(SPK_EN_OPEN);
	
    util_time_abs(&last_spken_time);
#endif
	while(1) 
    {
        if(has_left >= pb_attr.buf_size)  //大于一帧
        {
            memcpy(buf,start+(totel-has_left),pb_attr.buf_size);
            has_left -= pb_attr.buf_size;

        
            /* Step 5: send frame data. */        
            frm.virAddr = (uint32_t *)buf;
            frm.len = pb_attr.buf_size;
            ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
            if(ret != 0) {
                QLOG(FATAL, "send Frame Data error\n");
                return -1;
            }
//            printf("has_left = %d\n",has_left);

        }else{
            memcpy(buf,start+(totel-has_left),has_left);

            /* Step 5: send frame data. */        
            frm.virAddr = (uint32_t *)buf;
            frm.len = has_left;
            ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
            if(ret != 0) {
                QLOG(FATAL, "send Frame Data error\n");
                return -1;
            }            
           
           break;
        }
    }

#if 0
	pthread_mutex_lock(&mut);
	shm_buffer->lst_time = time(NULL); 
	play_done = 0;
	pthread_mutex_unlock(&mut);
#endif
    return 0;
#endif
}


int QCamAudioOutputSetVolume(int vol)
{
#ifdef NO_SPK
		return 0;
#else

    int ret;
    int value;
    value = vol;

//#ifdef VOLUME_360
//    if(vol == -1)
//        vol = 90;
//    value = vol-30;

//#endif
    printf("before transfer:vol=%d\n",vol);
#if 1
    if(value == -1)
        value = 90; 
    if(value > 100)
        value = 100;
    if(value < 0 )
        value = 0;

   
#if AGC_ENABLE        
   if(value < 10)
        value = 11*value/2-30;
   else if(value < 45)
        value = value+1;
    else if(value <= 100)
        value = value/6+30;

#endif

    if(value < 10)
         value = 11*value/2-30;
    else if(value < 45)
         value = value*8/9+5;
    else if(value < 75)
        value = value/2+22;
    else if(value <= 100)
         value = value/3+32;

#endif

    printf("set output volume = %d\n",value);
    ret = IMP_AO_SetVol(pb_attr.DevId, pb_attr.ChnId, value);
    if(ret != 0) {
        QLOG(FATAL, "Audio Speaker set volume failed\n");
        return -1;
    }
    
    return 0;
#endif	
}


int QCamAudioOutputQueryBuffer(QCamAudioOutputBufferStatus *pStat)
{
#ifdef NO_SPK
	return 0;
#else	
    int ret;
    IMPAudioOChnState status;
    if(NULL == pStat)
    {
        QLOG(FATAL, "pStat is null\n");
        return -1;   
    }

    memset(&status,0,sizeof(IMPAudioOChnState));
    ret = IMP_AO_QueryChnStat(pb_attr.DevId,pb_attr.ChnId,&status);
    if(ret < 0)
    {
        QLOG(FATAL, "IMP_AO_QueryChnStat failed\n");
        return -1;
    }
//    printf("totol=%d,busy=%d,free=%d\n",status.chnTotalNum,status.chnBusyNum,status.chnFreeNum);
	const int frame_duration = 40;
	pStat->total = status.chnTotalNum * frame_duration;
    pStat->busy  = status.chnBusyNum * frame_duration;

    return 0;
#endif	
}


/**********************AUDIO  OUTPUT******************************/

