/*************************************************************************
	> File Name: utils.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月22日 星期六 14时05分13秒
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"



int open_file(const char * name)
{       
	int ret = -1;    
	int fd;    
	fd = open(name, O_RDWR);    
	if (fd < 0) {
	    ret = errno > 0 ? errno : -1;
		printf("open %s failed with: %s!\n", name, strerror(ret));        
		fd = ret;    
    }            
	return fd;
} 

int close_file(int handle)
{   
	int ret = -1;    
	ret = close(handle);    
	if (ret < 0) { 
	    ret = errno > 0 ? errno : -1;
		printf("close failed with: %s!\n", strerror(ret));        
		return -1;    
	}    
	return ret;
}

int file_esixt(const char * name)
{



}


