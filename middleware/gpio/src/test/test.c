/*************************************************************************
	> File Name: test.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月22日 星期六 16时56分14秒
 ************************************************************************/

#include <stdio.h>
#include "ysx_gpio.h"


int main(int argc, char ** argv)
{
	int ret = 0;
	ysx_gpio_open(72);
	ysx_gpio_open(73);
	
	ysx_gpio_write(72, 1);
	ysx_gpio_write(73, 0);
	sleep(2);
	ysx_gpio_write(72, 0);
	ysx_gpio_write(73, 1);

	ysx_gpio_close(72);
	ysx_gpio_close(73);
	return 0;
}

