#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "log.h"
#include "common_env.h"
#include "common_func.h"
#include "media.h"
#include "vava_cloud.h"
#include "monitor.h"
#include "ota.h"

#define DEBUG_MODE  //for debug  打开这个宏  进入debug mode

static pthread_t master_tid = -1;
static int ysx_sys_flag = true;
int main_exit = 0;

/****func declare*****/

void sighandle(int signo)
{
    printf("Signal to close tstreamer!\n");

	ysx_h264_udp_socket_uinit();
		
    LOG("Signal to close tstreamer!\n");
    EMGCY_LOG("@@@@ Start Release Network @@@@\n");
#ifndef DEBUG_MODE
    network_check_uninit();
#endif	
    EMGCY_LOG("@@@@ Start Release Media @@@@\n");
    if((g_enviro_struct.is_bind)||(file_exist("/etc/SNIP39/ap"))){
        Media_Close();
        EMGCY_LOG("@@@@ Start Release AVInfo @@@@\n");
        DeInitAVInfo();
    }else{
        EMGCY_LOG("@@@@ Start Release Bind @@@@\n");
          
    }
    
    EMGCY_LOG("@@@@ Start avDeInitialize @@@@\n");
    
    EMGCY_LOG("@@@@ Start uninit_ppcs_service @@@@\n");
    uninit_ppcs_service();
	
    EMGCY_LOG("@@@@ UnInitialize End @@@@\n");

    uninit_ysx_curl_ssl();

	main_exit = 1;
    _exit(0);
}


void PRINTF_HELLO_BOX(void)
{
    EMGCY_LOG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    EMGCY_LOG("@@@@      YSX_TSTREAMER_START     @@@@\n"); 
    EMGCY_LOG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	return ;
}


//结束进程 信号函数
void ysx_signal_init(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGINT);

    sa.sa_handler = sighandle;
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = sighandle;
    sigaction(SIGINT, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);
}
#define URL_TEST  "https://cdn.sunvalleycloud.com/test/3d7a315374b23e7633363ac6745a20eb/19e4ae854d4c28f3e20bdfffab2485cb.bin"

int main(int argc, char *argv[])
{

    int ret;
    int  SID;
    pthread_t ThreadLogin_ID;

	PRINTF_HELLO_BOX();
	
	// 初始化 日志模块
    if (argc >1 && strlen(argv[1])) {
        LOG_INIT(argv[1]);
    }

    ysx_signal_init();

	create_monitor();

	DeInitAVInfo();
	g_enviro_struct._loop = 1;
	
	InitDeviceCommonConfig();

	
	if( GetEnviroStruct() != 0)
    //	return -1;//no uid then return

	Initialization_Pthread_Led_Ctrl(NULL);

	g_enviro_struct.is_bind = GetDeviceMode();
	
	LOG("### IPC is_bind=%d!\n", g_enviro_struct.is_bind);
	//network_check_init(dev_network_cb, g_enviro_struct.is_bind);

	//ysx_h264_udp_socket_init(6666); // just for h264 streamer test UDP SOCKET PORT : 6666

	if( init_media() != 0 )
		return -1;
	
#ifndef DEBUG_MODE
	// 启动配网线程 因为需要音视频的接入  init meida 需要在这个线程之前 启动
	if( init_ez_thread() != 0)
		return -1;
#endif

	init_ysx_curl_ssl();

	create_ntp_thread();
	
	init_smart_ai();   //人形AI
	init_motion_detect();// 移动侦测
	

	init_vava_cloud();

	//初始化尚云 P2P 服务
	init_ppcs_service();  // uninit is  uninit_ppcs_service

	init_ota_thread();
	
	//loop forever !!!
	while(!main_exit)
	{
		sleep(1);
	}

	Media_Close();
	uninit_ppcs_service();
	uninit_ota_thread();
	
    return 0;
}


