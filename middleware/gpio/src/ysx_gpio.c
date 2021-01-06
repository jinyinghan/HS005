/*************************************************************************
	> File Name: ysx_gpio.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月22日 星期六 13时26分09秒
 ************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

#include "ysx_gpio.h"
#include "utils.h"



static inline int ysx_gpio_exist(int pin)
{
    char buf[BUF_SIZE] = {0};
    snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d", pin);
    if (access(buf,F_OK) == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

inline int ysx_gpio_open(int pin)
{
    if(pin < 0 || pin > 96){
        printf("illegal pin number!\n");
        return -1;
    }
    
    char buf[BUF_SIZE] = {0};
    if (ysx_gpio_exist(pin)) {
        return 0;
    }

    snprintf(buf, BUF_SIZE, "echo %d > /sys/class/gpio/export", pin);
    int ret = system(buf);
	if (ret < 0) {
	    ret = errno > 0 ? errno : -1;
		printf("system failed with: %s!\n", strerror(errno));
		return -1;
    }
	return 0;
}

inline int ysx_gpio_read(int pin, int *val)
{
    int fd;
    char value_str[5];
    char buf[BUF_SIZE] = {0};
    
    snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        printf("set gpio %d direction error !\n",pin);
        return -1;        
    }
    write(fd, "in", sizeof("in"));
    close(fd);

    CLEAR(buf);
    snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/value",pin);
    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        printf("set gpio %d direction error !\n",pin);
        return -1;        
    }
    
    CLEAR(value_str);
    read(fd, value_str, 5);   
    close(fd);
    if (NULL != val) {
        *val = atoi(value_str);
    }
    return 0;
}

inline int ysx_gpio_write(int pin , int value)
{
    int ret = 0, fd = -1;
    char buf[128] = {0};
    snprintf(buf, BUF_SIZE, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(buf, O_RDWR);
    if (fd < 0) {
        ret = errno > 0 ? errno : -1;
        printf("open file failed with: %s!\n", strerror(ret));
        return -1;        
    }
	read(fd, buf, 5); 
    write(fd, "out", sizeof("out"));
	close(fd);

    CLEAR(buf);
    snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/value",pin);
    fd = open(buf,O_WRONLY);
    if (fd < 0) {
        printf("set gpio %d direction error [%d]!\n",pin,__LINE__);
        return -1;        
    }
    if (value == 0) {
        write(fd,"0",sizeof("0"));
    }
    else {
        write(fd,"1",sizeof("1"));
    }    
    close(fd);
    return 0;
}


inline int ysx_gpio_close(int pin)
{
    if(pin < 0 || pin > 96){
        printf("illegal pin number!\n");
        return -1;
    }
    char buf[BUF_SIZE] = {0};
    if (!ysx_gpio_exist(pin)) { 
        return 0;
    }
    
    CLEAR(buf);
    snprintf(buf,BUF_SIZE,"echo %d > /sys/class/gpio/unexport",pin);
    system(buf);

    return 0;
}


