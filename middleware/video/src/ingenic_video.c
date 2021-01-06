    /*************************************************************************
	> File Name: ingenic_video.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月20日 星期四 14时27分23秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>



#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
#include "sample-common.h"

#include "video_input.h"
#include "ingenic_video.h"
#include "common.h"
#include "ysx_gpio.h"

#define TAG "ingenic-video"




void set_ircut_onoff(int day)
{
    ysx_gpio_open(PIN_IRCUT_P);
    ysx_gpio_open(PIN_IRCUT_N);
    ysx_gpio_write(PIN_IRCUT_P, ~day);
    ysx_gpio_write(PIN_IRCUT_N, day);
    ysx_gpio_close(PIN_IRCUT_P);
    ysx_gpio_close(PIN_IRCUT_N);  
}
int get_isp_running_mode() 
{
    int ret = 0;
    IMPISPRunningMode mode;



    ret = IMP_ISP_Tuning_GetISPRunningMode(&mode);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to get running mode\n");
        return -1; 
    }
    printf("get running mode: %d\n", mode);
    return mode;
} 


int set_isp_running_mode(int day) 
{
    int ret = 0;
    IMPISPRunningMode mode;

    if (day) {
        mode = IMPISP_RUNNING_MODE_DAY;
    }
    else {
        mode = IMPISP_RUNNING_MODE_NIGHT;
    }

    ret = IMP_ISP_Tuning_SetISPRunningMode(mode);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to set running mode\n");
        return -1; 
    }

    ret = IMP_ISP_Tuning_GetISPRunningMode(&mode);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to get running mode\n");
        return -1; 
    }
    printf("set running mode: %d\n", mode);
    return 0;
}   


int set_isp_fps(int day)
{
    int ret = 0;
    int fps_num = 0, fps_den = 0;
    if (day) {
        ret = IMP_ISP_Tuning_SetSensorFPS(FPS_DAY_DEFAULT, 1);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "failed to set fps!\n");
            return -1;
        }
    }
    else {
        ret = IMP_ISP_Tuning_SetSensorFPS(FPS_NIGHT_DEFAULT, 1);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "failed to set fps!\n");
            return -1;
        }
    }

    ret = IMP_ISP_Tuning_GetSensorFPS(&fps_num, &fps_den);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to get fps!\n");
        return -1;
    }
    printf("set fps_num: %d, fps_den: %d\n", fps_num, fps_den);
    return 0;
}


int set_isp_antiflicker(int day)
{
    int ret = 0;
    IMPISPAntiflickerAttr flick_attr;

    if (day) {
        flick_attr = IMPISP_ANTIFLICKER_DISABLE;
    }
    else {
        flick_attr = IMPISP_ANTIFLICKER_50HZ;
    }

    ret = IMP_ISP_Tuning_SetAntiFlickerAttr(flick_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to set antiflicker!\n");
        return -1; 
    }

    ret = IMP_ISP_Tuning_GetAntiFlickerAttr(&flick_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "failed to set antiflicker!\n");
        return -1; 
    }  
    printf("set antiflicker mode: %d\n", flick_attr);
    return 0;
}


int set_ircut_mode(int mode)
{
    if (mode < IRCUT_MODE_ON || mode > IRCUT_MODE_AUTO) {
        QLOG(FATAL, "ircut arguments error!\n");
        return -1;
    }

  //  int ret = 0; 
    switch(mode)
    {
        case IRCUT_MODE_ON:     // ҹ��
            set_ircut_onoff(CURRENT_IS_NIGHT);
            set_isp_antiflicker(CURRENT_IS_NIGHT);
            set_isp_running_mode(CURRENT_IS_NIGHT);
            usleep(300 * 1000);
            set_isp_fps(CURRENT_IS_NIGHT);
            break;
            
        case IRCUT_MODE_OFF:    // ����
            set_ircut_onoff(CURRENT_IS_DAY);
            set_isp_antiflicker(CURRENT_IS_DAY);
            set_isp_running_mode(CURRENT_IS_DAY);
            usleep(300 * 1000);
            set_isp_fps(CURRENT_IS_DAY);
            break;
            
        case IRCUT_MODE_AUTO:
//            ircut_mode_auto();
            break;
            
        default:
            break;
    }
    return 0;
}



IMPCell framesource_chn[] = {
    {DEV_ID_FS, 0, 0},                       //Main Stream
    {DEV_ID_FS, 1, 0},                       //Second Stream
   // {DEV_ID_FS, 3, 0}, 
    {DEV_ID_FS, 2, 0},                       //Third Stream
};

IMPCell ivs_grp = {DEV_ID_IVS, 0, 0};

IMPCell osd_grp[] = {
    {DEV_ID_OSD, 0, 0},      //Main Stream
    {DEV_ID_OSD, 1, 0},      //Second Stream   
   // {DEV_ID_OSD, 3, 0}, 
    {DEV_ID_OSD, 2, 0},      //Third Stream
};  

IMPCell encoder_chn[] = {
    {DEV_ID_ENC, 0, 0},                       //Main Stream
    {DEV_ID_ENC, 1, 0},                       //Second Stream
    //{DEV_ID_ENC, 3, 0},
    {DEV_ID_ENC, 2, 0},                       //Third Stream
};



// bind framesource -> osd -> encoder
// bind framesource -> ivs -> osd -> encoder
int system_module_bind(void)
{
	int ret = 0;
    int index = 0;
	//printf("++++++++++++++++max_fs %d\n",MAX_FS);
	#ifdef USE_MOTION
    for (index = 0; index < MAX_FS; index ++) {
	//printf("++++++++++++++++index %d\n",index);
		if(MAX_FS - 1 != index){
        	ret = IMP_System_Bind(&framesource_chn[index], &osd_grp[index]);
        	if (ret < 0) {
            	IMP_LOG_ERR(TAG, "bind framesource %d to osd group %d failed!\n", index, index);
            	return -1;
        	}
        	ret = IMP_System_Bind(&osd_grp[index], &encoder_chn[index]);   
    		if (ret < 0) {
    			IMP_LOG_ERR(TAG, "bind osd group %d to endoder channel %d failed!\n", index, index);
    			return -1;
    		}
		}
        //// 使用最小的码流作为ivs的输入
		else{
            ret = IMP_System_Bind(&framesource_chn[index], &ivs_grp);   
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "bind framesource channel %d to ivs failed!\n", index);
                return -1;
            }  
            
            ret = IMP_System_Bind(&ivs_grp, &osd_grp[index]);   
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "bind ivs to osd failed!\n");
                return -1;
            }  

            ret = IMP_System_Bind(&osd_grp[index], &encoder_chn[index]);   
        	if (ret < 0) {
        		IMP_LOG_ERR(TAG, "bind osd group %d to encoder channel %d failed!\n", index, index);
        		return -1;
        	}                    
        }
    }
	#else
	   for (index = 0; index < MAX_FS; index ++) {
	
        	ret = IMP_System_Bind(&framesource_chn[index], &osd_grp[index]);
        	if (ret < 0) {
            	IMP_LOG_ERR(TAG, "bind framesource %d to osd group %d failed!\n", index, index);
            	return -1;
        	}
        	ret = IMP_System_Bind(&osd_grp[index], &encoder_chn[index]);   
    		if (ret < 0) {
    			IMP_LOG_ERR(TAG, "bind osd group %d to endoder channel %d failed!\n", index, index);
    			return -1;
    		}
		}
      
   	
	#endif
	//printf("out\n");
	return 0;
}


int system_module_unbind(void)
{
	int ret;
    int index = MAX_FS - 1;
	#ifdef USE_MOTION
    for (index; index >= 0; index --) {
        if (MAX_FS - 1 == index) {
            ret = IMP_System_UnBind(&framesource_chn[index], &ivs_grp);   
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "unbind framesource channel %d to ivs failed!\n", index);
                return -1;
            }  
            
            ret = IMP_System_UnBind(&ivs_grp, &osd_grp[index]);   
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "unbind ivs to osd failed!\n");
                return -1;
            }  

            ret = IMP_System_UnBind(&osd_grp[index], &encoder_chn[index]);   
        	if (ret < 0) {
        		IMP_LOG_ERR(TAG, "unbind osd group %d to encoder channel %d failed!\n", index, index);
        		return -1;
        	}                    
        }    
        else{
        	ret = IMP_System_UnBind(&framesource_chn[index], &osd_grp[index]);
        	if (ret < 0) {
            	IMP_LOG_ERR(TAG, "unbind framesource %d to osd group %d failed!\n", index, index);
            	return -1;
        	}
        	ret = IMP_System_UnBind(&osd_grp[index], &encoder_chn[index]);   
    		if (ret < 0) {
    			IMP_LOG_ERR(TAG, "unbind osd group %d to endoder channel %d failed!\n", index, index);
    			return -1;
    		}
        }
    }
    #else
	for (index; index >= 0; index --) {
     
        	ret = IMP_System_UnBind(&framesource_chn[index], &osd_grp[index]);
        	if (ret < 0) {
            	IMP_LOG_ERR(TAG, "unbind framesource %d to osd group %d failed!\n", index, index);
            	return -1;
        	}
        	ret = IMP_System_UnBind(&osd_grp[index], &encoder_chn[index]);   
    		if (ret < 0) {
    			IMP_LOG_ERR(TAG, "unbind osd group %d to endoder channel %d failed!\n", index, index);
    			return -1;
    		}
        
    }
	#endif
	return 0;
}


/*
void * get_video_stream(void * arg)
{
    int ret = 0;
    int i = 0, len = 0;
    int keyframe;
    struct timeval tv;
	IMPEncoderStream stream = {0};
    video_input_context * vc = (video_input_context *)arg;
    
	prctl(PR_SET_NAME, vc -> thread_name);
	ret = IMP_Encoder_StartRecvPic(vc -> channel);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "start receive vide channel %d data failed!\n", vc -> channel);
		return (void *)-1;
	} 

    vc -> started = 1;
	while(1)
	{
		ret = IMP_Encoder_PollingStream(vc -> channel, 2000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG,  "polling video stream timeout, chn[%d]\n", vc -> channel);
			continue;
		}

		ret = IMP_Encoder_GetStream(vc -> channel, &stream, 1);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
			continue;
		}


		for (i = 0, len = 0; i < stream.packCount; i++) {            
			len += stream.pack[i].length;
		} 

		gettimeofday(&tv, NULL);   
		//keyframe = (stream.refType == IMP_Encoder_FSTYPE_IDR)?1:0;
		int keyframe = 0;
        if (vc -> cb) {
            vc -> cb(vc -> channel, &tv, (void *)stream.virAddr, len, keyframe);   
        }
		IMP_Encoder_ReleaseStream(vc -> channel, &stream);

        if (vc -> destroyed) {
            IMP_LOG_ERR(TAG, "exit getting video data thread...\n");
            break;
        }
	}

	ret = IMP_Encoder_StopRecvPic(vc -> channel);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "stop receive video data failed!\n");
		return (void *)-1;
	} 
}

*/





