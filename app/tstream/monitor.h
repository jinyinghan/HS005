/*************************************************************************
	> File Name: monitor.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Wed 04 Sep 2019 08:07:25 PM CST
 ************************************************************************/

#ifndef _MONITOR_H_
#define _MONITOR_H_





#define DEV_BOOT_FLAG	 			"/tmp/sys_start"
#define DEV_NONEED_BOOT_FLAG	 	"/etc/SNIP39/sys_start"



int create_ntp_thread(void);
int create_monitor(void);





#endif 
