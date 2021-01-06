/*************************************************************************
	> File Name: uln2803_test.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Thu 09 Aug 2018 05:02:39 PM CST
 ************************************************************************/

#ifndef _ULN2803_H_
#define _ULN2803_H_
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

#define MOTOR_POS_X (0x0)
#define MOTOR_POS_Y (0x1)

#define MOTOR_DIRECTIONAL_UP	0x0
#define MOTOR_DIRECTIONAL_DOWN	0x1
#define MOTOR_DIRECTIONAL_LEFT	0x2
#define MOTOR_DIRECTIONAL_RIGHT	0x3

#define MOTOR_STOP			0x1
#define MOTOR_RESET			0x2
#define MOTOR_MOVE			0x3
#define MOTOR_GET_STATUS	0x4
#define MOTOR_SPEED			0x5
#define MOTOR_GOBACK		0x6
#define MOTOR_CRUISE		0x7


#define MOTOR_INIT_SPEED	1000
#define MOTOR_MOVE_STOP		0x0
#define MOTOR_MOVE_RUN		0x1

typedef enum {
	MOTOR_IS_STOP       = 0x00,
	MOTOR_IS_RUNNING    = 0x01,
} EM_MOTOR_STATUS;


struct motor_move_st {
	int motor_directional;
	int motor_move_steps;
	int motor_move_speed;
};

typedef struct motor_status_st {
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
    int total_x_steps;
    int total_y_steps;
} motor_status;


typedef struct {
	int max_x_steps;
	int max_y_steps;
	int cur_x_steps;
	int cur_y_steps;
} motor_reset;


int uln2803_open(void);
int uln2803_close(void);
int uln2803_set_move(int which, int direction, int steps);
int uln2803_set_stop(int which);
int motor_set_speed(int which, int speed);
int uln2803_get_status(motor_status * status);
int uln2803_reset(motor_reset * reset);



#endif 
