
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

#include "network_detection.h"
#include "include/common_func.h"

NETWORK_INFO_S network_info;
network_cb cb = NULL;

void network_set_timer(int seconds, int mseconds)
{
    struct timeval temp;
    temp.tv_sec = seconds;
    temp.tv_usec = mseconds;
    select(0, NULL, NULL, NULL, &temp);
	
    return ;
}

void network_set_status(NETWORK_STATUS_E network_status)
{
	network_info.cur_network_status = network_status;
	if(network_info.last_network_status != network_info.cur_network_status){
		network_info.last_network_status = network_info.cur_network_status;
		cb(network_info.cur_network_mode, network_info.cur_network_status);
	}
}

void network_enable_porc(NETWORK_PTHREAD_E network_proc, NETWORK_ENABLE_E network_enable)
{
	switch(network_proc){
		case CHECK_MODE:
			network_info.check_mode_running = network_enable;
			break;
		case CHECK_STATUS:
			network_info.check_status_running = network_enable;
			break;
		case CHECK_WAN:
			network_info.check_wan_running = network_enable;
			break;
		case CHECK_LAN:
			network_info.check_lan_running = network_enable;
			break;
		default:
			break;
	}
}

int network_popen_tag(const char *cmd, const char *tag)
{
    FILE *fp = NULL;
    char buf[1024]= {0};
    if((fp = popen(cmd, "r")) == NULL){
        return -1;
    }
    while(fgets(buf, sizeof(buf), fp) != NULL){
		if(strstr(buf, tag)){ 
		    pclose(fp);
            fp = NULL;
			return 1;
		}
    }
    pclose(fp);
    fp = NULL;
    return 0;
}

int get_network_mode(char *net_name, int *statue)
{
    int ret = 0;
    int skfd = 0;
    struct ifreq ifr;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);	//建议socket通信，AF_INET协议族，SOCK_DGRAM表示UDP协议，0表示使用默认套接字，即这里的UDP
    if(skfd < 0){
        printf("open socket error\n");
        return skfd;
    }
    strcpy(ifr.ifr_name, net_name);	//要检测网卡的名称
    if((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)) < 0){	//调用内核接口，获取网卡信息
        printf("ethernet interface is not valid\n");
        close(skfd);
        return ret;
    }
    close(skfd);
    if(ifr.ifr_flags & IFF_RUNNING){	//判断网卡状态
        *statue = 1;
    }else{
        *statue = 0;
    } 
    return ret;
}

void network_wlan0_start()
{
	
    	AMCSystemCmd("ifconfig wlan0 up");
		network_set_timer(0, 50);
    	AMCSystemCmd("ifconfig wlan0 0.0.0.0");
		network_set_timer(0, 50);
    	AMCSystemCmd("wpa_supplicant -B -i wlan0  -c /etc/SNIP39/wpa_supplicant.conf");
		network_set_timer(0, 50);
    	AMCSystemCmd("udhcpc -i wlan0&");
		network_set_timer(0, 50);
	

	return ;
}

void network_wlan0_stop()
{
	AMCSystemCmd("killall -q hostapd");
	network_set_timer(0, 50);
	AMCSystemCmd("killall -q udhcpd");
	network_set_timer(0, 50);
	
#if 1
    	AMCSystemCmd("killall -9 wpa_supplicant");
		network_set_timer(0, 50);
    	AMCSystemCmd("killall -9 udhcpc");
		network_set_timer(0, 50);

	
	AMCSystemCmd("ifconfig wlan0 0.0.0.0");
	network_set_timer(0, 50);
    AMCSystemCmd("ifconfig wlan0 down");
	network_set_timer(0, 50);
#endif	

	return ;
}

void network_eth0_start()
{ 	
	AMCSystemCmd("ifconfig eth0 up");
	network_set_timer(0, 50);
    AMCSystemCmd("udhcpc -i eth0&");
	network_set_timer(0, 50);

	return ;
}

void network_eth0_stop()
{
    AMCSystemCmd("ifconfig eth0 0.0.0.0");
	network_set_timer(0, 20);
    AMCSystemCmd("ifconfig eth0 down");
	network_set_timer(0, 20);
    AMCSystemCmd("killall -9 udhcpc");
	network_set_timer(0, 50);

	return ;
}

int network_connect_check(char *ip_addr)
{
	char cmd[128] = {'\0'};
	if(strlen(ip_addr)){
		sprintf(cmd,"ping -q -c 1 -w 2 %s", ip_addr);
		return network_popen_tag(cmd, "0 packets received");
	}
	else
		return -1;
}

void *network_lan_proc(void *arg)
{
	char gw_ip[16];
    char cmd[128];
    int ret = 0, connt = 0;
	prctl(PR_SET_NAME, __FUNCTION__);

	while(network_info.network_running){
		if(network_info.check_lan_running){
			network_enable_porc(CHECK_LAN, NETWORK_DISENABLE);
		
			FILE *gw_fp = fopen("/system_rw/router_ip","r");
    		if(gw_fp != NULL){
        		memset(gw_ip, '\0', sizeof(gw_ip));
				fgets(gw_ip, sizeof(gw_ip), gw_fp);
				gw_ip[strlen(gw_ip)-1] = '\0';
        		printf("@@@ gw_ip:%s len:%d @@@\n", gw_ip, strlen(gw_ip));
        		fclose(gw_fp);
    		}

			ret = network_connect_check(gw_ip);  
        	if(ret == 0){
				connt = 0;
				network_set_status(NET_CONNECT_LAN);
        	}
        	else {
				printf("@@@ wait add route ...time=%d @@@\n",connt);
				if(connt >= 6){	//6*5=30s
					connt = 0;
					if(network_info.cur_network_mode == WIRED_MODE)
						network_eth0_stop();
					else
						network_wlan0_stop();

					network_set_timer(3, 0);	
					network_set_status(NET_CONNECT_INIT);
					network_enable_porc(CHECK_LAN, NETWORK_DISENABLE);
					network_enable_porc(CHECK_WAN, NETWORK_DISENABLE);
					network_enable_porc(CHECK_STATUS, NETWORK_ENABLE);
				}
				else{	
					if(network_info.cur_network_mode == WIRED_MODE){
						AMCSystemCmd("ifconfig eth0 down");
						AMCSystemCmd("ifconfig eth0 up");
					}
					else{
						AMCSystemCmd("ifconfig wlan0 down");
						AMCSystemCmd("ifconfig wlan0 up");
					}	
				
					network_set_status(NET_CONNECT_FAIL);
					connt ++;
					memset(cmd, 0, sizeof(cmd));
            		sprintf(cmd, "route add default gw %s", gw_ip);
            		AMCSystemCmd(cmd);
				}
        	}
		}

		network_set_timer(5, 0);
	}

	pthread_exit((void*)0);
}

void *network_wan_proc(void *arg)
{
	int ret = 0;
	prctl(PR_SET_NAME, __FUNCTION__);
	unsigned char cnt = 0;

	while(network_info.network_running){
		if(network_info.check_wan_running){
			if(cnt == 0){
				ret = network_connect_check("8.8.8.8");  
        		if(ret == 0)
					network_set_status(NET_CONNECT_WAN);
				else
					network_enable_porc(CHECK_LAN, NETWORK_ENABLE);
			}
			network_set_timer(1, 0);
			cnt++;
			if(cnt >= 120)
				cnt = 0;
		}
		network_set_timer(1, 0);
	}

	pthread_exit((void*)0);
}

void *network_status_proc(void *arg)
{
	char cmd[64];
    int ret = 0;
	unsigned char cnt1 = 0, cnt2 = 0, cnt3 = 0;
	prctl(PR_SET_NAME, __FUNCTION__);

	while(network_info.network_running){
		if(network_info.check_status_running){
			if(cnt1 == 0){
				memset(cmd, 0, sizeof(cmd));
        		if(network_info.cur_network_mode == WIRED_MODE)
            		sprintf(cmd, "%s", "eth0");
        		else
            		sprintf(cmd, "%s", "wlan0");
		
        		if(network_popen_tag("ifconfig", cmd)){			
        			memset(cmd, 0, sizeof(cmd));
        			if(network_info.cur_network_mode == WIRED_MODE)
            			sprintf(cmd, "ifconfig %s", "eth0");
        			else
            			sprintf(cmd, "ifconfig %s", "wlan0");
		
        			if(network_popen_tag(cmd, "inet addr:")){
						cnt2 = 0;
						network_enable_porc(CHECK_STATUS, NETWORK_DISENABLE);
						network_set_status(NET_GET_IPADDR);
						network_enable_porc(CHECK_WAN, NETWORK_ENABLE);
        			}
        			else{
						printf("@@@ wait get ipaddr ...time=%d @@@\n",cnt2);
            			if(cnt2 >= 9){	//9*10=90s
							cnt2 = 0;
							network_set_status(NET_CONNECT_INIT);
            			}
						else{
							if(cnt3 == 0){
								if(network_info.cur_network_mode == WIRED_MODE){
									network_eth0_stop();
									network_eth0_start();
								}
        						else{
									network_wlan0_stop();
									network_wlan0_start();	
								}
							}
						
							network_set_timer(1, 0);
							cnt3 ++;
							if(cnt3 > 60){
								cnt3 = 0;
								network_set_status(NET_CONNECT_FAIL);
							}
            				cnt2 ++;
						}
        			}
				}
				else{
					if(network_info.cur_network_mode == WIRED_MODE){
						network_eth0_stop();
						network_eth0_start();
					}
        			else{
						network_wlan0_stop();
						network_wlan0_start();	
					}
				}
			}
			network_set_timer(1, 0);
			cnt1 ++;
			if(cnt1 > 10)
				cnt1 = 0;
		}
		
		network_set_timer(1, 0);
	}
	
	pthread_exit((void*)0);
}

void *network_mode_proc(void *arg)
{
	int ret = 0, net_status = 0;
	prctl(PR_SET_NAME, __FUNCTION__);
    AMCSystemCmd("ifconfig eth0 up");
	network_set_timer(2, 0);

	unsigned char cnt = 0;
	network_info.last_network_mode = -1;
	network_info.check_mode_running = 1;

	while(network_info.network_running){
		if(network_info.check_mode_running){
			if(cnt == 0){
				AMCSystemCmd("ifconfig eth0 up");
        		get_network_mode("eth0", &net_status);
				if(net_status)
					network_info.cur_network_mode = WIRED_MODE;
				else
					network_info.cur_network_mode = WIRELESS_MODE;
		
        		if(network_info.cur_network_mode != network_info.last_network_mode){
					network_set_status(NET_CONNECT_SWTICH);
					network_enable_porc(CHECK_STATUS, NETWORK_DISENABLE);
					network_enable_porc(CHECK_LAN, NETWORK_DISENABLE);
					network_enable_porc(CHECK_WAN, NETWORK_DISENABLE);
			
					network_info.last_network_mode = network_info.cur_network_mode;
            		if(network_info.cur_network_mode == WIRED_MODE){
						printf("@@@ Switch WIRED_MODE @@@\n");	
						network_wlan0_stop();
    					network_eth0_start();
            		}
            		else{
						printf("@@@ Switch WIRELESS_MODE @@@\n");
						network_eth0_stop();
    					network_wlan0_start();
            		}
					network_set_status(NET_CONNECT_INIT);
					if(network_info.is_bind){
						network_enable_porc(CHECK_STATUS, NETWORK_ENABLE);
					}
				}
			}
			network_set_timer(1, 0);
			cnt++;
			if(cnt >= 5)
				cnt = 0;
		}
        network_set_timer(1, 0);
	}
	
	pthread_exit((void*)0);
}

void network_set_bind(unsigned char is_bind)
{
	network_info.is_bind = is_bind;
}

void network_restart_wlan0()
{
	network_wlan0_stop();
	network_wlan0_start();
}

void network_switch_wifi()
{
	printf("start network_switch_wifi\n");
	#if 0
	if(network_info.check_lan_running)
		network_enable_porc(CHECK_LAN, NETWORK_DISENABLE);

	if(network_info.check_wan_running)
		network_enable_porc(CHECK_WAN, NETWORK_DISENABLE);

	if(network_info.check_status_running)
		network_enable_porc(CHECK_STATUS, NETWORK_DISENABLE);

	if(network_info.check_mode_running)
		network_enable_porc(CHECK_MODE, NETWORK_DISENABLE);
	#endif
	network_wlan0_stop();
	network_set_timer(5, 0);
	network_info.last_network_mode = -1;
	network_enable_porc(CHECK_MODE, NETWORK_ENABLE);
	
}


int network_check_init(network_cb net_cb, unsigned char is_bind)
{
	int ret;
	memset(&network_info, 0, sizeof(NETWORK_INFO_S));
	pthread_mutex_init(&network_info.pthread_mutex, NULL);

	// AMCSystemCmd("modprobe 8188fu");
	network_eth0_stop();
	network_wlan0_stop();
	network_set_timer(2, 0);
	network_info.network_running = NETWORK_ENABLE;
	cb = net_cb;
	network_info.is_bind = is_bind;

	ret = pthread_create(&network_info.pthread_id[CHECK_MODE], NULL, network_mode_proc, NULL);
	if(ret != 0){
		printf("create network_mode_proc error\n");
		return -1;
	}

	ret = pthread_create(&network_info.pthread_id[CHECK_STATUS], NULL, network_status_proc, NULL);
	if(ret != 0){
		printf("create network_status_proc error\n");
		return -1;
	}

	ret = pthread_create(&network_info.pthread_id[CHECK_WAN], NULL, network_wan_proc, NULL);
	if(ret != 0){
		printf("create network_wan_proc error\n");
		return -1;
	}

	ret = pthread_create(&network_info.pthread_id[CHECK_LAN], NULL, network_lan_proc, NULL);
	if(ret != 0){
		printf("create network_lan_proc error\n");
		return -1;
	}

	return 0;
}

void network_check_uninit()
{
	network_info.network_running = NETWORK_DISENABLE;

	if(network_info.check_lan_running){
		network_enable_porc(CHECK_LAN, NETWORK_DISENABLE);
		pthread_join(network_info.pthread_id[CHECK_LAN], NULL);
	}
	printf("Release CHECK_LAN\n");

	if(network_info.check_wan_running){
		network_enable_porc(CHECK_WAN, NETWORK_DISENABLE);
		pthread_join(network_info.pthread_id[CHECK_WAN], NULL);
	}
	printf("Release CHECK_WAN\n");

	if(network_info.check_status_running){
		network_enable_porc(CHECK_STATUS, NETWORK_DISENABLE);
		pthread_join(network_info.pthread_id[CHECK_STATUS], NULL);
	}
	printf("Release CHECK_STATUS\n");
	
	if(network_info.check_mode_running){
		network_enable_porc(CHECK_MODE, NETWORK_DISENABLE);
		pthread_join(network_info.pthread_id[CHECK_MODE], NULL);
	}
	printf("Release CHECK_MODE\n");

	network_wlan0_stop();
	//network_eth0_stop();

	pthread_mutex_destroy(&network_info.pthread_mutex);
	memset(&network_info, 0, sizeof(NETWORK_INFO_S));

	printf("network_check_uninit\n");
	return ;
}



