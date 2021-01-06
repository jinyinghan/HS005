/*
 * ipc_ircut.c
 * Author:yuanhao@yingshixun.com
 * Date:2019.4
 */

#include "ipc_ircut.h"
#include <imp/imp_log.h>
#include "ysx_video.h"

static int ev_log = 0;
#define ircut_dbg(...) \
    do { \
        if(ev_log) { \
            fprintf(stderr, __VA_ARGS__); \
        } \
    } while(0)

int software_ir_process(int top,int button,int flag);

typedef struct {
	QCAM_IR_MODE ir_mode;
    unsigned char running;
	QCAM_IR_STATUS ir_status;
    pthread_t ircut_pid;
    pthread_mutex_t ircut_lock;
	int has_light;
} IPC_IRCUT_INFO_S;

static IPC_IRCUT_INFO_S *g_ircut_args = NULL;

static int ircut_gpio_write(int pin , int value)
{
    int fd;
	char buf[128];
	
    memset(buf, 0, sizeof(buf));
    snprintf(buf, 128,"/sys/class/gpio/gpio%d/value",pin);
    fd = open(buf,O_WRONLY);
	if(fd < 0){
		QLOG(FATAL, "set gpio %d direction error %s!\n", pin, strerror(errno));
		return -1;
	}

    if (value == 0)
        write(fd,"0",sizeof("0"));
    else
        write(fd,"1",sizeof("1"));
    close(fd);
   
    return 0;
}

static int ipc_ircut_switch_mode(QCAM_IR_STATUS enCurrentStatus)
{
	struct timeval tv;
    gettimeofday(&tv, NULL);
    char pline[50];
    memset(pline, '\0', sizeof(pline));

    sprintf(pline, "%s%d", "touch /tmp/ircuttime_", (tv.tv_sec));
    if(system("rm /tmp/ircuttime_*") != 0)
        QLOG(FATAL, "ysx_ircut_set error: %s\n", strerror(errno));
    if(system(pline) != 0)
        QLOG(FATAL, "ysx_ircut_set error: %s, %s\n", pline, strerror(errno));
    QLOG(TRACE,"ysx_ircut_set dbg: %s, %d\n", pline, tv.tv_sec);

    int ret = -1;
    if (enCurrentStatus == DAY_MODE){
	    ircut_gpio_write(IRCUT_N, 1);
        ircut_gpio_write(IRCUT_P, 0); 
        usleep(300*1000);
        ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
        ircut_gpio_write(IRCUT_P, 0); 
        usleep(100*1000);
        ircut_gpio_write(IR_LED, IR_LED_OFF);

		QLOG(FATAL,"IMP_ISP_Tuning_SetSensorFPS mode Day, fps %d\n", SENSOR_FPS_DAY);

		if (IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_DAY, 1) != 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS Error\n");
			return ;
		}	   
		if (IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY) != 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error\n");
			return ;
		}
		if (IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISP_ANTIFLICKER_50HZ) != 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_SetAntiFlickerAttr Error\n");
			return -1;
		}
    }
    else{
		QLOG(FATAL,"IMP_ISP_Tuning_SetSensorFPS mode Night, fps %d\n", SENSOR_FPS_NIGHT);
		
		if (IMP_ISP_Tuning_SetSensorFPS(SENSOR_FPS_NIGHT, 1) != 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_SetSensorFPS Error\n");
			return -1;
		}	   
		if (IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_NIGHT) != 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_GetISPRunningMode Error\n");
			return -1;
		}
		if (IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISP_ANTIFLICKER_DISABLE) != 0) {
			QLOG(FATAL, "IMP_ISP_Tuning_SetAntiFlickerAttr Error\n");
			return -1;
		}

	    ircut_gpio_write(IRCUT_N, 0);
        ircut_gpio_write(IRCUT_P, 1);  
        usleep(300*1000);
        ircut_gpio_write(IRCUT_N, 0);  // IRCUT --> open
        ircut_gpio_write(IRCUT_P, 0);   
        usleep(100*1000);
        ircut_gpio_write(IR_LED, IR_LED_ON);

    }

	g_ircut_args->ir_status = enCurrentStatus;
    return 0;
}

#define USE_SOFT_IR        0

#if 1
static void* ipc_ircut_thread(void* arg)
{
    prctl(PR_SET_NAME, __FUNCTION__);

    int ret = -1; 
    QCAM_IR_STATUS enCurrentStatus = UNKNOW_MODE;
    QCAM_IR_STATUS enLastStatus = UNKNOW_MODE;
    QCAM_IR_STATUS enLastStatusTmp = enLastStatus;
	
    int iADCVal = 0;
    int iCnt = 0;

	int lg_top    = DAY_THRESHOLD;
    int lg_button = NIGHT_THRESHOLD;
	ysx_read_config(SYMBOL_LIGHTNESS_TOP2, &lg_top);
	ysx_read_config(SYMBOL_LIGHTNESS_BUTTON2, &lg_button);
	int ev_log = 0;	
    ysx_read_config(SYMBOL_EV_LOG, &ev_log);  

	printf("ipc_ircut_thread start ev_log %d\n",ev_log);
	
    while (g_ircut_args->running){
        if (g_ircut_args->ir_mode == QCAM_IR_MODE_AUTO){
			#if USE_SOFT_IR
			IMPISPEVAttr bright;
			//static uint8_t agian_cnt =0;
	
			//static uint8_t gain_low = true, gain_hig = true;
    		ret = IMP_ISP_Tuning_GetEVAttr(&bright);
    		if (ret < 0) {
        		QLOG(FATAL, "IMP_ISP_Tuning_GetEVAttr Error with result %d\n",ret);
        		//return -1;
    		} 
			if(ev_log)
				QLOG(TRACE,"[%s]--again=%d --gain_log2=%d\n",__FUNCTION__,bright.again,bright.gain_log2);
			if (bright.ev > lg_button){
				enLastStatusTmp = NIGHT_MODE;
			}else if (bright.ev < lg_top){
				enLastStatusTmp = DAY_MODE;
			}
			#else
			if(ev_log)
				printf("######iADCVal=%d######\n", iADCVal);
			ret = SU_ADC_GetChnValue(ADC_CHN, &iADCVal);
			if (ret < 0) {
				QLOG(FATAL,"SU_ADC_GetChnValue failed !\n");
			}
			
			if (iADCVal > lg_button){
				enLastStatusTmp = NIGHT_MODE;
			}else if (iADCVal < lg_top){
				enLastStatusTmp = DAY_MODE;
			}
			#endif
			
			

			
			if(enCurrentStatus != enLastStatusTmp){
				iCnt ++;
				if(iCnt > 10){
					enLastStatus = enLastStatusTmp;
					iCnt = 0;
				}
			}
			else if(iCnt){
				iCnt = 0;
			}
        }
        else if (g_ircut_args->ir_mode == QCAM_IR_MODE_ON){
        	enLastStatus = NIGHT_MODE;
        }
        else if (g_ircut_args->ir_mode == QCAM_IR_MODE_OFF){
        	enLastStatus = DAY_MODE;
        }

        if (enLastStatus != enCurrentStatus){
			enCurrentStatus = enLastStatus;
			ret = ipc_ircut_switch_mode(enCurrentStatus);
			if(ret){
        		QLOG(FATAL, "Error with %#x.\n", ret); 
        		pthread_exit((void*)0);
    		}
        }

        usleep(200*1000);
    }

   pthread_exit((void*)0);
}
#endif


#if 0
#if 0
static int ircut_switch = 0;


int get_has_light_status()
{
	int has_light = 0;
	pthread_mutex_lock(&g_ircut_args->ircut_lock);
	has_light =	g_ircut_args->has_light ;
	pthread_mutex_unlock(&g_ircut_args->ircut_lock);
	return has_light;
	
}


extern int has_light = 1;


static void* ipc_ircut_thread(void* arg)
{
    prctl(PR_SET_NAME, __FUNCTION__);

    int ret = -1; 
    //QCAM_IR_STATUS enCurrentStatus = UNKNOW_MODE;
   // QCAM_IR_STATUS enLastStatus = UNKNOW_MODE;
    //QCAM_IR_STATUS enLastStatusTmp = enLastStatus;
	
    ///int iADCVal = 0;
    ///int iCnt = 0;

	///int lg_top    = DAY_THRESHOLD;
    //int lg_button = NIGHT_THRESHOLD;
	///ysx_read_config(SYMBOL_LIGHTNESS_TOP2, &lg_top);
	//ysx_read_config(SYMBOL_LIGHTNESS_BUTTON2, &lg_button);
	int ev_log = 0;	

	int i = 0;
	float gb_gain,gr_gain;
	float iso_buf;
	
	int ircut_status = 1;
	int night_count = 0;
	int day_count = 0;
	//int day_oth_count = 0;
	//bayer域的 (g分量/b分量) 统计值
	float gb_gain_record = 200;
	float gr_gain_record = 200;
	float gb_gain_buf = 200, gr_gain_buf = 200;
	IMPISPRunningMode pmode;
	IMPISPEVAttr ExpAttr;
	IMPISPWB wb;
    ///ysx_read_config(SYMBOL_EV_LOG, &ev_log);  

	IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
	//ysx_ircut_set2(0);
    while (g_ircut_args->running){
       // if (g_ircut_args->ir_mode == QCAM_IR_MODE_AUTO){
        printf("=====================================\n");
		int ret = IMP_ISP_Tuning_GetEVAttr(&ExpAttr);
		if (ret ==0) {
			printf("u32ExposureTime: %d\n", ExpAttr.ev);
			printf("u32AnalogGain: %d\n", ExpAttr.again);
			printf("u32DGain: %d\n", ExpAttr.dgain);
		} else {
			usleep(1000*1000);
			printf("get ev failed\n");
			continue;
			
        }
		iso_buf = ExpAttr.ev;
		printf(" iso buf ==%f\n",iso_buf);
		ret = IMP_ISP_Tuning_GetWB_Statis(&wb);
		if (ret == 0) {
			gr_gain =wb.rgain;
			gb_gain =wb.bgain;
			printf("gb_gain: %f\n", gb_gain);
			printf("gr_gain: %f\n", gr_gain);
			printf("gr_gain_record: %f\n", gr_gain_record);
		} else {
			usleep(1000*1000);
			printf("get wb failed\n");
			continue;
        }
		printf("=====================================\n");
		//平均亮度小于20，则切到夜视模式
		if (iso_buf > 41000) {
			night_count++;
			printf("night_count==%d\n",night_count);
			if (night_count>3 /*&& get_ircut_status() == 0*/) {
				printf("now is night mode\n");
				/*
				IMP_ISP_Tuning_GetISPRunningMode(&pmode);
				if (pmode!=IMPISP_RUNNING_MODE_NIGHT) {
					printf("### entry night mode ###\n");
					IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_NIGHT);
					ysx_ircut_set2(QCAM_IR_MODE_ON);
					ircut_status = true;
				*/
				//	QCamSetIRCut(1);
					ircut_status = 1;
					pthread_mutex_lock(&g_ircut_args->ircut_lock);
					g_ircut_args->has_light = 0;
					pthread_mutex_unlock(&g_ircut_args->ircut_lock);
				}
				//切夜视后，取20个gb_gain的的最小值，作为切换白天的参考数值gb_gain_record ，gb_gain为bayer的G/B
				for (i=0; i<20; i++) {
					IMP_ISP_Tuning_GetWB_Statis(&wb);
					gr_gain =wb.rgain;
					gb_gain =wb.bgain;
					if (i==0) {
						gb_gain_buf = gb_gain;
						gr_gain_buf = gr_gain;
					}
					gb_gain_buf = ((gb_gain_buf>gb_gain)?gb_gain:gb_gain_buf);
					gr_gain_buf = ((gr_gain_buf>gr_gain)?gr_gain:gr_gain_buf);
					usleep(300000);
					gb_gain_record = gb_gain_buf;
					gr_gain_record = gr_gain_buf;
					// printf("gb_gain == %f,iso_buf=%f",gb_gain,iso_buf);
					// printf("gr_gain_record == %f\n ",gr_gain_record);
				}
			}
		else {
			night_count = 0;
        }
		//满足这3个条件进入白天切换判断条件
		if (((int)iso_buf < 11000) &&( ircut_status == 1) &&(gb_gain>gb_gain_record+15)) {
			if ((iso_buf<3000)||(gb_gain >145)) {
				day_count++;
            } else {
				day_count=0;
            }
			// printf("gr_gain_record == %f gr_gain =%f line=%d\n",gr_gain_record,gr_gain,__LINE__);
			// printf("day_count == %d\n",day_count);
			if (day_count>3) {
				/*
				printf("### entry day mode ###\n");
				IMP_ISP_Tuning_GetISPRunningMode(&pmode);
				if (pmode!=IMPISP_RUNNING_MODE_DAY) {
					IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
					//sample_SetIRCUT(1);
					ysx_ircut_set2(QCAM_IR_MODE_OFF);
					ircut_status = false;
				}
				*/
				printf("now is day mode\n");
				//QCamSetIRCut(0);
				ircut_status = 0;
				pthread_mutex_lock(&g_ircut_args->ircut_lock);
				g_ircut_args->has_light = 1;
				pthread_mutex_unlock(&g_ircut_args->ircut_lock);
			}
		} else {
            day_count = 0;
        }
        
        usleep(1000*1000);
    }

   pthread_exit((void*)0);
}

#else
uint8_t ircut_status = true;

int get_has_light_status()
{
	int has_light = 0;
	pthread_mutex_lock(&g_ircut_args->ircut_lock);
	has_light =	!ircut_status;
	pthread_mutex_unlock(&g_ircut_args->ircut_lock);
	return has_light;
	
}


//extern int has_light = 1;

static int suspend = 1;

int suspend_light_set(int su){

	suspend = su;
	return 0;
}

static void* ipc_ircut_thread(void* arg){
	//IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
	//ysx_ircut_set2(0);
    while (g_ircut_args->running){
		//ev_log = 1;
		if(suspend)
			software_ir_process(0,0,ev_log);
		sleep(1);
	}
}


void ir_led_close(void)
{
    ircut_gpio_write(IR_LED, IR_LED_OFF);
    printf("ir led close\n");
    return ;
}

void ir_led_open(void)
{
    ircut_gpio_write(IR_LED, IR_LED_ON);
    printf("ir led open\n");
    return ;
}


int ircut_pre_atcion(void)//此时红外灯 已经关闭
{
    const int a3 = 1000;
    IMPISPEVAttr bright;
    
    ir_led_close();
    sleep(2);//稳定一下

    IMP_ISP_Tuning_GetEVAttr(&bright);

    ircut_dbg("----------------- 0 ircut pre atc -----------------\n");
    ircut_dbg("exp = %d\ntime = %d\n", bright.ev, bright.expr_us);
    ircut_dbg("again = %d, dgain = %d\n", bright.again, bright.dgain);

    sleep(2);//稳定一下

    IMP_ISP_Tuning_GetEVAttr(&bright);

    ircut_dbg("----------------- 1 ircut pre atc -----------------\n");
    ircut_dbg("exp = %d\ntime = %d\n", bright.ev, bright.expr_us);
    ircut_dbg("again = %d, dgain = %d\n", bright.again, bright.dgain);

    sleep(2);//稳定一下

    IMP_ISP_Tuning_GetEVAttr(&bright);

    ircut_dbg("----------------- 2 ircut pre atc -----------------\n");
    ircut_dbg("exp = %d\ntime = %d\n", bright.ev, bright.expr_us);
    ircut_dbg("again = %d, dgain = %d\n", bright.again, bright.dgain);


    if(bright.ev < a3)//切到白天
    {
        return 1;
    }
    else//切到夜视 本来就是也是 恢复ir led即可
    {
        ir_led_open();
        return 0;
    }
}

int statictist_exp_red(float *red, const int loop_time)
{
    IMPISPEVAttr bright;
    IMPISPWB wb;
    int gb_gain, gr_gain;
    int gb_gain_buf , gr_gain_buf ;
	int exp_sum = 0;
	float red_sum = 0;
//	const int loop_time=10;
	int i = 0;
	
	for (i=0; i<loop_time; i++) {
		IMP_ISP_Tuning_GetEVAttr(&bright);
		IMP_ISP_Tuning_GetWB_Statis(&wb);
		gb_gain = wb.bgain;
		gr_gain = wb.rgain;
		red_sum += (gr_gain*1.0/gb_gain);//统计红外强度 red0
		exp_sum += bright.ev;//统计 曝光量
		
//		if (flag) {
			ircut_dbg("----------------- %02d -----------------\n", i);
			ircut_dbg("ev = %d, time : %d\n", bright.ev, bright.expr_us);
			ircut_dbg("rgain = %d, bgain = %d\n", gr_gain, gb_gain);
			ircut_dbg("R/B record = %f\n", (gr_gain*1.0/gb_gain));
//		}
		usleep(400000);  //400ms
	}
	if(red)
		*red = red_sum/loop_time;
	
	return (int)(exp_sum/loop_time);
}


int software_ir_process(int top,int button,int flag)
{

    int ret;
    IMPISPEVAttr bright;

    
    IMPISPWB wb;
    int rgain = 0, bgain = 0, exp = 0;
    float red = 0.0;
    static float red0 = 0.0;
    static int exp0 = 0;
    static int small_scenes_flag = 0; //小场景模式 0:正常场景, 1:小场景但未预切过, 2:小场景且待预切过
//    static int count = 0;
	static int night_cnt = 0,day_cnt = 0;
    const int a0 = 100000, a1 = 3000, a2 = 1000; // a0切夜视的 a1正常切白天 a2小场景
    const float B0 = 0.2, B1 = 1.2, B2 = 10;

    IMP_ISP_Tuning_GetWB_Statis(&wb);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetWB Error with result %d\n",ret);
        return -1;
    }

    ret = IMP_ISP_Tuning_GetEVAttr(&bright);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetEVAttr Error with result %d\n",ret);
        return -1;
    }

    exp = bright.ev;
    rgain = wb.rgain;
    bgain = wb.bgain;
    red = (rgain*1.0)/bgain;

    if(ircut_status)
        ircut_dbg("--------------- night ----------------\n");
    else
        ircut_dbg("---------------- day -----------------\n");
    
    ircut_dbg("ev = %d, time = %d\n", exp, bright.expr_us);
    ircut_dbg("rgain = %d, bgain = %d\n", rgain, bgain);
    ircut_dbg("R/B = %f\n", red);
    ircut_dbg("small_scenes_flag = %d\n", small_scenes_flag);

    // 进入夜晚模式
    if (ircut_status == false  && exp > a0)
    {
        night_cnt++;
        if (night_cnt > 4)
        { 
            night_cnt = 0;

            //if (IR_AUTO_ENABLE) 
            {
              //  ysx_ircut_set(QCAM_IR_MODE_ON);
                sleep(3);//抵抗扰动时间
            
                small_scenes_flag = 0;
                exp0 = statictist_exp_red(&red0, 10);
                ircut_dbg("----------------- average -----------------\n");
                ircut_dbg("exp0 = %d, red0 = %f, a2 = %d\n", exp0, red0, a2);
                
                if (exp0 < a2) 
                {
                    small_scenes_flag = 1;//进入小场景模式,    下一次进来再进行小场景判断
                }
                ircut_dbg("small_scenes_flag = %d\n", small_scenes_flag);
            }
            pthread_mutex_lock(&g_ircut_args->ircut_lock);
            ircut_status = true;
			pthread_mutex_unlock(&g_ircut_args->ircut_lock);
        }
    }
    else {
        night_cnt = 0;
    }
    
    //小场景模式  (纸盒子盖住应该能进入)
    if (ircut_status == true && small_scenes_flag > 0)
    {
        ircut_dbg("ev change: %f\n", abs(exp0-exp)*1.0/exp0);
        ircut_dbg("red change: %f\n", abs(red0-red)*1.0/red0);
        
        if(2 == small_scenes_flag)
        {
            if(abs(exp0-exp)*1.0/exp0 > B0 && abs(exp0-exp)*1.0/exp0 < B2) //光线发生一点点变化
            {            
                if(ircut_pre_atcion()) // 预切
                {
                    exp0 = statictist_exp_red(&red0, 10);
                    ircut_dbg("----------------- average -----------------\n");
                    ircut_dbg("exp0 = %d, red0 = %f, a2 = %d\n", exp0, red0, a2);
                    
                    if (exp0 < a2 && exp < exp0) // 白天
                    {
                     //   if (IR_AUTO_ENABLE) {
                      //      ysx_ircut_set(QCAM_IR_MODE_OFF);
                     //   }
                     	pthread_mutex_lock(&g_ircut_args->ircut_lock);
                        ircut_status = false;
						pthread_mutex_unlock(&g_ircut_args->ircut_lock);
                    }
                }
                else {
                    sleep(3);//抵抗扰动时间
                
                    small_scenes_flag = 0;
                    exp0 = statictist_exp_red(&red0, 10);
                    ircut_dbg("----------------- average -----------------\n");
                    ircut_dbg("exp0 = %d, red0 = %f, a2 = %d\n", exp0, red0, a2);
                    
                    if (exp0 < a2)
                    {
                        small_scenes_flag = 1;//进入 小场景模式
                    }
                }
            }
            else if(abs(exp0-exp)*1.0/exp0 >= B2) // 恢复正常场景
            {
                small_scenes_flag = 0;
            }
    //        else  //光线没有发生什么变化
    //        {    
    //            if(exp1 == exp)//exp1为上次 小场景预切的exp记录 
    //            {
    //                count = 0;
    //            }
    //            else//没有预切过
    //            {
    //                count++;
    //                printf("@@@@@@@@@@@@@@@@@@@@@ count = %d\n", count);
    //                if(count > 10)
    //                {
    //                    exp1=exp;//记录下当前ev值
    //                    ircut_pre_atcion();//进入ircut 预切
    //                    if(ircut_status == true)
    //                    {
    //                        sleep(2);
    //                        exp0 = statictist_exp_red(NULL,flag);
    //                        small_scenes_flag = 0;
    //                        printf("###################### after pre atcion: exp0 = %d\n", exp0);
    //                        if (exp0 < a1) { // exp0 <a1 且 red0 >red1
    //                            small_scenes_flag = 1; //进入小场景模式
    //                            count = 0;//count = 0
    //                        }
    //                    }
    //                    else {
    //                        small_scenes_flag = 0;
    //                    }
    //                }
    //            }
    //        }
        }
        else
        {
            small_scenes_flag = 2;
        }
    }
    //正常场景模式
    else if(ircut_status == true && ((exp < a1 && red < B1)))// 若干次小于a1则为 进入白天模式
    {
        day_cnt++;
        if (day_cnt > 4) {
            day_cnt = 0;
          //  if (IR_AUTO_ENABLE) {
             //   ysx_ircut_set(QCAM_IR_MODE_OFF);
           // }
            pthread_mutex_lock(&g_ircut_args->ircut_lock);
            ircut_status = false;
			pthread_mutex_unlock(&g_ircut_args->ircut_lock);
            small_scenes_flag = 0;
        }
    }
    else {
        day_cnt = 0;
    }

    return 0;
}

#endif
#endif



uint8_t ircut_status = true;

int get_has_light_status()
{
	int has_light = 0;
	pthread_mutex_lock(&g_ircut_args->ircut_lock);
	has_light =	!ircut_status;
	pthread_mutex_unlock(&g_ircut_args->ircut_lock);
	return has_light;
	
}


//extern int has_light = 1;

static int suspend = 1;

int suspend_light_set(int su){

	suspend = su;
	return 0;
}

static int ircut_gpio_open(int pin)
{
	char buf[128];

	if(pin < 0 || pin > 96){
		printf("illegal pin number!\n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf,128,"echo %d > /sys/class/gpio/export",pin);
	system(buf);    

	return 0;
}

static int ircut_gpio_output(int pin)
{
	char buf[128];
	int fd;
	memset(buf, 0, sizeof(buf));	
	snprintf(buf,128,"/sys/class/gpio/gpio%d/direction",pin);
	fd = open(buf,O_RDWR);
	if(fd < 0){
		QLOG(FATAL, "set gpio %d output error %s!\n", pin, strerror(errno));
		return -1;
	}

	write(fd,"out",sizeof("out"));
	close(fd);

	return 0;
}

static int ircut_gpio_init()
{
	int ret;
	ret = ircut_gpio_open(IRCUT_P);
	if(ret < 0){
		QLOG(FATAL, "open IRCUT_P %d faield\n",IRCUT_P);
		return -1;
	}

	ret = ircut_gpio_output(IRCUT_P);
	if(ret < 0){
		QLOG(FATAL, "gpio_output IRCUT_P %d faield\n",IRCUT_P);
		return -1;
	}

	ret = ircut_gpio_open(IRCUT_N);
	if(ret < 0){
		QLOG(FATAL, "open IRCUT_N %d faield\n",IRCUT_N);
		return -1;
	}

	ret = ircut_gpio_output(IRCUT_N);
	if(ret < 0){
		QLOG(FATAL, "gpio_output IRCUT_N %d faield\n",IRCUT_N);
		return -1;
	}

	ret = ircut_gpio_open(IR_LED);
	if(ret < 0){
		QLOG(FATAL, "open IR_LED %d faield\n",IR_LED);
		return -1;
	}

	ret = ircut_gpio_output(IR_LED);
	if(ret < 0){
		QLOG(FATAL, "gpio_output IR_LED %d faield\n",IR_LED);
		return -1;
	}

    ircut_gpio_write(IRCUT_N, 0);  
    ircut_gpio_write(IRCUT_P, 0); 
    ircut_gpio_write(IR_LED, 0);

	return 0;
}

static int ircut_ADC_init()
{
	int ret;
    ret = SU_ADC_Init();
	if(ret < 0){
		QLOG(FATAL, "SU_ADC_Init failed !\n");
		return -1;
	}

    ret = SU_ADC_EnableChn(ADC_CHN);
	if(ret < 0){
		QLOG(FATAL, "SU_ADC_EnableChn failed !\n");
		return -1;
	}

	return 0;
}

int ipc_ircut_init(void)
{
	if(NULL != g_ircut_args){
        QLOG(FATAL, "ipc_ircut_init failed\n"); 
        return -1;
    }
	int ret = 0;

    g_ircut_args = (IPC_IRCUT_INFO_S *)malloc(sizeof(IPC_IRCUT_INFO_S));
	if(NULL == g_ircut_args){
        QLOG(FATAL, "failed to malloc %d bytes.\n", sizeof(IPC_IRCUT_INFO_S)); 
        return -1;
    }
    memset(g_ircut_args, 0, sizeof(IPC_IRCUT_INFO_S));

    pthread_mutex_init(&g_ircut_args->ircut_lock, NULL);
    g_ircut_args->ir_mode = QCAM_IR_MODE_UNSUPPORT;

    ret = ircut_gpio_init();
	if(ret){
        QLOG(FATAL, "Error with %#x.\n", ret); 
        return -1;
    }
	
    ret = ircut_ADC_init();
	if(ret){
        QLOG(FATAL, "Error with %#x.\n", ret); 
        return -1;
    }
	
    g_ircut_args->running = 1;
	g_ircut_args->has_light = 1;
    ret = pthread_create(&g_ircut_args->ircut_pid, NULL, ipc_ircut_thread, NULL);
	if(ret){
		QLOG(FATAL, "Error with %s.\n", strerror(errno)); 
		return -1;
	}

    return 0;
}

int ipc_ircut_exit()
{
	if(NULL == g_ircut_args){
        QLOG(FATAL, "ipc_ircut_init is not inited.\n"); 
        return -1;
    }

    if (g_ircut_args->running){
        g_ircut_args->running = 0;
        pthread_join(g_ircut_args->ircut_pid, NULL);
    }
	
	SU_ADC_DisableChn(ADC_CHN);
    SU_ADC_Exit();
	
    pthread_mutex_destroy(&g_ircut_args->ircut_lock);
    free(g_ircut_args);
    g_ircut_args = NULL;

    return 0;
}

int ipc_ircut_mode_set(QCAM_IR_MODE mode)
{
	if(NULL == g_ircut_args){
        QLOG(FATAL, "ipc_ircut_init is not inited.\n"); 
        return -1;
    }

    pthread_mutex_lock(&g_ircut_args->ircut_lock);
    g_ircut_args->ir_mode = mode;
    pthread_mutex_unlock(&g_ircut_args->ircut_lock);

    return 0;
}



QCAM_IR_MODE ipc_ircut_mode_get()
{
	if(NULL == g_ircut_args){
        QLOG(FATAL, "ipc_ircut_init is not inited.\n"); 
        return -1;
    }

    return g_ircut_args->ir_mode;
}

QCAM_IR_STATUS ipc_ircut_status_get()
{
	if(NULL == g_ircut_args){
        QLOG(FATAL, "ipc_ircut_init is not inited.\n"); 
        return -1;
    }

    return g_ircut_args->ir_status;
}


