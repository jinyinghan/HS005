/*************************************************************************
	> File Name: ysx_gpio.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月22日 星期六 13时26分17秒
 ************************************************************************/

#ifndef _YSX_GPIO_H_
#define _YSX_GPIO_H_


#define CLEAR(x) memset(x, 0, sizeof(x))
#define BUF_SIZE 128


int ysx_gpio_open(int pin);
int ysx_gpio_read(int pin, int *val);
int ysx_gpio_write(int pin , int value);
int ysx_gpio_close(int pin);

#endif 
