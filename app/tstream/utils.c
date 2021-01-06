
#include "include/common_env.h"
#include "include/common_func.h"
#include <sys/types.h>
#include <sys/wait.h>

#define debug LOG
static int led_status = EZ_STATUS , led_enable = 1;


int get_fill_light_status(){
  return g_enviro_struct.f_light_st;

}
int set_fill_light_status(int status){

	g_enviro_struct.f_light_st = status;
}
int LedEnable(int flag)
{
	//if(g_enviro_struct.fill_light==1)
		//return ;
	static uint8_t before_disable;

	if(flag == 0)
	{
		before_disable = GetLedStatus();
		SetLed(1,LED_DISABLE);
	}
	
	led_enable = flag;
	if(flag && (!get_fill_light_status())/*(!g_enviro_struct.f_light_st)*/)
	{
		SetLed(1,led_status);
	}
//	pr_dbg("flag: %d\n",flag);
	return 0;
}


enum{
BLINK_MODE_ALWAYS = 0,
BLINK_MODE_FLICK = 1,
BLINK_MODE_CLOSE = 2,

};
int SetLed(int onoff,int status)
{
//	pr_dbg("var %d, %d\n", onoff, status);
	if (LED_DISABLE != status)
		led_status = status;
	if( led_enable == 0)
		return 0;

	switch(status)
	{
		case OFF_LINE_STATUS:						//lixian，蓝灯闪烁
			//debug("OFF_LINE_STATUS\n");
			break;
		case EZ_STATUS:								//等待连接，绿灯闪烁
			debug("EZ_STATUS\n");
			
			QCamLedSet(LED_MODE_GREEN,BLINK_MODE_FLICK);
			break;
		case EZ_STATUS_ROUTE:						//连接到路由，绿灯常亮
			debug("EZ_STATUS_ROUTE\n");
			
			QCamLedSet(LED_MODE_GREEN,BLINK_MODE_ALWAYS);
			break;
		case ON_LINE_STATUS:						//连接到网络，蓝灯常亮
			debug("ON_LINE_STATUS\n");;
			
			QCamLedSet(LED_MODE_BLUE,BLINK_MODE_ALWAYS);
			break;
		case VIDEO_START_STATUS:					//查看摄像机，蓝灯闪烁
			debug("VIDEO_START_STATUS\n");
			
			QCamLedSet(LED_MODE_BLUE,BLINK_MODE_FLICK);
			break;
		case VIDEO_STOP_STATUS:
			debug("VIDEO_STOP_STATUS\n");
			
			QCamLedSet(LED_MODE_BLUE,BLINK_MODE_ALWAYS);
			break;
		case BURN_FIRMWARE_STATUS:
			debug("VIDEO_STOP_STATUS\n");
			
			QCamLedSet(LED_MODE_RED,BLINK_MODE_ALWAYS);
			break;
		case LED_DISABLE:
			debug("LED_DISABLE\n");
			
			QCamLedSet(LED_MODE_OFF,BLINK_MODE_CLOSE);
			break;
	}
	//printf("Setled end+++++++++++++++\n");
	return 0;
}

int GetLedStatus()
{
	return led_status;
}



#if 1
#define _CMD_LEN    (256)

static void _close_all_fds (void)
{
    int i;
    for (i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
      if (i != STDIN_FILENO && i != STDOUT_FILENO && i != STDERR_FILENO)
        close(i);
    }
}
extern int __libc_fork (void);
static int _system (char *command)
{
    int pid = 0;
    int status = 0;
    char *argv[4];
    extern char **environ;

    if (NULL == command) {
        return -1;
    }

    pid = __libc_fork();        /* vfork() also works */
    if (pid < 0) {
        return -1;
    }
    if (0 == pid) {             /* child process */
        _close_all_fds();       /* 这是我自己写的一个函数，用来关闭所有继承的文件描述符。可不用 */
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = NULL;

        execve ("/bin/sh", argv, environ);    /* execve() also an implementation of exec() */
        _exit (127);
    }

	// else
 /* wait for child process to start */
 
    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR) {
            status = -1; /* error other than EINTR from waitpid() */
            break;
        }

    return (status);
}

int AMCSystemCmd (const char *format, ...)
{
    char cmdBuff[_CMD_LEN];
    va_list vaList;
	int i_ret = 0;
	int i_dbg = 1;
    va_start (vaList, format);
    vsnprintf ((char *)cmdBuff, sizeof(cmdBuff), format, vaList);
    va_end (vaList);
	
    i_ret = _system ((char *)cmdBuff);
    if ((i_ret) && (i_dbg == 1)){
        pr_error("err %s, cmd: %s\n",strerror(errno), cmdBuff);
		LOG("err %s, cmd: %s\n",strerror(errno), cmdBuff);
    	}
	
    return i_ret;
}
#endif

FILE* ysx_popen(const char *pCommand, const char *pMode, pid_t *pid)
{
    int i;
    int parent_end = 0;
    int child_end = 0;
    int child_std_end = 0;
    int pipe_fds[2] = {0};
    pid_t child_pid = 0;

    if(NULL == pCommand || NULL == pMode || pid == NULL)
    {
        return (NULL);
    }

    *pid = 0;

    if (pipe(pipe_fds) < 0)
    {
        return(NULL);
    }

    /* only allow "r" or "w" */
    if (pMode[0] == 'r' && pMode[1] == '\0')
    {
        parent_end = pipe_fds[0];
        child_end = pipe_fds[1];
        child_std_end = STDOUT_FILENO;
    }   
    else if (pMode[0] == 'w' && pMode[1] == '\0')
    {   
        parent_end = pipe_fds[1];
        child_end = pipe_fds[0];
        child_std_end = STDIN_FILENO;
    }   
    else
    {   
        return(NULL);
    }   
    
    child_pid = vfork();
    if (child_pid < 0)
    {   
        close(child_end);
        close (parent_end);
        return NULL;
    }   
    else if (child_pid == 0) /* child */
    {   
        close (parent_end);
        if (child_end != child_std_end)
        {   
            dup2 (child_end, child_std_end);
            close (child_end);
        }

        /* close all descriptors in child sysconf(_SC_OPEN_MAX) */
        for (i = 3; i < sysconf(_SC_OPEN_MAX); i++)
        {
            close(i);
        }

        execl("/bin/sh", "sh", "-c", pCommand, (char *) 0);

        _exit(127);
    }
    close(child_end);
	*pid = child_pid;

    return fdopen(parent_end, pMode);
}

int  ysx_pclose(FILE *fp, pid_t pid)
{
    int stat;

    if (fclose(fp) == EOF)
        return(-1);

    while (waitpid(pid, &stat, 0) < 0)
        if (errno != EINTR)
            return(-1); /* error other than EINTR from waitpid() */

    return(stat);   /* return child's termination status */
}

/*seconds: the seconds; mseconds: the micro seconds*/
void ysx_setTimer(int seconds, int mseconds)
{
    struct timeval temp;
 
    temp.tv_sec = seconds;
    temp.tv_usec = mseconds*1000;
 
    select(0, NULL, NULL, NULL, &temp);
    return ;
}
int set_flight_mode(int mode)
{

printf("flight mode %s\n",mode == 0 ? "close":"open");
QCamLedSet(LED_MODE_FLIGHT,mode);

}
int time_scope_set(int time_start_o,int time_start_m,int time_end_o,int time_end_m){
		 time_t timep;
         struct tm *sync_start_time;
           time(&timep);
           sync_start_time = localtime(&timep);
		   int nflag=-1;
        if(time_start_o > time_end_o){
                if((time_start_o == sync_start_time->tm_hour)&&(time_start_m < sync_start_time->tm_min)){

                        nflag=1;
                }else if((time_end_o == sync_start_time->tm_hour)&&(time_end_m > sync_start_time->tm_min)){

                        nflag=1;
                }else if((time_end_o > sync_start_time->tm_hour)||( sync_start_time->tm_hour > time_start_o)){

                        nflag=1;
                }else{

                        nflag=0;
                }
        }else if(time_start_o < time_end_o){
                if((time_start_o == sync_start_time->tm_hour)&&(time_start_m < sync_start_time->tm_min)){

                        nflag=1;
                }else if((time_end_o == sync_start_time->tm_hour)&&(time_end_m > sync_start_time->tm_min)){

                        nflag=1;
                }else if((time_start_o < sync_start_time->tm_hour)&&(sync_start_time->tm_hour < time_end_o)){

                        nflag=1;
                }else{

                        nflag=0;
                }

        }else if(time_start_o ==time_end_o){
                if((time_start_o == sync_start_time->tm_hour)&&(time_start_m <= sync_start_time->tm_min)&&(sync_start_time->tm_min <time_end_m)){

                        nflag=1;
                }
                else{

                        nflag=0;
                }
        }



		return nflag;

}
int rtsp_rev_cmd_parse(char*tmp,char*pdata,int*close_sock){

#ifdef YSX_RTSP_WEBSOCK

			if(!pdata)
				return -1 ;
			 memset(tmp,0,sizeof(tmp));
			 strcpy(tmp,pdata);
			 memset(g_enviro_struct.rtspAddr,0,sizeof(g_enviro_struct.rtspAddr));
			 sprintf(g_enviro_struct.rtspAddr,"rtsp:%s",tmp);
	
			 char*pdata2 = strstr(pdata+2,":");
	
			 if(pdata2 != NULL){
					 memset(tmp,0,sizeof(tmp));
					 strncpy(tmp,pdata+2,pdata2 - pdata - 2);
	
					 printf("[%s %d] %s\n",__FUNCTION__,__LINE__,tmp);
					 memset(g_enviro_struct.rtspServHost,0,sizeof(g_enviro_struct.rtspServHost));
					 sprintf(g_enviro_struct.rtspServHost,"%s",tmp);
	
					 char*pdata3 = strstr(pdata2+1,"/");
	
					 if(pdata3 != NULL){
							 memset(tmp,0,sizeof(tmp));
							 strncpy(tmp,pdata2+1,pdata3 - pdata2 -1);
							 printf("[%s %d] %s\n",__FUNCTION__,__LINE__,tmp);
							 memset(g_enviro_struct.rtspServPort,0,sizeof(g_enviro_struct.rtspServPort));
							 sprintf(g_enviro_struct.rtspServPort,"%s",tmp);
							 g_enviro_struct.rtsp_is_ok = 1;
							 *close_sock = 1;
							 
							 gettimeofday(&g_enviro_struct.rtsp_st_t, NULL);
					 }
	
			 }
#endif	
	
}
 struct json_object *set_fixed_attr(struct json_object *indata_object){
	#ifdef YSX_USER_REGISTRATION
    jsoon_add_str(indata_object, "model", g_enviro_struct.t_model);
    jsoon_add_str(indata_object, "vendor", g_enviro_struct.t_vendor);
    jsoon_add_str(indata_object, "device_type","shake");
	jsoon_add_str(indata_object, "ptz_type","horizontal+vertical"); 
	jsoon_add_str(indata_object, "language",g_enviro_struct.t_lan); 
	jsoon_add_str(indata_object, "wifi_ssid",g_enviro_struct.t_wifissid);
	jsoon_add_str(indata_object, "wifi_passwd",g_enviro_struct.t_wifipass);
    jsoon_add_int(indata_object, "fw_version",g_enviro_struct.t_version);
    jsoon_add_str(indata_object, "uid",g_enviro_struct.t_uid);
	#endif
	
	return indata_object;
	

}

struct json_object * set_realtime_attr(struct json_object *indata_object){
	#ifdef YSX_USER_REGISTRATION
	jsoon_add_int(indata_object,"camera_en",GetYSXCtl(YSX_C_EN));
	jsoon_add_int(indata_object,"mic_enable",GetYSXCtl(YSX_MIC_EN));
	jsoon_add_int(indata_object,"spk_vol",GetYSXCtl(YSX_VOL));
	jsoon_add_int(indata_object,"md_enable",-1);
	jsoon_add_int(indata_object,"md_sense",-1);
	jsoon_add_int(indata_object,"md_start",g_enviro_struct.md_tStart);
	jsoon_add_int(indata_object,"md_stop",g_enviro_struct.md_tEnd);
	jsoon_add_int(indata_object,"led_online_enable",-1);
	jsoon_add_int(indata_object,"track_enable",-1);
	jsoon_add_int(indata_object,"track_sense",-1);
	jsoon_add_int(indata_object,"pt_enable",GetYSXCtl(YSX_PTRACK_EN));
	jsoon_add_int(indata_object,"pt_times",GetYSXCtl(YSX_PTRACK_TT));
	jsoon_add_int(indata_object,"pt_start",GetYSXCtl(YSX_PTRACK_TS));
	jsoon_add_int(indata_object,"pt_end",GetYSXCtl(YSX_PTRACK_TE));
	jsoon_add_int(indata_object,"mosd_enable",GetYSXCtl(YSX_AI_MODE));
	jsoon_add_int(indata_object,"ir_mode",-1);
	jsoon_add_int(indata_object,"server_env",GetYSXCtl(YSX_CLOUD_ENV));
	jsoon_add_int(indata_object,"sd_resolution",GetYSXCtl(YSX_STORAGE_RESOLUTION));
	jsoon_add_int(indata_object,"auto_birate_enable",GetYSXCtl(YSX_AUTO_BITRATE));
	#endif

	return indata_object;

}
