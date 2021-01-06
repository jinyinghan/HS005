#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "qcam_sys.h"



static void usage(char *name)
{   
    printf("please input:gpio_opt -n pin -m mode -v [data]\n");  
	printf("pin  : pin num\n");
	printf("mode : 0 -- write 1---read\n");
	printf("value: if mode is 0 , then data is what you write\n");
	printf("example: gpio_opt -n 1 -m 1 -v 1\n");
	printf("         gpio_opt -n 1 -m 0 \n");
}


int main(int argc, char **argv)
{
    int val=-1;
	int pin=-1;
	int mode=-1;
	int ch; 
	int ret;


    pin = atoi(argv[1]);
    mode = atoi(argv[2]);
    printf("mode %d , blink = %d\n",pin,mode);
    QCamLedSet(pin, mode);

    return 0;
}
