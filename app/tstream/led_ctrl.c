#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qcam_sys.h"

void print_usage()
{
    printf("usage : led_ctrl [options]\n");
    printf("-t <type>       Led type,this value is red/green/blue/off\n");
    printf("-m <mode>       Led mode,this value is flicker or always or strength\n");
	printf("-s <Strength>   Led Strength,this value is 0-100\n");
    printf("-h show help\n");
}

int main(int argc , char **argv)
{
    int ch;
    const char *opt = "t:m:s:h";
    QCAM_LED_MODE led;
    int blink;
    char type[20]={'\0'},mode[20]={'\0'},streng[20]={'\0'};
	int strength;

    while((ch = getopt(argc, argv, opt)) != -1)
    {
        switch(ch)
        {
            case 't':
                strcpy(type,optarg);
                break;
            case 'm':
                strcpy(mode,optarg);
                break;
			case 's':
				strcpy(streng,optarg);
				break;
            case 'h':
            case '?':
                print_usage();
                exit(-1);
                break;
        }
    }
    if(strcmp("red",type) == 0)
	{
        led = LED_MODE_RED;
	}
	else if(strcmp("green",type) == 0)
	{
        led = LED_MODE_GREEN;
	}
	else if(strcmp("blue",type) == 0)
	{
        led = LED_MODE_BLUE;
	}
    else
	{
        led = LED_MODE_OFF;
	}
    if(strcmp("flicker",mode) == 0)
	{
        blink = 1;
	}
	else if(strcmp("strength",mode) == 0)
	{
        blink = 2;
	}
	else
	{
		blink = 0;
	}
    if(blink==2)
	{
		strength = atoi(streng);
		if(strength<=0||strength>100)
		{
			strength=0;
			led = LED_MODE_OFF;
		}
	}
#if 0//defined (DG201)
    QCamLedSet(led,blink,strength);
#else
	QCamLedSet(led,blink);
#endif
    return 0;
}
