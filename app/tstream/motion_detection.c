
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>      
#include <sys/select.h>    
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>

#include "log.h"
#include "common_env.h"
#include "common_func.h"

#include "network.h"
#include "media.h"
#include "qcam_motion_detect.h"
#include "qcam_smart.h"

static unsigned char m_registerok = 0;


int check_motion_durtion (time_t seconds)
{
        int i_ret = 0;
        static time_t dis_seconds;

        if((seconds - dis_seconds) >= MOTION_DTC_TIME_OUT) {
                EMGCY_LOG("@@@@@@@@@@MD post md message! last_time=%ld, curr_time=%ld\n", dis_seconds, seconds);
                dis_seconds = seconds;
                i_ret = 0;
        }
        else {
                i_ret = -1;
        }
        return i_ret;
}



int dtc_alert_spk (time_t seconds)
{
        int i_ret = 0;
        static time_t dis_seconds = 0;
        int i_cnt = 0, i_cnt_max = 2;
        if((seconds-dis_seconds) >= MOTION_DTC_ALERT_TIME_OUT) {
                for(i_cnt=0; i_cnt<i_cnt_max; i_cnt++){
                        if(GetYSXCtl(YSX_VINFO_EN))
                                AMCSystemCmd("sample_play 16000 /usr/notify/dtc_alarm.pcm");
                        //			usleep(100*1000);
                }
                i_ret = 0;
        }
        else {
                i_ret = -1;
        }
        pr_dbg("time %ld, %ld\n", seconds, dis_seconds);
        return i_ret;
}





int g_iIsMotion=0;
void main_motion_cb(int md_result, int ir_status)
{
        //    EMGCY_LOG("@@@@@ md result: %d, ir status: %d\n", md_result, ir_status);
        unsigned int cStart, cEnd;
        int lightret = 0;
        int i_ret = 0;

        printf("++ motion ++\n");

        if(!g_enviro_struct.standalone_mode){
                if(0 == m_registerok)
                        return;
        }

#ifdef PATRACK
        if(GetYSXCtl(YSX_CAMERA_MOVE) == true || GetYSXCtl(YSX_MD_REGION_BIT) == 0){
                return;
        }
#else
        if(GetYSXCtl(YSX_MOTION) == 0 || GetYSXCtl(YSX_CAMERA_MOVE) == 0 || GetYSXCtl(YSX_MD_REGION_BIT) == 0){
                return;
        }
#endif
        if ((md_result & GetYSXCtl(YSX_MD_REGION_BIT))==0)
                return;

#ifdef S301 
        g_iIsMotion=1;
        return;
#endif

        printf("++ motion2 ++\n");

        GetMotionTime(&cStart, &cEnd);

        time_t timep;
        struct tm *sync_start_time;
        int invaild_time  = 0;
        timep = time(NULL);
        sync_start_time = localtime(&timep);
        if(cStart > cEnd )	
        {
                if((sync_start_time->tm_hour >= cEnd)&&(sync_start_time->tm_hour < cStart))
                        invaild_time = 1;

        }else if(cStart < cEnd ) /*0-24*/
        {
                if ((sync_start_time->tm_hour < cStart) || (sync_start_time->tm_hour >= cEnd))
                        invaild_time = 1;
        }else{
                ;/*start == end */
        }

        if(invaild_time)
        {
                LOG("md time region(%d-%d) now %d\n", cStart, cEnd, sync_start_time->tm_hour);
                sleep(5);
                return;
        }

#ifdef PATRACK
        if (GetYSXCtl(YSX_MOTION) == 0){
                EMGCY_LOG("motion not allow: %d\n", GetYSXCtl(YSX_MOTION));
                return;
        }
#endif

        i_ret = check_motion_durtion (g_enviro_struct.motion_timep);
        if(i_ret != 0)
                return;

}


void init_motion_detect(void)
{
	QCamInitMotionDetect2(main_motion_cb);
	return ;
}



void init_smart_ai(void)
{
	QCamSmartInParam pInParam;
	memset(&pInParam,0,sizeof(pInParam));
	pInParam.width = 640;
	pInParam.height = 360;
	pInParam.smart_enable = 20;
	QCamSmartCreate(&pInParam);
	
	create_smt_draw();
}

void uninit_smart_ai(void)
{
	destroy_smt_draw();
		
	QCamSmartDestory();
	
}


