    /*************************************************************************
	> File Name: uln2803_test.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Thu 09 Aug 2018 04:52:05 PM CST
 ************************************************************************/

#include <sys/types.h>      
#include <sys/stat.h>       
#include <fcntl.h>          
#include <errno.h>          
#include <unistd.h>         
#include <stdlib.h>         
#include <sys/ioctl.h>      
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>   
#include <stdlib.h>
#include <errno.h>

#include "uln2803.h"

static int g_uln_fd  = -1;

int open_file(const char * name)
{       
	int ret = -1;    
	int fd;    
	fd = open(name, O_RDWR | O_CLOEXEC);    
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

int uln2803_open(void)
{
	g_uln_fd = open_file(DEV_NAME);
	if (g_uln_fd < 0) {
		return -1;
	}
	return 0;
}

int uln2803_close(void)
{
	if (g_uln_fd > 0) {
		close_file(g_uln_fd);
	}
	return 0;
}


int uln2803_set_move(int which, int direction, int steps)
{	
	int ret = 0;
	struct motor_status_st status = {0};
	
	struct motor_move_st move;

	if (MOTOR_POS_Y == which) {
		(direction == 1) ? (move.motor_directional = MOTOR_DIRECTIONAL_UP) : (move.motor_directional = MOTOR_DIRECTIONAL_DOWN);
	}
	else if (MOTOR_POS_X == which) {
		(direction == 1) ? (move.motor_directional = MOTOR_DIRECTIONAL_RIGHT) : (move.motor_directional = MOTOR_DIRECTIONAL_LEFT);	
	}

//	move.motor_directional = direction;
	move.motor_move_steps = steps;
//	move.motor_move_speed = speed;

	ret = ioctl(g_uln_fd, MOTOR_MOVE, &move);   
	if (ret < 0) {
		printf("ioctl fadiled with: %s\n", strerror(errno));
		return -1;
	}

	do {
		ioctl(g_uln_fd, MOTOR_GET_STATUS, &status);
		usleep(200 * 1000);
	} while(status.status == MOTOR_MOVE_RUN);
	
#ifdef PTZ_DBG
	printf("XXXXXXXXXXXXXXXXXX %02d, %04d, %04d, %04d\n", 
			status.directional_attr,
			status.total_steps,
			status.current_steps,
			status.cur_speed);
#endif

	ioctl(g_uln_fd, MOTOR_STOP, &move);
	return status.current_steps;
}

int uln2803_set_stop(int which)
{
	int ret = 0;
	ret = ioctl(g_uln_fd, MOTOR_STOP, NULL);
	if (ret < 0) {
		printf("ioctl failed with: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}


int motor_set_speed(int which, int speed)
{
	int ret = 0;
	ret = ioctl(g_uln_fd, MOTOR_SPEED, &speed);
	if (ret < 0) {
		printf("ioctl failed with: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int uln2803_get_status(motor_status * status)
{
	int ret = 0;	
	ret = ioctl(g_uln_fd, MOTOR_GET_STATUS, status);
	if (ret < 0) {
		printf("ioctl failed with: %s\n", strerror(errno));
		return -1;
	}
	
	return 0;
}

int uln2803_reset(motor_reset * reset)
{
	int ret = 0;
	ret = ioctl(g_uln_fd, MOTOR_RESET, reset);
	if (ret < 0) {
		printf("ioctl failed with: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}
