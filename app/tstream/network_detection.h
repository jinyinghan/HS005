
#ifndef __NETWORK_DETECTION_H__
#define __NETWORK_DETECTION_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include <pthread.h>
#include <sys/prctl.h>


typedef enum network_enable{
    NETWORK_DISENABLE   = 0,	
	NETWORK_ENABLE      = 1,	
}NETWORK_ENABLE_E; 

typedef enum network_mode{
	INIT_MODE			= -1,
    WIRELESS_MODE      	= 0,          //无线模式
    WIRED_MODE       	= 1,          //有线模式
	AP_MODE            	= 2,          //AP模式
}NETWORK_MODE_E; 

typedef enum network_status{
   	NET_CONNECT_INIT   	= -1,    
    NET_GET_IPADDR	    = 0,
    NET_CONNECT_LAN	   	= 1,
    NET_CONNECT_WAN    	= 2,	
    NET_CONNECT_FAIL   	= 3,  
    NET_CONNECT_SWTICH	= 4,
}NETWORK_STATUS_E;  

typedef enum network_pthred{
    CHECK_MODE      	= 0,          //检测联网模式线程ID
    CHECK_STATUS       	= 1,          //检测联网状态线程ID
    CHECK_LAN			= 2,		  //检测局域网线程ID	
    CHECK_WAN			= 3,		  //检测广域网线程ID
	PTHREAD_ID_SUM      = 4,          
}NETWORK_PTHREAD_E; 

typedef struct network_info{	               
    NETWORK_MODE_E 	last_network_mode;         
    NETWORK_MODE_E 	cur_network_mode;    	
    NETWORK_STATUS_E  cur_network_status;
	NETWORK_STATUS_E  last_network_status;

	pthread_t pthread_id[PTHREAD_ID_SUM];
	pthread_mutex_t pthread_mutex;
	unsigned char network_running;
	unsigned char check_mode_running;
	unsigned char check_status_running;
	unsigned char check_lan_running;
	unsigned char check_wan_running;
	unsigned char is_bind;
}NETWORK_INFO_S;

extern NETWORK_INFO_S network_info;

typedef int (*network_cb)(NETWORK_MODE_E net_mode, NETWORK_STATUS_E net_status);
int get_network_mode(char *net_name, int *statue);
void network_set_bind(unsigned char is_bind);
void network_restart_wlan0();
int network_check_init(network_cb net_cb, unsigned char is_bind);
void network_check_uninit();
void network_switch_wifi();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

