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

struct motor_status_st {
	int directional_attr;
	int total_steps;
	int current_steps;
	int min_speed;
	int cur_speed;
	int max_speed;
	int move_is_min;
	int move_is_max;
	int status;

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
int main(int argc, char ** argv)
{
	int ret = 0;
	int fd = 0;
	int direction = 0;
	int speed = 0;
	int cmd_type = 0;
	struct motor_status_st status = {0};

	fd = open_file(DEV_NAME);
	if (fd < 0) {
		return -1;
	}

	cmd_type = atoi(argv[1]);

	switch(cmd_type)
	{
		case MOTOR_MOVE:
			if (argc != 4) {
				printf("XXXXXXXXXXXXXX motor move argument error!\n");
				printf("ptz_test %d 0 1000\n", MOTOR_MOVE);
				goto finally;
			}
			
			int direction = atoi(argv[2]);		
			int steps = atoi(argv[3]);
			struct motor_move_st move;
			move.motor_directional = direction;
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
			printf("XXXXXXXXXXXXXXXXXX %02d, %04d, %04d, %04d\n", 
				status.directional_attr,
				status.total_steps,
				status.current_steps,
				status.cur_speed);
			
			ioctl(fd, MOTOR_STOP, &move);
			break;
			
		case MOTOR_SPEED:
			if (argc != 3) {
				printf("XXXXXXXXXXXXXX motor move argument error!\n");
				printf("ptz_test %d 500\n", MOTOR_SPEED);
				goto finally;
			}
			int speed = atoi(argv[2]);
			printf("XXXXXXXXX set speed: %d\n", speed);
			ioctl(fd, MOTOR_SPEED, &speed);
			break;
			
		case MOTOR_GET_STATUS:
			if (argc != 2) {
				printf("XXXXXXXXXXXXX get motor argument error!\n");
				goto finally;
			}

			ioctl(fd, MOTOR_GET_STATUS, &status);
			printf("motor pos: (%d, %d)\n", 
				status.cur_x_steps,
				status.cur_y_steps);
			break;
	}
	


finally:
	if (fd > 0) {
		close_file(fd);
	}

	return 0;
}
