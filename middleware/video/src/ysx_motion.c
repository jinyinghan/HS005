#include "ysx_video.h"
#include <imp/imp_ivs.h>
#include <imp/imp_ivs_move.h>
#include <imp/imp_log.h>

#define SENSE_CONF "/etc/SNIP39/sense.conf"
#define SYMBOL_FACE "sense_face="
#define SYMBOL_WIND "sense_window="
#define SYMBOL_RECORD "sense_record="
#define SYMBOL_FULLSCREEN "sense_fullscreen="


extern bool IROnStatus;

QCam_MD_cb md_cb=NULL;

typedef (* motion_detect_cb)(void * args);


typedef struct {
    int sense;
    unsigned int window;
    IMPIVSInterface * move;
} motion_param;



typedef struct {
    int started;
    int destroy_thread;
    pthread_t thread_id;
    char thread_name[32];
    QCam_MD_cb qcam_cb;
    ysx_motion_cb cb;
    void * cbargs;
    motion_param mv_param;
    pthread_mutex_t mutex;
	unsigned char running;
} ysx_motion_detect;



ysx_motion_detect g_motion;


static int sense_enable[IMP_IVS_MOVE_MAX_ROI_CNT];



//motion sense
int g_sensor_sense = 0;

void set_sensor_sense(int sense)
{
	if ((sense >= 0) && (sense <= 4)) {
		g_sensor_sense = sense;
	}
	EMGCY_LOG("set sense value: %d\n", g_sensor_sense);
}

int get_sensor_sense(void)
{
	return g_sensor_sense;
	EMGCY_LOG("read sense value: %d\n", g_sensor_sense);
}



static int set_thread_name(pthread_t thread, const char * name)
{
    if (thread < 0 || NULL == name) {
        fprintf(stderr, "argument error!\n");
        return -1;
    }
    prctl(PR_SET_NAME, name);
    return 0;
}



int ysx_sense_config(IMP_IVS_MoveParam *param)
{
    char line_buf[128];
    char *match;
    FILE *fp;
    fp = fopen(SENSE_CONF,"r");
    if(NULL == fp)
    {
        QLOG(TRACE,"can not find %s, user default config\n",SENSE_CONF);
        return -1;
    }


     while(fgets(line_buf,128,fp) != NULL){
        if((match = strstr(line_buf,SYMBOL_FACE)) != NULL)
         {
            match += strlen(SYMBOL_FACE);
            param->sense[0]= atoi(match);            
        }
        if((match = strstr(line_buf,SYMBOL_WIND)) != NULL)
         {
            match += strlen(SYMBOL_WIND);
            param->sense[1] = atoi(match);            
        }
        if((match = strstr(line_buf,SYMBOL_RECORD)) != NULL)
         {
            match += strlen(SYMBOL_RECORD);
            param->sense[2] = atoi(match);            
        }
        if((match = strstr(line_buf,SYMBOL_FULLSCREEN)) != NULL)
         {
            match += strlen(SYMBOL_FULLSCREEN);
            param->sense[3] = atoi(match);            
        }        
    }
    fclose(fp);

    return 0;
}




#define MOTION_CONF_PATH    "/etc/SNIP39/md.conf"
static int file_exist(const char * filename)
{
    if (0 == access(filename, F_OK)) {
        return 1;
    }
    return 0;
}

int get_motion_conf(motion_param * param)
{
    int ret = 0;
    if (NULL == param) {
        fprintf("%s() argument error!\n", __FUNCTION__);
        return -1;        
    }
    memset(param, '\0', sizeof(motion_param));
    if (file_exist(MOTION_CONF_PATH)) {
        pthread_mutex_lock(&g_motion.mutex);
        FILE * fp = fopen(MOTION_CONF_PATH, "rb");
        if (NULL == fp) {
            fprintf(stderr, "open file %s failed with: %s!\n", MOTION_CONF_PATH, strerror(errno));
            return -1;
        }
        
        fread(param, sizeof(char), sizeof(motion_param), fp);
        fflush(fp);
        fclose(fp); 

        fprintf(stderr, "[get] sense: %d, window: %d\n", param -> sense, param -> window);
        pthread_mutex_unlock(&g_motion.mutex);      
        return 0;
    }
    else {
        return -1;
    }
}


int set_motion_conf(motion_param * param)
{
    int ret = 0;
    motion_param param_def;
    memset(&param_def, '\0', sizeof(motion_param)); 
    if (NULL == param) {
        fprintf("%s() argument error!\n", __FUNCTION__);
        return -1;
    }

    if (file_exist(MOTION_CONF_PATH)) {      
        pthread_mutex_lock(&g_motion.mutex);
        FILE * fp = fopen(MOTION_CONF_PATH, "wb");
        if (NULL == fp) {
            fprintf(stderr, "open file %s failed with: %s!\n", MOTION_CONF_PATH, strerror(errno));
            return -1;
        }
        
        fwrite(param, sizeof(char), sizeof(motion_param), fp);  
        fflush(fp);
        fclose(fp);
        
        fprintf(stderr, "[set] sense: %d, window: %d\n", param -> sense, param -> window);
        pthread_mutex_unlock(&g_motion.mutex);
        return 0;
    }
    else {
        return -1;
    }
}


static int ysx_ivs_move_start(int grp_num, int chn_num)
{
	int ret = 0;
	IMP_IVS_MoveParam param;
	int i = 0, j = 0;
 //   IMPIVSInterface * interface;
    
	memset(&param, 0, sizeof(IMP_IVS_MoveParam));
	param.skipFrameCnt = 5;
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	param.roiRectCnt = 12;
    //灵敏度的取值范围为0~4,灵敏度一次递增

	printf("####### 1 #########\n");
	/*  0 | 1 | 2  | 3
		4 | 5 | 6  | 7
		8 | 9 | 10 | 11
		3*4区域
	*/
    
	for(i=0; i<3; i++)
		for (j=0; j<4; j++) {
			param.roiRect[(i*4)+j].p0.x = j*(640/4);
	        param.roiRect[(i*4)+j].p0.y = i*(360/3);
	        param.roiRect[(i*4)+j].p1.x = (j+1)*(640/4) - 1;
	        param.roiRect[(i*4)+j].p1.y = (i+1)*(360/3) - 1;
			param.sense[(i*4)+j] = 2;

		}

    
	g_motion.mv_param.move = IMP_IVS_CreateMoveInterface(&param);
	if (g_motion.mv_param.move == NULL) {
		QLOG(FATAL, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, g_motion.mv_param.move);
	if (ret < 0) {
		QLOG(FATAL, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		QLOG(FATAL, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		QLOG(FATAL, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	printf("####### 2 #########\n");

	return 0;
}



static int ysx_ivs_move_start2(int grp_num, int chn_num)
{
	int ret = 0;
	IMP_IVS_MoveParam param;
	int i = 0, j = 0;
    IMPIVSInterface * interface;
    
	memset(&param, 0, sizeof(IMP_IVS_MoveParam));
	param.skipFrameCnt = 5;
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	param.roiRectCnt = 12;
    //灵敏度的取值范围为0~4,灵敏度一次递增

	printf("####### 1 #########\n");
	/*  0 | 1 | 2  | 3
		4 | 5 | 6  | 7
		8 | 9 | 10 | 11
		3*4区域
	*/
    
	for(i=0; i<3; i++)
		for (j=0; j<4; j++) {
			param.roiRect[(i*4)+j].p0.x = j*(640/4);
	        param.roiRect[(i*4)+j].p0.y = i*(360/3);
	        param.roiRect[(i*4)+j].p1.x = (j+1)*(640/4) - 1;
	        param.roiRect[(i*4)+j].p1.y = (i+1)*(360/3) - 1;
			param.sense[(i*4)+j] = 2;

		}

    
	interface = IMP_IVS_CreateMoveInterface(&param);
	if (interface == NULL) {
		QLOG(FATAL, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, interface);
	if (ret < 0) {
		QLOG(FATAL, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		QLOG(FATAL, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		QLOG(FATAL, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	printf("####### 2 #########\n");

	return 0;
}


static int sample_ivs_move_stop(int chn_num)
{
	int ret = 0;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	sleep(1);

	ret = IMP_IVS_UnRegisterChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
		return -1;
	}

	IMP_IVS_DestroyBaseMoveInterface(g_motion.mv_param.move);

	return 0;
}


void * ysx_motion(void *arg)
{
    ysx_ivs_move_start(0, 0);    
    int i = 0, ret = 0;


    if (strlen(g_motion.thread_name) > 0) {
        set_thread_name(g_motion.thread_id, g_motion.thread_name);
    }
    else {
        set_thread_name(g_motion.thread_id, "ysx-motion");
    }
    
	unsigned int md_result;
    int chn_num = (int)arg;
    IMP_IVS_MoveOutput *result = NULL;

    int motion_type[4] = {QCAM_MD_FACE,QCAM_MD_WINDOW,QCAM_MD_RECORD,QCAM_MD_FULLSCREEN};
	
	prctl(PR_SET_NAME,"ysx_motion");
    while(g_motion.running)
    {
        ret = IMP_IVS_PollingResult(chn_num, -1);
        if (ret < 0) {
            QLOG(FATAL, "IMP_IVS_PollingResult(%d, -1) failed\n", chn_num);
            break;
        }
        ret = IMP_IVS_GetResult(chn_num, (void **)&result);
        if (ret < 0) {
            QLOG(FATAL, "IMP_IVS_GetResult(%d) failed\n", chn_num);
            break;
        } 
		
		md_result = 0;
		for (i = 0; i < 12; i ++) {
			md_result |= (result->retRoi[i]<<i);
		}

		if (md_result != 0) {
            g_motion.cb(md_result, IROnStatus);
        }
        
        ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
        if (ret < 0) {
            QLOG(FATAL, "IMP_IVS_ReleaseResult(%d) failed\n", chn_num);
            break;;
        }

        if (g_motion.destroy_thread) {
            g_motion.started = 0;
            break;
        }
    }
	
	return (void *)0;
}



void *ysx_motion2(void *arg)
{
    ysx_ivs_move_start(0, 0);
    
    int i = 0, ret = 0;
	unsigned int md_result;
    int chn_num = (int)arg;
    IMP_IVS_MoveOutput *result = NULL;

    int motion_type[4] = {QCAM_MD_FACE,QCAM_MD_WINDOW,QCAM_MD_RECORD,QCAM_MD_FULLSCREEN};
	
	prctl(PR_SET_NAME,"ysx_motion");
    while(1)
    {
        ret = IMP_IVS_PollingResult(chn_num, -1);
        if (ret < 0) {
            QLOG(FATAL, "IMP_IVS_PollingResult(%d, -1) failed\n", chn_num);
            break;
        }
        ret = IMP_IVS_GetResult(chn_num, (void **)&result);
        if (ret < 0) {
            QLOG(FATAL, "IMP_IVS_GetResult(%d) failed\n", chn_num);
            break;
        }
//        QLOG(TRACE, "@@@@@@@@@@frame[%d], result->retRoi(%d,%d,%d,%d)\n", i, result->retRoi[0], result->retRoi[1], result->retRoi[2], result->retRoi[3]);
#if 0
        for(i=0;i<4;i++)
            if(sense_enable[i] && result->retRoi[i])
                md_cb(motion_type[i]);
#endif  
		md_result = 0;
		for (i = 0; i < 12; i ++) {
			md_result |= (result->retRoi[i]<<i);
		}
		
		if (md_result != 0) {
			md_cb(md_result);
		}
		
        ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
        if (ret < 0) {
            QLOG(FATAL, "IMP_IVS_ReleaseResult(%d) failed\n", chn_num);
            break;;
        }
#if 0
        if (i % 20 == 0) {
            ret = sample_ivs_set_sense(chn_num, i % 5);
            if (ret < 0) {
                QLOG(FATAL, "sample_ivs_set_sense(%d, %d) failed\n", chn_num, i % 5);
                return (void *)-1;
            }
        }
#endif
    }

    pthread_exit(0);
}

void QCamStartFaceDetect()
{
    sense_enable[0] = 1;  
}

void QCamStopFaceDetect()
{
    sense_enable[0] = 0;  
}
void QCamStartWindowAlert(double p1x, double p1y, double p2x, double p2y)
{
    sense_enable[1] = 1;

}
void QCamStopWindowAlert()
{
    sense_enable[1] = 0;

}
void QCamStartCloudRecord()
{
    sense_enable[2] = 1;
}

void QCamStopCloudRecord()
{
    sense_enable[2] = 0;
}
void QCamStartFullScreenDetect()
{
    sense_enable[3] = 1;
}
void QCamStopFullScreenDetect()
{
    sense_enable[3] = 0;
}





void QCamInitMotionDetect(QCam_MD_cb cb)
{
    int ret = 0;
    if (NULL == cb) {
        QLOG(FATAL,"QCam_MD_cb is illegal!\n");
        return ;
    }   
	memset(&g_motion, 0, sizeof(ysx_motion_detect));
    g_motion.qcam_cb = cb;
    g_motion.destroy_thread = 0;
    g_motion.started = 1;

    pthread_mutex_init(&g_motion.mutex, NULL);
    // 如过默认配置文件不存在,给一组默认值并写入文件
    if (!file_exist(MOTION_CONF_PATH)) {                
        g_motion.mv_param.move = NULL;
        g_motion.mv_param.sense = 4;
        g_motion.mv_param.window = 0;
        set_motion_conf(&g_motion.mv_param);
    }  
    else {
        get_motion_conf(&g_motion.mv_param);
    }
 
    snprintf(&g_motion.thread_name, sizeof(g_motion.thread_name) - 1, "motion-proc");
    ret = pthread_create(&g_motion.thread_id, NULL, ysx_motion, NULL);
    if (ret < 0) {
        QLOG(FATAL,"Create pthread for motion detecton Failed!\n");
        return ;
    }
    return;
}


void QCamUninitMotionDetect()
{
    g_motion.destroy_thread = 1;
	g_motion.running = 0;

    pthread_join(g_motion.thread_id, NULL);
    sample_ivs_move_stop(0);
    pthread_mutex_destroy(&g_motion.mutex);
	QLOG(FATAL, "Release MotionDetect pthread\n");
}


void QCamInitMotionDetect2(ysx_motion_cb cb)
{
    int ret = 0;
    if (NULL == cb) {
        QLOG(FATAL,"QCam_MD_cb is illegal!\n");
        return ;
    }   
	memset(&g_motion, 0, sizeof(ysx_motion_detect));
    g_motion.cb = cb;
    g_motion.destroy_thread = 0;
    g_motion.started = 1;
	g_motion.running = 1;

    pthread_mutex_init(&g_motion.mutex, NULL);
    // 如过默认配置文件不存在,给一组默认值并写入文件
    if (!file_exist(MOTION_CONF_PATH)) {                
        g_motion.mv_param.move = NULL;
        g_motion.mv_param.sense = 4;
        g_motion.mv_param.window = 0;
        set_motion_conf(&g_motion.mv_param);
    }  
    else {
        get_motion_conf(&g_motion.mv_param);
    }
 
    snprintf(&g_motion.thread_name, sizeof(g_motion.thread_name) - 1, "motion-proc");
    ret = pthread_create(&g_motion.thread_id, NULL, ysx_motion, NULL);
    if (ret < 0) {
        QLOG(FATAL,"Create pthread for motion detecton Failed!\n");
        return ;
    }
    return;

}

int QCamSetMotionSense(int sense)
{
    if (g_motion.started) {
        fprintf(stderr, "motion already start!\n");
        return -1;
    }

    g_motion.mv_param.sense = sense; 
    set_motion_conf(&g_motion.mv_param);
    return 0;
}

int QCamSetMotionWindow(int window)
{    
    if (g_motion.started) {
        fprintf(stderr, "motion already start!\n");
        return -1;
    }

    g_motion.mv_param.window = window; 
    set_motion_conf(&g_motion.mv_param);
    return 0;
}



void QCamGetMotionSense(int * sense)
{
    * sense = g_motion.mv_param.sense;
}


void QCamGetMotionWindow(int * window)
{
    * window = g_motion.mv_param.window;
}


