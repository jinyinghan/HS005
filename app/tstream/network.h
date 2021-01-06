/*************************************************************************
	> File Name: network.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年08月30日 星期四 02时45分27秒
 ************************************************************************/

#ifndef _NET_WORK_H_
#define _NET_WORK_H_

//#define EMERGENCY_DEBUG
#if defined  EMERGENCY_DEBUG
#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"

#define EMGCY_LOG(fmt...)   \
	        do {\
				printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0)  
#else
#define EMGCY_LOG(...) 
#endif 


#define CHAR_SPACE	' '
#define is_a_space(c)    (c  ==  CHAR_SPACE )  
#define CMD_START_WLAN_WPA      "wpa_supplicant -Dnl80211 -iwlan0 -c \
    /etc/SNIP39/wpa_supplicant.conf -B"

#define CMD_START_ETH_UDHCPC    "udhcpc -i eth0 -b -R"


#define CMD_START_WLAN_UDHCPC   "udhcpc -i wlan0 -b -R"
#define CMD_STOP_WLAN_WPA       "killall -q wpa_supplicant"
#define CMD_STOP_UDHCPC         "killall -q udhcpc"
#define CMD_STOP_HOSTAPD        "killall -q hostapd"
#define CMD_STOP_UDHCPD         "killall -q udhcpd"
#define CMD_WLAN_DONE           "ifconfig wlan0 down"
#define CMD_WLAN_UP             "ifconfig wlan0 up"
#define CMD_STOP_ETH            "ifconfig eth0 down"
//#define CMD_START_ETH           "ifcofnig eth0 up"
#define CMD_START_ETH           "ifconfig eth0 up"

#define ETH_LINK_CHECK_TIME     60      // sec
#define WLAN_LINK_CHECK_TIME    60      // sec

#define ETH_NET_CHECK_TIME      5       // min
#define WLAN_NET_CHECK_TIME     5       // min
#define NET_ADAPTDER_INTER_TIME 1       // sec


int network_wan_available(const char * getwayIP, int * success);
int check_net_cable(const char * hwname);




#endif 

