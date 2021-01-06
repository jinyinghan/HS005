/*************************************************************************
	> File Name: wdt.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Sat 24 Aug 2019 08:18:40 PM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>

static int g_wdt_fd = -1;

int wdt_keep_alive(void)
{
	int ret = -1;
    int dummy;
	printf("Feed watchdog\n");
    ret = ioctl(g_wdt_fd, WDIOC_KEEPALIVE, &dummy);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}



int wdt_set_timeout(int to)
{
	int ret = -1;
    int timeout = to;
    ret = ioctl(g_wdt_fd, WDIOC_SETTIMEOUT, &timeout);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;

}

int wdt_get_timeout(void)
{
	int ret = -1;
    int timeout = 0;
    ret = ioctl(g_wdt_fd, WDIOC_GETTIMEOUT, &timeout);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return timeout;

}


int wdt_disable(void)
{
	int ret = -1;
	int flags = 0;
	flags = WDIOS_DISABLECARD;
	ret = ioctl(g_wdt_fd, WDIOC_SETOPTIONS, &flags);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}

	if (g_wdt_fd >= 0) {
		close(g_wdt_fd);
		g_wdt_fd = -1;
	}
	return 0;

}

int wdt_enable(void)
{
	int ret = -1;
	int flags = 0;
	
	if (g_wdt_fd < 0) {
		g_wdt_fd = open("/dev/watchdog", O_WRONLY);
		if (-1 == g_wdt_fd) {
			printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
			exit(-1);
		}	
	}
	
	flags = WDIOS_ENABLECARD;
	ret = ioctl(g_wdt_fd, WDIOC_SETOPTIONS, &flags);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;

}
int QCamWatchdogEnable(int enable)
{
    if(enable == 1){
        wdt_enable();
    }else if(enable == 0){
        wdt_disable();
    }else{
        printf("not supported mode\n");
    }

    return 0;


}
int QCamWatchdogKeepAlive(){
    
    wdt_keep_alive();

    return 0;
}
int QCamWatchdogSetTimeout(int secs){
    if(secs <= 0 ){
        printf("Time of failure\n");
        return -1;
    }
    wdt_set_timeout(secs);
    return 0;


}
