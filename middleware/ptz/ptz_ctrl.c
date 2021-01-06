/*************************************************************************
	> File Name: ptz_ctl.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Mon 13 Aug 2018 10:16:22 AM CST
 ************************************************************************/

#include <stdio.h>
#include <errno.h>  
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "uln2803.h"
#include "ptz_ctrl.h"

#define PTZ_CONTOR_VERSION "V2.0.2"


int open_ptz(void)                                                                                                                              
{
	return uln2803_open();
}

int close_ptz(void)
{
	return uln2803_close();
}


int ptz_set_speed(int which, int speed)
{	
	return motor_set_speed(which, speed);
}

int ptz_set_steps_x(int direction, int steps)
{
    system("gpio_opt -n 62 -m 1 -v 0");
	return uln2803_set_move(MOTOR_POS_X, direction, steps);
}

int ptz_set_steps_y(int direction, int steps)
{
    system("gpio_opt -n 62 -m 1 -v 1");
	return uln2803_set_move(MOTOR_POS_Y, direction, steps);
}

int ptz_stop_turn(int which)
{   
	return uln2803_set_stop(which);
}   

int ptz_get_status(ptz_status * st)
{
	return uln2803_get_status((motor_status *)st);		
}
int ptz_get_steps(int which){
    if((which < 0) || (which > 1)){
        printf("not recoginzed motor num!\n");
        return 0;
    }
    ptz_status st;
    uln2803_get_status((motor_status *)&st);
    if(st.status){
        st.current_steps = st.current_steps > 0 ? st.current_steps : 0;
        return st.current_steps; 
    }
    return 0;    


}

int ptz_set_reset(ptz_reset * reset)
{
	return uln2803_reset((motor_reset *)reset);
}



