/*************************************************************************
	> File Name: ptz_ctrl.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Mon 13 Aug 2018 10:16:30 AM CST
 ************************************************************************/

#ifndef _PTZ_CTRL_H_
#define _PTZ_CTRL_H_

enum {
    PTZ_MOTOR_HIGH_SPEED    = 0x00,
    PTZ_MOTOR_LOW_SPEED     = 0x01,
    PTZ_MOTOR_SPEED_NUMS    = 0x02,
};


enum {
    MOTOR_SPEED_HIGH		= 0x00,
    MOTOR_SPEED_MEDIUM		= 0x01,
	MOTOR_SPEED_LOW			= 0x02,
	MOTOR_SPEED_HIGH_LOW    = 0x03,
	MOTOR_SPEED_NUMS		= 0X04,
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

	int cur_x_steps;
	int cur_y_steps;
    int total_x_steps;
    int total_y_steps;
} ptz_status;


typedef struct {
	int max_x_steps;
	int max_y_steps;
	int cur_x_steps;
	int cur_y_steps;
} ptz_reset;


int ptz_init(void);
int ptz_deinit(void);
int ptz_set_speed(int which, int speed);
int ptz_set_steps_x(int direction, int steps);
int ptz_set_steps_y(int direction, int steps);
int ptz_stop_turn(int which);
int ptz_get_status(ptz_status * st);
int ptz_set_reset(ptz_reset * reset);

#endif  
