/*************************************************************************
  > File Name: ptz_test.c
  > Author: unixcc
  > Mail: 2276626887@qq.com 
  > Created Time: Fri 26 Jul 2019 10:07:15 AM CST
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


/* directional_attr */
#define MOTOR_DIRECTIONAL_UP	0x0
#define MOTOR_DIRECTIONAL_DOWN	0x1
#define MOTOR_DIRECTIONAL_LEFT	0x2
#define MOTOR_DIRECTIONAL_RIGHT	0x3

#define DEV_NAME "/dev/motor"


/* directional_attr */
#define MOTOR_DIRECTIONAL_UP	0x0
#define MOTOR_DIRECTIONAL_DOWN	0x1
#define MOTOR_DIRECTIONAL_LEFT	0x2
#define MOTOR_DIRECTIONAL_RIGHT	0x3

/* ioctl cmd */
#define MOTOR_STOP			0x1
#define MOTOR_RESET			0x2
#define MOTOR_MOVE			0x3
#define MOTOR_GET_STATUS	0x4
#define MOTOR_SPEED			0x5
#define MOTOR_GOBACK		0x6
#define MOTOR_CRUISE		0x7


/* init speed */
#define MOTOR_INIT_SPEED	1000	/* unit :ns */

/* rrun status */
#define MOTOR_MOVE_STOP		0x0
#define MOTOR_MOVE_RUN		0x1

struct motor_move_st {
	int motor_directional;
	int motor_move_steps;
	int motor_move_speed;
};


typedef struct {
	int directional_attr;
	int total_steps;
	int current_steps;
	int min_speed;
	int cur_speed;
	int max_speed;
	int move_is_min;
	int move_is_max;
	int status;
} motor_status;

struct motor_reset_data {
	int max_x_steps;
	int max_y_steps;
	int cur_x_steps;
	int cur_y_steps;
};


int open_file(const char * name)
{
	int ret = -1;
	int fd;

	fd = open(name, O_RDWR | O_CLOEXEC);
	if(fd < 0){
		printf("open %s failed!\n", name);
		fd = ret;
	}

	return fd;
} 

int close_file(int fd)
{
	int ret = -1;
	ret = close(fd);
	if(ret < 0){
		printf("close failed!\n");
		return -1;
	}
	return ret;
}


#define SLEEP_TIME (10)
#define MAX_RUN_STEPS (2000)
#define MAX_SPEED (300)
//#define MAX_TURN_RIGHT_STEPS (4500)
//#define MAX_TURN_LEFT_STEPS (2200)

#define MAX_TURN_LEFT_STEPS (4500)
#define MAX_TURN_RIGHT_STEPS (2200)

#define MAX_TURN_DOWN_STEPS (1500)
#define MAX_TURN_UP_niiSTEPS (200)
#define MAX_BUF_SIZE (32)
#define STR_MAX_X_STEPS "X_STEPS"
#define STR_MAX_Y_STEPS "Y_STEPS"

int get_factory_data(const char * name, char * buf, int buflen)
{
	int ret = 0;
	FILE * fp = NULL;
	char cmd[MAX_BUF_SIZE] = {0};

	snprintf(cmd, MAX_BUF_SIZE  - 1, "nvram_get %s", name);
	printf("cmd: %s\n", cmd);
	fp = popen(cmd, "r");
	if (NULL == fp) {
		printf("XXXXXXXXXXXXXXX popen failed with: %s\n", strerror(errno));
		return -1;
	}

	fread(buf, sizeof(char), buflen - 1, fp);
	if (NULL != fp) {
		pclose(fp);
	}	
	return 0;
}


int main(int argc, char ** argv)
{
	int ret = 0;
	int fd = 0;
	int speed, steps;
	
	struct motor_move_st move;
	struct motor_reset_data reset_data = {0};
	motor_status status = {0};
	char max_x_steps[MAX_BUF_SIZE] = {0};
	char max_y_steps[MAX_BUF_SIZE] = {0};

	fd = open_file(DEV_NAME);
	if (fd < 0) {
		return -1;
	}
	

	speed = MAX_SPEED;
	ioctl(fd, MOTOR_SPEED, &speed);

	// 水平归位
	steps = MAX_TURN_LEFT_STEPS;
	move.motor_directional = MOTOR_DIRECTIONAL_LEFT;
	move.motor_move_steps = steps;
	move.motor_move_speed = speed;
	ret = ioctl(fd, MOTOR_MOVE, &move);   
	if (ret < 0) {
		printf("ioctl fadiled with: %s\n", strerror(errno));
		goto finally;
	}
	
	do {
		ioctl(fd, MOTOR_GET_STATUS, &status);
		usleep(200 * 1000);
	} while(status.status == MOTOR_MOVE_RUN);

	
	ioctl(fd, MOTOR_STOP, &move);
	steps = MAX_TURN_RIGHT_STEPS;
	move.motor_directional = MOTOR_DIRECTIONAL_RIGHT;
	move.motor_move_steps = steps;
	move.motor_move_speed = speed;
	ret = ioctl(fd, MOTOR_MOVE, &move);   
	if (ret < 0) {
		printf("ioctl fadiled with: %s\n", strerror(errno));
		goto finally;
	}
	
	do {
		ioctl(fd, MOTOR_GET_STATUS, &status);
		usleep(200 * 1000);
	} while(status.status == MOTOR_MOVE_RUN);

	// 垂直归位
	steps = MAX_TURN_DOWN_STEPS;
	move.motor_directional = MOTOR_DIRECTIONAL_DOWN;
	move.motor_move_steps = steps;
	move.motor_move_speed = speed;
	ret = ioctl(fd, MOTOR_MOVE, &move);   
	if (ret < 0) {
		printf("ioctl fadiled with: %s\n", strerror(errno));
		goto finally;
	}
	
	do {
		ioctl(fd, MOTOR_GET_STATUS, &status);
		usleep(200 * 1000);
	} while(status.status == MOTOR_MOVE_RUN);


    #define MAX_TURN_UP_STEPS  3000	
	steps = MAX_TURN_UP_STEPS;
	move.motor_directional = MOTOR_DIRECTIONAL_UP;
	move.motor_move_steps = steps;
	move.motor_move_speed = speed;
	ret = ioctl(fd, MOTOR_MOVE, &move);   
	if (ret < 0) {
		printf("ioctl fadiled with: %s\n", strerror(errno));
		goto finally;
	}
	
	do {
		ioctl(fd, MOTOR_GET_STATUS, &status);
		usleep(200 * 1000);
	} while(status.status == MOTOR_MOVE_RUN);

	get_factory_data(STR_MAX_X_STEPS, max_x_steps, MAX_BUF_SIZE);
	get_factory_data(STR_MAX_Y_STEPS, max_y_steps, MAX_BUF_SIZE);	
	reset_data.max_x_steps = atol(max_x_steps);
	reset_data.max_y_steps = atol(max_y_steps);
	reset_data.cur_x_steps = MAX_TURN_RIGHT_STEPS;
	reset_data.cur_y_steps = MAX_TURN_UP_STEPS;

	ioctl(fd, MOTOR_RESET, &reset_data);
	ioctl(fd, MOTOR_STOP, NULL);
	
finally:
	if (fd > 0) {
		close_file(fd);
	}

	return 0;
}
