#ifndef __MOTOR_H__
#define __MOTOR_H__


/*
电机步数统计规则如下：
1 保持摄像头正面对操作人员, 规定向右转为步数累加方向
2 保持摄像头正面对操作人员, 规定向上转为步数累加方向
3 如果不同的模具上下电机对调,为保持应用层控制代码不变，需要调整驱动满足以上规则
*/

// 水平电机
#define MOTOR_NUMS (2)
#define MOTOR0_MAX_SPEED	(1000)	
#define MOTOR0_MIN_SPEED	(10)
#define MOTOR0_MIN_GPIO		(-1)		
#define MOTOR0_MAX_GPIO		(-1)		
#define MOTOR0_GPIO_LEVEL	(0)	
//#define MOTOR0_ST1_GPIO		GPIO_PC(17)
//#define MOTOR0_ST2_GPIO	 	GPIO_PC(18)
//#define MOTOR0_ST3_GPIO		GPIO_PB(19)
//#define MOTOR0_ST4_GPIO		GPIO_PB(21)
//#define MOTOR0_ST1_GPIO		GPIO_PB(18)
//#define MOTOR0_ST2_GPIO	 	GPIO_PC(18)
//#define MOTOR0_ST3_GPIO		GPIO_PB(13)
//#define MOTOR0_ST4_GPIO		GPIO_PB(14)

#define MOTOR0_ST1_GPIO		GPIO_PB(26)
#define MOTOR0_ST2_GPIO		GPIO_PB(25)
#define MOTOR0_ST3_GPIO	    GPIO_PB(18)
#define MOTOR0_ST4_GPIO	    GPIO_PB(17)



// 垂直电机
#define MOTOR1_MAX_SPEED	(1000)
#define MOTOR1_MIN_SPEED	(10)
#define MOTOR1_MIN_GPIO		(-1)	
#define MOTOR1_MAX_GPIO		(-1)	
#define MOTOR1_GPIO_LEVEL	(0)
//#define MOTOR1_ST1_GPIO		GPIO_PC(14)
//#define MOTOR1_ST2_GPIO		GPIO_PC(13)
//#define MOTOR1_ST3_GPIO		GPIO_PC(12)
//#define MOTOR1_ST4_GPIO		GPIO_PC(11)
//#define MOTOR1_ST1_GPIO		GPIO_PC(11)
///#define MOTOR1_ST2_GPIO		GPIO_PC(12)
//#define MOTOR1_ST3_GPIO		GPIO_PC(13)
//#define MOTOR1_ST4_GPIO		GPIO_PC(14)

#define MOTOR1_ST1_GPIO		GPIO_PB(26)
#define MOTOR1_ST2_GPIO		GPIO_PB(25)
#define MOTOR1_ST3_GPIO	    GPIO_PB(18)
#define MOTOR1_ST4_GPIO	    GPIO_PB(17)


#define DEF_MOTOR(NO) \
	 struct motor_platform_data jz_motor##NO##_pdata = { \
	    .name = "jz-tcu", \
	    .motor_min_gpio = MOTOR##NO##_MIN_GPIO, \
	    .motor_max_gpio = MOTOR##NO##_MAX_GPIO, \
	    .motor_gpio_level = MOTOR##NO##_GPIO_LEVEL, \
	    .motor_st1_gpio = MOTOR##NO##_ST1_GPIO, \
	    .motor_st2_gpio = MOTOR##NO##_ST2_GPIO, \
	    .motor_st3_gpio = MOTOR##NO##_ST3_GPIO, \
	    .motor_st4_gpio = MOTOR##NO##_ST4_GPIO, \
	    .max_speed = MOTOR##NO##_MAX_SPEED, \
	    .min_speed = MOTOR##NO##_MIN_SPEED, \
	};

#define MOTOR_POS_UP (0)
#define MOTOR_POS_DOWN (1)


#define MOTOR_MOVE_STOP		0x0
#define MOTOR_MOVE_RUN		0x1


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

/* init speed */
#define MOTOR_INIT_SPEED	1000	/* unit :ns */

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
	int total_x_steps;
	int total_y_steps;
};

struct motor_platform_data {
	const char * name;
	int directional_attr;

	unsigned int motor_min_gpio;
	unsigned int motor_max_gpio;
	int motor_gpio_level;

	unsigned int motor_st1_gpio;
	unsigned int motor_st2_gpio;
	unsigned int motor_st3_gpio;
	unsigned int motor_st4_gpio;

	int max_speed;
	int min_speed;
	int cur_speed;
	int step_angle;
};

struct motor_attribute {
	int total_steps;
	int cur_steps;
	int set_steps;
};


struct motor_info {
	struct semaphore semaphore;
	struct platform_device * pdev;
	const struct mfd_cell * cell;
	struct device * dev;
	struct miscdevice mdev;
	struct regulator * power;
	struct motor_platform_data * pdata[MOTOR_NUMS];
	struct mutex lock;
	struct task_struct * motor_thread;
	struct completion time_completion;
	struct jz_tcu_chn * tcu;
	wait_queue_head_t motor_wq;


	volatile status;
	int total_steps[MOTOR_NUMS];
	int current_steps[MOTOR_NUMS];
	int run_step;
	int run_step_irq;
	int speed;
	int move_is_min;
	int move_is_max;

	
	int direction;	// 当前的方向
	int id; // 当前的指定的电机
	int cur_steps[4];
	int set_steps[4];

	struct motor_attribute mattr[MOTOR_NUMS];
	int inited;
};

struct motor_reset_data {
	int max_x_steps;
	int max_y_steps;
	int cur_x_steps;
	int cur_y_steps;
};

struct motor_poinit {
	int index;
	int x;
	int y;
};


#endif
