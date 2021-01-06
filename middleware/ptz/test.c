/*************************************************************************
	> File Name: teest.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Fri 09 Aug 2019 05:18:05 PM CST
 ************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "ptz_ctrl.h"


/* ioctl cmd */
#define MOTOR_STOP			0x1
#define MOTOR_RESET			0x2
#define MOTOR_MOVE			0x3
#define MOTOR_GET_STATUS	0x4
#define MOTOR_SPEED			0x5
#define MOTOR_GOBACK		0x6
#define MOTOR_CRUISE		0x7

enum PTD_RUNNING_DIR{
	PTD_RUNNING_DIR_UP = 0,
	PTD_RUNNING_DIR_DOWN,
	PTD_RUNNING_DIR_LEFT,
	PTD_RUNNING_DIR_RIGHT,
	PTD_RUNNING_DIR_NUMS,
};


void * test_func(void * arg)
{
	ptz_status status;
	while(1)
	{
		sleep(10);
		ptz_get_status(&status);		
		printf("XXXXXXXXXXXXXXXXXXXXXXXX status: %d,  stop running\n", status.status);
		ptz_stop_turn(0);
	}
	
	return (void *)NULL;
}


int create_test_thread(void)
{
	int ret = 0;
	pthread_t thread_id;
	ret = pthread_create(&thread_id, NULL, test_func, NULL);
	if (ret < 0) {
		printf("create thread failed with: %s\n", strerror(errno));
		return -1;
	}
	pthread_detach(thread_id);
	return 0;
}

typedef struct {
	int index;
	int x_steps;
	int y_steps;
} ptz_point;

ptz_point g_point[6] = {0};


int reach_point(ptz_point * point)
{
	int ret = 0;
	ptz_status status;
	int set_steps = 0;

	ptz_get_status(&status);
	printf("XXXXXXX index: %d,  (%d, %d) -> (%d, %d)\n",
		point -> index,
		status.cur_x_steps, 
		status.cur_y_steps,
		point -> x_steps,
		point -> y_steps);
	usleep(200 * 1000);
	
	// 判断是否被中断,如果被中断,直接退出函数	
	set_steps = point -> x_steps - status.cur_x_steps;
	ret = ptz_set_steps_x((set_steps > 0 ? 1 : 0), abs(set_steps));
	if (set_steps == ret) {
		set_steps = point -> y_steps - status.cur_y_steps;
		ret = ptz_set_steps_y((set_steps > 0 ? 1 : 0), abs(set_steps));
		if (set_steps != ret) {
			ret = -1;
		}
	}
	else {
		ret = -1;
	}
	
	return ret;
}




int main(int argc, char ** argv)
{
	printf("XXXXXXXXXXX build time: %s\n", __TIME__);
	int ret = 0;
	int fd = 0;
	int direction = 0;
	int speed = 0;
	int cmd_type = 0;
	ptz_status status;
	int steps = 0;


	ret = open_ptz();
	if (ret < 0) {
		return -1;
	}

//	ptz_set_speed(0, 300);


	cmd_type = atoi(argv[1]);

	switch(cmd_type)
	{
		case MOTOR_MOVE:
		{
			if (argc != 4) {
				printf("set motor move argument error!\n");
				goto finally;
			}
		
			direction = atol(argv[2]);
			steps = atol(argv[3]);
			printf("set direction: %d, steps: %d\n", direction, steps);
			
			switch(direction)
			{
				ptz_stop_turn(0);
				case PTD_RUNNING_DIR_UP:
					ptz_set_steps_y(1, steps);
					break;
				case PTD_RUNNING_DIR_DOWN:
					ptz_set_steps_y(0, steps);
					break;
				case PTD_RUNNING_DIR_LEFT:
					ptz_set_steps_x(0, steps);
					break;
				case PTD_RUNNING_DIR_RIGHT:
					ptz_set_steps_x(1, steps);
					break;
				default:
					break;
			}
			
			break;
		}

		case MOTOR_SPEED:
		{
			speed = atol(argv[2]);
			if (argc != 3) {
				printf("set motor speed argument error!\n");
				break;
			}
			printf("set motor speed %d\n", speed);
			ptz_set_speed(0, speed);
			break;
		}

		case MOTOR_GET_STATUS:
		{
			break;
		}
	}
	


finally:
	close_ptz();
	return 0;
}
