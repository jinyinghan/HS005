#include "ysx_thread.h"
//#include "device.h"
ysx_pthread_flag_s ysx_p_flag = {0};
ysx_pthread_id_s ysx_p_id = {0};
unsigned char ledset=0;



int ch=0;
void *pthread_led_ctrl(void *arg)
{
    prctl(PR_SET_NAME,"p_led_ctrl");
    pr_info("#### start ...\n");
    int i_ret = 0, i_con_status = -1, i_con_status_dis = -1;
    while (!ysx_p_flag.p_led_ctrl_en){
        //pr_info("%d %d\n", i_ret, GetLedStatus());
        sleep(3);
        //printf("led 1+++++++++++++++++++++++++++\n");
        i_ret = GetYSXCtl(YSX_C_EN);
        if((i_ret == CAM_CLARM_OFF) || (i_ret == CAM_CLARM_ON_OFF)) {
           LedEnable(0);
           //ysx_led_en(0,ledset);
            continue;
        }
		//printf("led 2+++++++++++++++++++++++++++\n");
        i_ret = GetYSXCtl(YSX_LIGHT_EN);
        if(i_ret == CAM_CLARM_OFF){
          LedEnable(0);
          //ysx_led_en(0,ledset);
            continue;
        }
        LedEnable(1);
		//printf("led 2+++++++++++++++++++++++++++\n");
        if(GetYSXCtl(YSX_VIEW_NUM) > 0) {
			if(GetLedStatus()!=ON_LINE_STATUS){
              i_ret = SetLed(1, ON_LINE_STATUS);
		   	  ledset=2;
			}
            continue;
        }
        //get the route connect status
        #if 0
        if(g_enviro_struct.d_online== 0) {
		   
			if(GetLedStatus()!=OFF_LINE_STATUS){
               i_ret = SetLed(1, OFF_LINE_STATUS);
		   ledset=OFF_LINE_STATUS;
		   if(g_enviro_struct.fill_light==1){
			ysx_fill_light_mode(66,FILL_CLOSE,ledset,GetYSXCtl(YSX_LIGHT_EN));
			ch=0;
		   	}
		}
            continue;
        }
		#endif
        //get the net connect status
       // printf("!!!!!!!!!!!!!!!!!!!!!!!!!g_enviro_struct.t_online :%d\n",g_enviro_struct.t_online);
        //printf("led 3+++++++++++++++++++++++++++\n");
        if(GetYSXCtl(YSX_NET_T_ONLINE) == 1) {
			
			if(GetLedStatus()!=ON_LINE_STATUS){
              i_ret = SetLed(1, ON_LINE_STATUS);
		   	  ledset=2;
			}
			
            continue;
        	}else{
		  	if(GetLedStatus()!=OFF_LINE_STATUS){
               i_ret = SetLed(1, OFF_LINE_STATUS);
		   	   ledset=OFF_LINE_STATUS;
		  	}
		  
		      continue;
			}
		//printf("led 4+++++++++++++++++++++++++++\n");
		//printf("xxxxxxxxxxxxxx33331111111111!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        i_con_status = internet_is_ok();
        if((i_con_status_dis == -1) ||
            (i_con_status_dis != i_con_status)) {
            i_con_status_dis = i_con_status;
            if(i_con_status_dis != 0) {
                update_route_gw();
            } else {
                update_route_ip_mac();
            }
        }
        switch(i_con_status) {
            case 0: {
			
				if(GetLedStatus()!=EZ_STATUS){
                i_ret = SetLed(1, EZ_STATUS);
				
				ledset=EZ_STATUS;
					}
            }break;
            case 1: {
				if(GetLedStatus()!=EZ_STATUS_ROUTE){
               i_ret = SetLed(1, EZ_STATUS_ROUTE);
			   ledset=EZ_STATUS_ROUTE;
					}
            }break;
            case 2: {
				if(GetLedStatus()!=ON_LINE_STATUS){
               i_ret = SetLed(1, ON_LINE_STATUS);
			   ledset=ON_LINE_STATUS;
					}
            }break;
            default:
				if(GetLedStatus()!=EZ_STATUS){
               i_ret = SetLed(1, EZ_STATUS);
			   ledset=EZ_STATUS;
					}
            break;
            }
        sleep(2);
    }
    pr_info("#### exit ...\n");
    pthread_exit(0);
    return (void *)0;
}

int Initialization_Pthread_Led_Ctrl(void *arg)
{
    LOG("enter Initialization_Pthread_Led_Ctrl\n");
    int i_ret = -1;
    i_ret = pthread_create(&ysx_p_id.id_led_ctrl, NULL, &pthread_led_ctrl, NULL);
    if(i_ret < 0) {
        pr_error("create fail, ret=[%d]\n", i_ret);
        return i_ret;
    }
    pthread_detach(ysx_p_id.id_led_ctrl);
	LOG("exit Initialization_Pthread_Led_Ctrl\n");
    return i_ret;
}

void Uninitialization_Pthread_Led_Ctrl(void)
{
    pr_dbg("in \n");
    ysx_p_flag.p_led_ctrl_en = 1;
    if(ysx_p_id.id_led_ctrl){
		printf("led pthread close++++++++++++++++\n");
        pthread_join(ysx_p_id.id_led_ctrl, NULL);
    	}
}

