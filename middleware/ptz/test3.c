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
        sleep(1);
        ptz_get_status(&status);		
        printf("XXXXXXXXXXXXXXXXXXXXXXXX status: %d, x_steps: %d y_steps: %d total_x_steps: %d total_y_steps: %d cur_steps: %d\n",status.status,status.cur_x_steps,status.cur_y_steps,status.total_x_steps,status.total_y_steps,status.current_steps);
        //ptz_stop_turn(0);
        int steps = ptz_get_steps(0);
        printf("%d\n",steps);
        steps = ptz_get_steps(1);
        printf("%d\n",steps);
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

int main(int argc, char ** argv)
{
    printf("XXXXXXXXXXX build time: %s\n", __TIME__);
    int ret = 0;
    //int fd = 0;
    //int direction = 0;
    //int speed = 0;
    //int cmd_type = 0;
    ptz_status status;
    int steps = 0;
    if(argc != 2 && argc != 3){
        printf("param fault\n");
        return 0; 
    }

    ret = open_ptz();
    if (ret < 0) {
        return -1;
    }
    int direction = atoi(argv[1]);
      
    //ptz_set_speed(0, 500);//是指速度，越大越快，建议(100-1000)
    #if 0
    ptz_reset reset = {0};
    memset(&reset,0,sizeof(reset));
    reset.max_x_steps = 12000;
    reset.max_y_steps = 8000;
    reset.cur_x_steps = 6000;
    reset.cur_y_steps = 4000; 
    ptz_set_reset(&reset);
    #endif
    steps = 5000;
    if(argc == 3)
    steps = atoi(argv[2]);

    //create_test_thread();
    printf("set steps %d\n",steps);
    switch(direction){
        case(0):
            ret = ptz_set_steps_x(0, steps);//水平最大步数4000步，
                break;
        case(1):
        ret = ptz_set_steps_x(1,steps);
                break;
        case(2):
    
        ret = ptz_set_steps_y(0, steps);//垂直电机最大步数应该根据角度来计算

                break;
        case(3):
        ret = ptz_set_steps_y(1, steps);
                break;
        case(4):
            ret = ptz_set_steps_x(0, steps);//水平最大步数4000步，
            ret = ptz_set_steps_x(1, steps/2);
                break;
    }




finally:
    close_ptz();
    return 0;
}
