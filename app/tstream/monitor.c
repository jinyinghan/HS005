/*************************************************************************
	> File Name: monitor.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Wed 04 Sep 2019 08:07:19 PM CST
 ************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/reboot.h>
#include <sys/prctl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>																													 
#include <netinet/in.h>

#include "log.h"
#include "wdt.h"
#include "monitor.h"

#include "common_env.h"
#include "common_func.h"

static int g_wdt_enalbe = 0;

extern G_StructEnviroment g_enviro_struct;

void * pthread_lightsocket(void *arg)
{
	int ret = 0;
	unsigned char chDeviceMode;
	prctl(PR_SET_NAME, "AP-SOCK");
	while (1)
	{
		sleep(3);
		chDeviceMode = GetDeviceMode();
		// ?豸δ??????
		if (chDeviceMode == 0) {
//			printf("socket init\n");
			int welcomeSocket, newSocket,fd;
			struct sockaddr_in serverAddr;
			struct sockaddr_storage serverStorage;
			struct timeval tv;
			tv.tv_sec = 3;
			tv.tv_usec = 0;
			socklen_t addr_size;
			fd_set  readfds,tmpfds;


			welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(24385);
			serverAddr.sin_addr.s_addr = inet_addr("192.168.1.1");
			memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
			int n = 1;
			setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
			if (bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))<0) {	
				sleep(3);
//				printf("bind Error\n");
				shutdown(welcomeSocket,2);
				close(welcomeSocket);
				continue;   
			}

			if (listen(welcomeSocket,5)<0) {
				printf("listen Error\n");
				sleep(3);
				shutdown(welcomeSocket,2);
				close(welcomeSocket);
				continue;
			}

			FD_ZERO(&readfds);
			FD_SET(welcomeSocket,&readfds);
			
			while (1)
			{
				tmpfds=readfds;
				// ??ȡ??ǰ?豸????״̬,?????Ѿ???????ֱ???˳?
				chDeviceMode = GetDeviceMode();
				if (chDeviceMode ==1) {
					printf("stop socket\n");
					break;
				}
				
				if (select(FD_SETSIZE, &tmpfds, NULL, NULL, &tv) < 1) {
					continue;
				}
				
				printf("select ok\n");
				for (fd = 0; fd < FD_SETSIZE; fd++) {
					if (FD_ISSET(fd, &tmpfds)) {
						printf("FD_ISSET\n");
						if (fd == welcomeSocket) {
							printf("fd1==welcomeSocket\n");
							addr_size = sizeof(serverStorage);
							newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
							printf("%d\n",newSocket);
							if (newSocket<0) {
								continue;
                            }
							else {
								printf("connect ok \n");
								system("touch /tmp/have_connect");
								break;
							}
						}
					}
				}
				usleep(100 * 1000);
			}
			
			printf("close\n");
			shutdown(newSocket, 2);
			close(newSocket);
			shutdown(welcomeSocket, 2);
			close(welcomeSocket);
		}
	}
	return (void* )1;
}


void set_device_mode(void)
{
	int ret = 0;
	char cmd[256];
	int f_exist_flag = 0;
	sprintf(cmd, "touch %s", DEV_BOOT_FLAG);


	pthread_t lightsocket_tid;
	ret = pthread_create(&lightsocket_tid, NULL, &pthread_lightsocket, NULL);
	if (ret < 0) {
		printf("motion_tracker_thread Thread create fail, ret=[%d]\n", ret);
		return ;
	}
	pthread_detach(lightsocket_tid);



	if (access(DEV_BOOT_FLAG, F_OK) == 0) {
		f_exist_flag = 1;
		LOG("boot flag alread exist! \n");
		return;
	}
	// ??Ч??־,δʹ??
	if (access(DEV_NONEED_BOOT_FLAG, F_OK) == 0) {
		remove(DEV_NONEED_BOOT_FLAG);
		LOG("DEV_NONEED_BOOT_FLAG exist! \n");
		return;
	}
	
	if (f_exist_flag == 0) {
		system(cmd);
	}
}


void * thread_monitor(void *arg)
{
	LOG("thread enter\n");
	set_device_mode();

	unsigned int ticket = 0;

#ifdef USE_WDT
	LOG("WDT Enable\n");
	wdt_enable();
	wdt_set_timeout(120);
	g_wdt_enalbe = 1;
#endif 

	prctl(PR_SET_NAME, "MONITOR");
	while(1)
	{
		if (QCamGetKeyStatus()) {
			set_device_mode();
			LOG("## detect reset \n");		
			system("sample_play 16000 /usr/notify/reset.pcm force");
			system("cp -rfv /usr/system/etc_rw/SNIP39/* /system_rw/SNIP39/");
			system("cp -rfv /usr/system/etc_rw/TZ /system_rw/");
			sync();
			reboot(RB_AUTOBOOT); 
		}

#ifdef USE_WDT
		if (ticket % 30 == 0) {
			if (g_wdt_enalbe) {
				wdt_keep_alive();
			}
		}

		if (0 == access("/tmp/stop_wdt", F_OK)) {
			if (g_wdt_enalbe) {
				wdt_disable();
				g_wdt_enalbe = 0;
				LOG("XXXXXXXXXXXXXXXXXXXXXXX stop wdt\n");
				remove("tmp/stop_wdt");
			}
		}
#endif 

		sleep(1);
		ticket ++;
	}
	return 0;
}


void * thread_ntp(void *arg)
{
    int ret = 0;
	int sleep_cnt = 0;
	remove("/tmp/sync_time");

	prctl(PR_SET_NAME, "NTP");
	
	while (1)
	{
		if (!g_enviro_struct.t_online) {
			sleep(1);
			continue;
		}

		//while(!g_enviro_struct.dev_mode)
		while(!g_enviro_struct.is_bind)// 设备已经绑定 需要确认下 TZ同步问题
		{
			sleep(60);
		}

		system("touch /tmp/sync_time");

		LOG("XXXXXXXXXXXXXXXXXXXXXXXXXXXX start ntpdate\n");
		
		if ( access("/tmp/sync_time", F_OK) != 0) {
			sleep(30);
			continue;
		}

        ret = access("/tmp/sync_time", X_OK);//文件有执行权限 则不进行同步
		if (ret != 0) {
			LOG("XXXXXXXXXXXXXXXXXXXXXX start to use ntpdate sync time\n");
			if (WEXITSTATUS(system("ntpdate -u 0.pool.ntp.org"))!=0) {
				if (WEXITSTATUS(system("ntpdate -u 1.pool.ntp.org"))!=0) {
					if (WEXITSTATUS(system("ntpdate -u 0.debian.pool.ntp.org"))!=0) {
						if (WEXITSTATUS(system("ntpdate -u asia.pool.ntp.org"))!=0) {
							sleep(10);
							continue;
						}
                    }
			    }
			}
        }
        
		system("chmod +x /tmp/sync_time");
		LOG("## ntp ok\n");
		sleep_cnt = 0;
		while (1)
		{
			if (access("/tmp/sync_time", F_OK) == 0 && (sleep_cnt ++) != (360 * 24)) {
				sleep(10);
			}
			else {
				break;
		    }
		}
		
		if (access("/tmp/sync_time", F_OK) == 0) {
			system("chmod -x /tmp/sync_time");
		}
	}
	return 0;
}

int create_monitor(void)
{
	int ret = 0;
	pthread_t thread_id;
	ret = pthread_create(&thread_id, NULL, thread_monitor, NULL);
	if (ret < 0) {
		EMGCY_LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! restart system\n");
		return -1;
	}

	return 0;
}

int create_ntp_thread(void)
{	
	int ret = 0;
	pthread_t thread_id;
	
	ret = pthread_create(&thread_id, NULL, thread_ntp, NULL);
	if (ret < 0) {
		EMGCY_LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! restart system\n");
		return -1;
	}
	
	return 0;
}

