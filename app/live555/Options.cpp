/*
 * Ingenic IMP RTSPServer Option variables, modifiable from the command line.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <getopt.h>
#include <dirent.h>
#include <UsageEnvironment.hh>
#include <stdio.h>
#include <unistd.h>

#include "Options.hh"

unsigned int audioNumChannels_t = 1;
unsigned int audioFormat_t = AFMT_PCM_ULAW;
unsigned int audioSamplingFrequency_t = 8000;
unsigned int audioOutputBitrate_t = 8000 * 16 / 2;

int gconf_FPS_Num = 20;
int gconf_FPS_Den = 1;
int gconf_EncSaveFPSNum = 6;
int gconf_EncSaveFPSDen = 1;
int gconf_Main_VideoWidth = 1280;
int gconf_Main_VideoHeight = 720;

int gconf_isp_tuning_thread = false;
char gconf_snapPath[PATH_MAX] = "/tmp";
bool gconf_cli = true;
sensor_type_t gconf_sensor_type = SENSOR_IMX323;
bool gconf_use_internal_move = false;
int gconf_Inversion = false;

// Initialize option variables to default values:

//for create RTSPServer
portNumBits rtspServerPortNum = 8553;
UserAuthenticationDatabase* authDB = NULL;
//for create ServerMediaSession;
char* streamDescription = strDup("RTSP/RTP stream from Ingenic Media");
int videoBitrate = 1024;
QCAM_VIDEO_RESOLUTION resolution = QCAM_VIDEO_RES_720P;
int IRMode = 0;

static int strToInt(char const* str) {
	int val;
	if (sscanf(str, "%d", &val) == 1) return val;
	return -1;
}

const char *opt = "p:w:h:b:m:f:i:";
void print_usage()
{
	printf("usgae : -p rtsp port number [default 8553]\n");
	printf("\t-w video width  [default 1280]\n");
	printf("\t-h video height [default 720]\n");
	printf("\t-b bitrate      [deafult 1M]\n");
	printf("\t-f fps      	  [deafult 20]\n");	
	printf("\t-i inversion    [deafult 0--false , 1--true]\n");	
	printf("\t-m ircut mode :0--auto , 1--night , 2--day  [default auto]\n");
	return;
}
void checkArgs(UsageEnvironment& env, int argc, char** argv) 
{
	int ch; 
	int port=0,width=0,height=0,bitrate=0,mode=0,fps=0;
	
    while((ch = getopt(argc, argv, opt)) != -1) 
    { 
	    switch(ch) 
	    { 
	        case 'p': 
				rtspServerPortNum = strToInt(optarg);
	            break; 
	        case 'w': 
				width = strToInt(optarg);
	            break; 
	        case 'h': 
				height = strToInt(optarg);
	            break; 
	        case 'b': 
				bitrate = strToInt(optarg);
	            break; 
			case 'm':
				mode = strToInt(optarg);
	            break; 
			case 'f':
				fps = strToInt(optarg);
	            break; 	
			case 'i':
				gconf_Inversion = strToInt(optarg);
	            break; 				
	        case '?': 
	            printf( "illegal option: %c\n", ch ); 
	          	print_usage();
				exit(-1);
	            break; 
        } 
    }

	if(width == 1920 && height ==  1080)
	{
		gconf_Main_VideoWidth  = 1920;
		gconf_Main_VideoHeight = 1080;
		resolution = QCAM_VIDEO_RES_1080P;
	}	
	else if(width == 1280 && height == 720 )
	{
		gconf_Main_VideoWidth  = 1280;
		gconf_Main_VideoHeight = 720;
		resolution = QCAM_VIDEO_RES_720P;
	}	
	else if(width == 0 && height == 0 )
	{
	}
	else
	{
		printf("illegal resolution !\n");
		print_usage();
		exit(-1);
	}

	if(bitrate != 0)
		videoBitrate  = bitrate;
	if(fps != 0)
		gconf_FPS_Num = fps;
	if(mode != 0)
		IRMode 		  = mode;

	printf("fps : %d\n", gconf_FPS_Num);
	printf("bitrate : %d\n", videoBitrate);
	printf("width : %d\n",gconf_Main_VideoWidth);
	printf("heght : %d\n",gconf_Main_VideoHeight);
	printf("IRMode : %d\n",IRMode);
	printf("gconf_Inversion : %d\n",gconf_Inversion);

	return;
}
