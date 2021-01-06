# include       <stdio.h>
# include       <stdarg.h>

#include "include/common_env.h"
#include "include/common_func.h"



/**** var declare***********/


/********** func declare*********/
int SetPass(char *pass);
int video_channel_get(int video_quality);

/**********func implementations**********/

void set_play_path_prefix(char *prefix)
{
	FILE *fp = fopen("/var/run/language","w");
	if(fp)
	{
		fprintf(fp,"/usr/bin/%s",prefix);
		fclose(fp);
	}
	return;
}
void SetFillLightTime(int tStart_o,int tStart_s, int tEnd_o,int tEnd_s)
{
	g_enviro_struct.t_nStart_o = tStart_o;
	g_enviro_struct.t_nStart_s = tStart_s;
	g_enviro_struct.t_nEnd_o = tEnd_o;
	g_enviro_struct.t_nEnd_s = tEnd_s;

	int  str[25], str1[25],str2[25],str3[25];
	sprintf((char *)str, "%d", tStart_o);
	sprintf((char *)str1,"%d", tStart_s);

	sprintf((char *)str2, "%d", tEnd_o);
	sprintf((char *)str3, "%d", tEnd_s);

	dictionary * ini;
	//  ini = iniparser_load(CAMERA_CONF);
	ini = iniparser_load(DEV_CONF);
	
	char pline[128],pline1[128];
	memset(pline, '\0', sizeof(pline));
	sprintf((char *)pline, "%s:%s", "ysx", "start_o");
	dictionary_set(ini, (const char * )pline, (const char * )str);

	memset(pline, '\0', sizeof(pline));
	sprintf((char *)pline, "%s:%s", "ysx", "start_s");
	dictionary_set(ini,  (const char * )pline,  (const char * )str1);

	memset(pline1, '\0', sizeof(pline1));
	sprintf((char *)pline1, "%s:%s", "ysx", "end_o");
	dictionary_set(ini,  (const char * )pline1,  (const char * )str2);

	memset(pline1, '\0', sizeof(pline1));
	sprintf((char *)pline1, "%s:%s", "ysx", "end_s");
	dictionary_set(ini,  (const char * )pline1,  (const char * )str3);
	
	inisync_file(ini, DEV_CONF);
	iniparser_freedict(ini);
}

void GetNightTime(int *tStart_o,int *tStart_s, int *tEnd_o,int *tEnd_s)
{
	*tStart_o = g_enviro_struct.t_nStart_o;
	*tStart_s = g_enviro_struct.t_nStart_s;
	*tEnd_o = g_enviro_struct.t_nEnd_o;
	*tEnd_s = g_enviro_struct.t_nEnd_s;
}

int InitDeviceCommonConfig()
{
	dictionary * ini;
    ini = iniparser_load(DEV_CONF);
	if(ini == NULL ){
		LOG("Can not load %s\n",DEV_CONF);
		return -1;
	}

	char pline[128];
	
	CLEAR(pline);
	sprintf(pline, "%s:%s", "common", "pass");
	const char *pass = iniparser_getstring(ini, pline, "admin");
	strcpy(g_enviro_struct.t_pass, pass);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "common", "model");
	const char *model = iniparser_getstring(ini, pline, "c20");
	strcpy(g_enviro_struct.t_model, model);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "common", "vendor");
	const char *vendor = iniparser_getstring(ini, pline, "ysx");
	strcpy(g_enviro_struct.t_vendor, vendor);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "common", "lan");
	const char *lan = iniparser_getstring(ini, pline, "ch");
	strcpy(g_enviro_struct.t_lan, lan);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "motion", "enable");
	g_enviro_struct.motion_en= iniparser_getint(ini, pline, VAVA_CTRL_DISABLE);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "motion", "sense");
	g_enviro_struct.t_sense = iniparser_getint(ini, pline, VAVA_MD_SENSITIVITY_LOW);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "motion", "type");
	g_enviro_struct.motion_type = iniparser_getint(ini, pline, VAVA_DETECT_MOVEMNT);
	
	/////////////////ysx/////////////
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "video_quality");
	g_enviro_struct.vc_default = iniparser_getint(ini, pline, VAVA_VIDEO_QUALITY_2K);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "video_mirrorflip");
	g_enviro_struct.vc_mf = iniparser_getint(ini, pline, VAVA_MIRROR_TYPE_NORMAL);


	LOG("camera quality %d mf %d pass %s|\n",
		g_enviro_struct.vc_default,
		g_enviro_struct.vc_mf,
		g_enviro_struct.t_pass);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "camera_en");
	g_enviro_struct.camera_en = iniparser_getint(ini, pline, 1);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "ir_mode");
	g_enviro_struct.ir_mode = iniparser_getint(ini, pline, 0);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "mic_en");
	g_enviro_struct.mic_en = iniparser_getint(ini, pline, 1);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "spk_en");
	g_enviro_struct.mic_en = iniparser_getint(ini, pline, 1);	

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "vol");
	g_enviro_struct.spk_vol = iniparser_getint(ini, pline, 15);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "light_en");
	g_enviro_struct.light_en = iniparser_getint(ini, pline, 1);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "voiceinfo_en");
	g_enviro_struct.t_volinfoEn = iniparser_getint(ini, pline, 1);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "sharepass");
	const char *share_pass = iniparser_getstring(ini, pline, "admin");
	strcpy(g_enviro_struct.share_pass, share_pass);

	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "share_en");
	g_enviro_struct.share_en = iniparser_getint(ini, pline, 0);
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "region_bit");
	g_enviro_struct.region_bit= iniparser_getint(ini, pline, 0xffff);
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "md_start");
	g_enviro_struct.md_tStart= iniparser_getint(ini, pline, 0);
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "md_end");
	g_enviro_struct.md_tEnd= iniparser_getint(ini, pline, 24);
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "track_sense");
	g_enviro_struct.track_sense= iniparser_getint(ini, pline, 3);
#ifdef CONFIG_TRACK_DRAWRECT
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "osd_en");
	g_enviro_struct.mosd_en= iniparser_getint(ini, pline, 0);
#endif
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "login_type");
	g_enviro_struct.login_type = iniparser_getint(ini, pline, 0);
#ifdef IOTYPE_USER_IPCAM_GET_ALERT_PROMPT_RESP
	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "dev_alert");
	g_enviro_struct.dev_alert = iniparser_getint(ini, pline, 0);
#endif

#ifdef CAM_ALARM_ON
    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "time_alarm_day");
    g_enviro_struct.time_alarm_day= iniparser_getint(ini, pline, 0);
    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "time_alarm_ts");
    g_enviro_struct.time_alarm_ts= iniparser_getint(ini, pline, 0);
    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "time_alarm_te");
    g_enviro_struct.time_alarm_te= iniparser_getint(ini, pline, 0);
#endif
#ifdef IVS_SENSE
    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "sensitivity");
    g_enviro_struct.sensitivity= iniparser_getint(ini, pline, 1);
#endif


	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "start_o");
	g_enviro_struct.t_nStart_o = iniparser_getint(ini, pline, 18); 

	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "start_s");
	g_enviro_struct.t_nStart_s = iniparser_getint(ini, pline, 0); 
	
	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "end_o");
	g_enviro_struct.t_nEnd_o = iniparser_getint(ini, pline, 6); 
	
	memset(pline, '\0', sizeof(pline));//
	sprintf(pline, "%s:%s", "ysx", "fill_mode");
	g_enviro_struct.t_fillMode= iniparser_getint(ini, pline, 0); 
	
	memset(pline, '\0', sizeof(pline));//
	sprintf(pline, "%s:%s", "ysx", "fill_open");
	g_enviro_struct.t_fillOpen= iniparser_getint(ini, pline, 0); 
	
	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "end_s");
	g_enviro_struct.t_nEnd_s = iniparser_getint(ini, pline, 0); 
	
	memset(pline, '\0', sizeof(pline));//
    sprintf(pline, "%s:%s", "ysx", "aiopen");
    g_enviro_struct.t_aiMode= iniparser_getint(ini, pline, 1); 
   
    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "main_frequency");
    g_enviro_struct.t_mainFrequency = iniparser_getint(ini, pline, 2);

	
	CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "auto_bitrate");
    g_enviro_struct.auto_bitrate = iniparser_getint(ini, pline, 1);

	CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "storage_resolution");
    g_enviro_struct.storage_resolution = iniparser_getint(ini, pline, YSX_RESOLUTION_HIGH);
	g_enviro_struct.record_status = YSX_RECORD_END;
	g_enviro_struct.resolution_status =YSX_RESOLUTION_SET;

    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "sound_detection");
    g_enviro_struct.sound_detection = iniparser_getint(ini, pline, VAVA_CTRL_DISABLE);

    CLEAR(pline);
    sprintf(pline, "%s:%s", "ysx", "detection_timer");
    g_enviro_struct.detection_timer = iniparser_getint(ini, pline, 0);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "record_quality");
	g_enviro_struct.record_quality = iniparser_getint(ini, pline, VAVA_VIDEO_QUALITY_HIGH);

	CLEAR(pline);
	sprintf(pline, "%s:%s", "ysx", "alarm_record_time");  // 10~180 s
	g_enviro_struct.alarm_record_time = iniparser_getint(ini, pline, 10);
	
	//初始化 录像模式
	char json_data[128]={0};
	GetYSXCtlString(YSX_RECORD_MODE, json_data, sizeof(json_data));
	parse_storage_cmd_json(json_data,VAVA_CMD_SET_RECMODE, &g_enviro_struct.record_mode);
		
	//初始化侦测区域
	memset(json_data, 0, sizeof(json_data));
	GetYSXCtlString(YSX_DETECT_REGION, json_data, sizeof(json_data));
	parse_system_cmd_json(json_data,VAVA_CMD_SET_AI_REGION, &g_enviro_struct.detect_region);

	iniparser_freedict(ini);
	set_play_path_prefix(g_enviro_struct.t_lan);

	g_enviro_struct.wifi_switch_st = -1;  
	return 1;
}
int InitDevIR(int c_en)
{
	int i_ir_val = 0;
	if (c_en == 1)
	{//start camera
		i_ir_val = Get_cfg_from_conf(YSX_IR_MODE_B_YSX_C_EN);
		if (i_ir_val != 1)
		{
			SetYSXCtl(YSX_IR_MODE, i_ir_val);
			SetYSXCtl(YSX_IR_MODE_B_YSX_C_EN, 1);
		}
	}
	else
	{//close camera
		i_ir_val = Get_cfg_from_conf(YSX_IR_MODE);
		if (i_ir_val != 1)
		{
			SetYSXCtl(YSX_IR_MODE, 1);			//1: dis
			SetYSXCtl(YSX_IR_MODE_B_YSX_C_EN, i_ir_val);
		}
	}
	//pr_dbg("val:(%d, %d)\n", c_en, i_ir_val);
	return 0;
}
int InitDevDtc(int c_en)
{
	int i_dtc_val = 0;
	if (c_en == 1)
	{//start camera
		i_dtc_val = Get_cfg_from_conf(YSX_MOTION_B_YSX_C_EN);
		if (i_dtc_val != 0)
		{
			SetYSXCtl(YSX_MOTION, i_dtc_val);
			SetYSXCtl(YSX_MOTION_B_YSX_C_EN, 0);
		}
	}
	else
	{//close camera
		i_dtc_val = Get_cfg_from_conf(YSX_MOTION);
		if (i_dtc_val != 0)
		{
			SetYSXCtl(YSX_MOTION, 0);			//0: dis
			SetYSXCtl(YSX_MOTION_B_YSX_C_EN, i_dtc_val);
		}
	}
	//pr_dbg("val:(%d, %d)\n", c_en, i_dtc_val);
	return 0;
}
int InitDeviceSetting()
{
	SetYSXCtl(YSX_C_EN, GetYSXCtl(YSX_C_EN));
	InitDevDtc(GetYSXCtl(YSX_C_EN));
	InitDevIR(GetYSXCtl(YSX_C_EN));
	return 0;
}

int SetPass(char *pass)
{

	memset(g_enviro_struct.t_pass,0,sizeof(g_enviro_struct.t_pass));
	strcpy(g_enviro_struct.t_pass, pass);

	dictionary * ini;
  //  ini = iniparser_load(CAMERA_CONF);
	ini = iniparser_load(DEV_CONF);

	char pline[128];
	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "common", "pass");
    dictionary_set(ini, pline, pass);

	inisync_file(ini, DEV_CONF);
	iniparser_freedict(ini);

	return 1;
}

int GetShareePass(char *passwd)
{
	strcpy(passwd ,g_enviro_struct.share_pass);
	return 1;
}

int SetSharePass(char *pass)
{
	memset(g_enviro_struct.share_pass,0,sizeof(g_enviro_struct.share_pass));
	strcpy(g_enviro_struct.share_pass, pass);
	dictionary * ini;
  //  ini = iniparser_load(CAMERA_CONF);
	ini = iniparser_load(DEV_CONF);

	char pline[128];
	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "sharepass");
    dictionary_set(ini, pline, pass);

	inisync_file(ini, DEV_CONF);
	iniparser_freedict(ini);

	printf("[%s]:--%s--\n",__FUNCTION__,g_enviro_struct.share_pass);
	return 1;
}



void set_ir_mode(int mode)
{
	if(mode == VAVA_IRLED_MODE_AUTO )
		QCamSetIRMode(QCAM_IR_MODE_AUTO);
	else if(mode == VAVA_IRLED_MODE_CLOSE)
		QCamSetIRMode(QCAM_IR_MODE_OFF);
	else{}
	
	return;
}

void SetMotionSense(unsigned int sense)
{
	return;
}
static time_t camera_moving = 0;
int GetYSXCtl(int iType)
{
	switch(iType)
	{
		case YSX_CAMERA_MOVE:
			{
				time_t now = time(NULL);
				int tmp = (now-camera_moving);
//				printf("tmp %ld , %ld , diff = %d\n",now,camera_moving,tmp);
#ifdef PATRACK
				if ( tmp >= 30) //motor stop
					return false;
				else
					return true;
#else
				if( tmp >= 30)//motor stop
					return 1;
				else
					return 0;

#endif
			}
		case YSX_C_EN:
			return g_enviro_struct.camera_en;
		case YSX_IR_MODE:
			return g_enviro_struct.ir_mode;
		case YSX_MIC_EN:
			return g_enviro_struct.mic_en;
		case YSX_SPK_EN:
			return g_enviro_struct.spk_en;
		case YSX_VINFO_EN:
			return g_enviro_struct.t_volinfoEn;
		case YSX_LIGHT_EN:
			return g_enviro_struct.light_en;
		case YSX_VOL:
			return g_enviro_struct.spk_vol;
		case YSX_MOTION:
			return g_enviro_struct.motion_en;
		case YSX_ROTATE:
			return g_enviro_struct.vc_mf;
		case YSX_SHARE:
			return g_enviro_struct.share_en;
		case YSX_VQUALITY:
			return g_enviro_struct.vc_default;
		case YSX_INVERSION:
			return g_enviro_struct.vc_mf;
		case YSX_MD_SENSE:
			return g_enviro_struct.t_sense;
	    case YSX_MD_TYPE:
	        return g_enviro_struct.motion_type;
		case YSX_MD_REGION_BIT: 
		    printf("@@@@@@@ MD region bit: %d\n", g_enviro_struct.region_bit);
			return g_enviro_struct.region_bit;
#ifdef PATRACK
		case YSX_AUTOTURN:
			return g_enviro_struct.ptz_autoturn_flag;
		case YSX_ATRACK_EN:
			return g_enviro_struct.atrack_en;
		case YSX_PTRACK_EN:
			return g_enviro_struct.ptrack_en;
		case YSX_PTRACK_TT:
			return g_enviro_struct.ptrack_tt;
		case YSX_PTRACK_TS:
			return g_enviro_struct.ptrack_ts;
		case YSX_PTRACK_TE:
			return g_enviro_struct.ptrack_te;

#endif
#ifdef CONFIG_TRACK_DRAWRECT
		case YSX_MOTION_OSD:
			return g_enviro_struct.mosd_en;
#endif
		case YSX_WIFI_SWTICH_ST:
			return g_enviro_struct.wifi_switch_st;
        case YSX_CLOUD_ENV:
            return g_enviro_struct.server_env;
		case YSX_DEV_LOGIN_TYPE:
            return g_enviro_struct.login_type;
#ifdef IOTYPE_USER_IPCAM_GET_ALERT_PROMPT_RESP
	case YSX_DEV_ALERT:
		return g_enviro_struct.dev_alert;

#ifdef CAM_ALARM_ON
		case YSX_CAM_TIME_ALARM_DAY:
		{
			return g_enviro_struct.time_alarm_day;
		}break;
		case YSX_CAM_TIME_ALARM_TS:
		{
			return g_enviro_struct.time_alarm_ts;
		}break;
		case YSX_CAM_TIME_ALARM_TE:
		{
			return g_enviro_struct.time_alarm_te;
		}break;
#endif
#ifdef IVS_SENSE
		case YSX_IVS_SENSE:
		{
			return g_enviro_struct.sensitivity;
		}break;
#endif
#endif
		case YSX_STORAGE_RESOLUTION:
		{
			return g_enviro_struct.storage_resolution;
		}break;
		
		
		case YSX_AUTO_BITRATE:
		{
			return g_enviro_struct.auto_bitrate;
		}break;
		case YSX_FILL_LED_MODE:{			
			return g_enviro_struct.t_fillMode;
		}break;
		case YSX_FILL_LED_OPEN:{			
			return g_enviro_struct.t_fillOpen;
		}break;
		case YSX_AI_MODE:{			
			return g_enviro_struct.t_aiMode;
		}break;
		case YSX_MAINS_FREQUENCY:{		
			return g_enviro_struct.t_mainFrequency;
		}break;
		case YSX_SOUND_DETECTION:{
		    return g_enviro_struct.sound_detection;
		}break;
		case YSX_DETECT_ALARM_TIMER:{
		    return g_enviro_struct.detection_timer;
		}break;
		case YSX_RECORD_QUALITY:{
			return g_enviro_struct.record_quality;
		}break;
		case YSX_ALARM_RECORD_TIME:{
			return g_enviro_struct.alarm_record_time;
		}break;		
	 
	}
	return -1;
}

void SetYSXCtl(int iType, int iValue)
{
	if(GetYSXCtl(iType) == iValue)
		return ;

	dictionary * ini;
	//pr_dbg("var:(%d,%d)\n", iType, iValue);
	char str[25];
	sprintf(str, "%d", iValue);

	char pline[128];
	memset(pline, '\0', sizeof(pline));
	switch(iType)
	{
		case YSX_CAMERA_MOVE:
#ifdef PATRACK
			if (iValue == true)
				camera_moving = time(NULL);
			else
				camera_moving = false;
#else
			camera_moving = time(NULL);
#endif
			return ;
		case YSX_C_EN:
		{
			g_enviro_struct.camera_en = iValue;

			sprintf(pline, "%s:%s", "ysx", "camera_en");
			if ((g_enviro_struct.camera_en == CAM_CLARM_OFF) ||
				(g_enviro_struct.camera_en == CAM_CLARM_ON_OFF))
			{
				LOG("close camera\n");
				LedEnable(0);
			}
			else if ((g_enviro_struct.camera_en == CAM_CLARM_ON) ||
				(g_enviro_struct.camera_en == CAM_CLARM_OFF_ON))
			{
				/*motion.c*/
				LOG("start camera\n");
				LedEnable(GetYSXCtl(YSX_LIGHT_EN));
			}
			else{
				pr_error("in camera_en %d\n", g_enviro_struct.camera_en);
			}
		}
		break;

		case YSX_IR_MODE:
		{
			if(g_enviro_struct.ir_mode == iValue)
				return ;
			sprintf(pline, "%s:%s", "ysx", "ir_mode");
			g_enviro_struct.ir_mode = iValue;
			LOG("ir mode change to %d\n",iValue);
			set_ir_mode(iValue);
		}
		break;


		case YSX_IR_MODE_B_YSX_C_EN:
		{
			sprintf(pline, "%s:%s", "bak", "ir_bak");
		}break;
		case YSX_MIC_EN:
		{
			g_enviro_struct.mic_en = iValue;
			sprintf(pline, "%s:%s", "ysx", "mic_en");
		}
		break;
		case YSX_VINFO_EN:
		{
			g_enviro_struct.t_volinfoEn = iValue;
			sprintf(pline, "%s:%s", "ysx", "voiceinfo_en");
			if(iValue==1){
				if(access("/tmp/play_prompt",F_OK)==0)
					AMCSystemCmd("rm -fr /tmp/play_prompt");	
				
			}else{
				if(access("/tmp/play_prompt",F_OK)!=0)
					AMCSystemCmd("touch /tmp/play_prompt");	
			}
		}
		break;
		case YSX_LIGHT_EN:
		{
			if(g_enviro_struct.light_en != iValue)
			{
				g_enviro_struct.light_en = iValue;
				sprintf(pline, "%s:%s", "ysx", "light_en");
				LedEnable(iValue);
			}
			else
				return ;
		}
		break;
		case YSX_VOL:
		{
			g_enviro_struct.spk_vol = iValue;
			sprintf(pline, "%s:%s", "ysx", "vol");
			LOG("vol iValue %d\n", g_enviro_struct.spk_vol);
			QCamAudioOutputSetVolume(iValue);
		}
		break;
		case YSX_MOTION: //移动/人形侦测
		{
			sprintf(pline, "%s:%s", "motion", "enable");
			g_enviro_struct.motion_en = iValue;
			LOG("motion iValue %d , %d\n", iValue,g_enviro_struct.motion_en);
		}
		break;
		case YSX_MOTION_B_YSX_C_EN:
		{
			sprintf(pline, "%s:%s", "bak", "dtc_bak");
		}break;
		case YSX_SHARE:
		{
			sprintf(pline, "%s:%s", "ysx", "share_en");
			g_enviro_struct.share_en = iValue;
			LOG("share_en iValue %d , %d\n", iValue,g_enviro_struct.share_en);

		}
		break;
		case YSX_VQUALITY:
		{
			sprintf(pline, "%s:%s", "ysx", "video_quality");
			g_enviro_struct.vc_default = iValue;
			//QCamVideoInput_SetIFrame(video_channel_get(iValue));
			LOG("vc_default iValue %d , %d\n", iValue,g_enviro_struct.vc_default);

		}
		break;
		case YSX_SPK_EN:
		{
			sprintf(pline, "%s:%s", "ysx", "spk_en");
			g_enviro_struct.spk_en = iValue;
			LOG("spk_en iValue %d , %d\n", iValue,g_enviro_struct.spk_en);
		}
		case YSX_INVERSION:
		{
			sprintf(pline, "%s:%s", "ysx", "video_mirrorflip");
			g_enviro_struct.vc_mf = iValue;
			
			QCamVideoInput_SetInversion(iValue);

			LOG("vc_mf iValue %d , %d\n", iValue,g_enviro_struct.vc_mf);
		}
		break;
		case YSX_MD_SENSE: //移动侦测灵敏度
		{
			sprintf(pline, "%s:%s", "motion", "sense");
			g_enviro_struct.t_sense = iValue;
			LOG("sense iValue %d , %d\n", iValue,g_enviro_struct.t_sense);
		}
		break;
		case YSX_MD_TYPE: //移动侦测类型
		{
            sprintf(pline, "%s:%s", "motion", "type");
			g_enviro_struct.motion_type= iValue;
			printf("motion_type iValue %d , %d\n", iValue, g_enviro_struct.motion_type);
		}
		break;
		/*
		case YSX_MD_REGION_BIT:
		{
			sprintf(pline, "%s:%s", "ysx", "region_bit");
			g_enviro_struct.region_bit= iValue;
			printf("region_bit iValue %d , %d\n", iValue, g_enviro_struct.region_bit);
		}
		break;
        */
#ifdef PATRACK
		case YSX_AUTOTURN:
		{
			sprintf(pline, "%s:%s", "ysx", "autoturn");
			g_enviro_struct.ptz_autoturn_flag = iValue;
			LOG("autoturn iValue %d , %d\n", iValue,g_enviro_struct.ptz_autoturn_flag);
		}break;
		case YSX_ATRACK_EN: //移动追踪使能
		{
			sprintf(pline, "%s:%s", "ysx", "auto_track");
			g_enviro_struct.atrack_en = iValue;
//			LOG("autoturn iValue %d , %d\n", iValue,g_enviro_struct.atrack_en);
		}break;
		case YSX_PTRACK_EN:
		{
			sprintf(pline, "%s:%s", "ysx", "point_track");
			g_enviro_struct.ptrack_en = iValue;
//			LOG("autoturn iValue %d , %d\n", iValue,g_enviro_struct.ptrack_en);
		}break;
		case YSX_PTRACK_TT:
		{
			sprintf(pline, "%s:%s", "ptrack_time", "pt_times");
			g_enviro_struct.ptrack_tt = iValue;
		}break;
		case YSX_PTRACK_TS:
		{
			sprintf(pline, "%s:%s", "ptrack_time", "pt_start");
			g_enviro_struct.ptrack_ts = iValue;
		}break;
		case YSX_PTRACK_TE:
		{
			sprintf(pline, "%s:%s", "ptrack_time", "pt_end");
			g_enviro_struct.ptrack_te = iValue;
		}break;

#endif

#ifdef CONFIG_TRACK_DRAWRECT
		case YSX_MOTION_OSD:
		{
			sprintf(pline, "%s:%s", "ysx", "osd_en");
			g_enviro_struct.mosd_en = iValue;
		}break;
#endif
        case YSX_CLOUD_ENV:
        {
			sprintf(pline, "%s:%s", "ysx", "server_env");
			g_enviro_struct.server_env = iValue;
            
        }break;
		case YSX_DEV_LOGIN_TYPE:
		{
			sprintf(pline, "%s:%s", "ysx", "login_type");
			g_enviro_struct.login_type = iValue;
			if(access(RELAY_INVAILD_F,F_OK) == 0)
				remove(RELAY_INVAILD_F);
			pr_info("login_type %d\n", g_enviro_struct.login_type);
		}break;
#ifdef IOTYPE_USER_IPCAM_GET_ALERT_PROMPT_RESP
		case YSX_DEV_ALERT:
		{
			sprintf(pline, "%s:%s", "ysx", "dev_alert");
			g_enviro_struct.dev_alert = iValue;
		}break;
#endif

#ifdef CAM_ALARM_ON
		case YSX_CAM_TIME_ALARM_DAY:
		{
			sprintf(pline, "%s:%s", "ysx", "time_alarm_day");
			g_enviro_struct.time_alarm_day = iValue;
		}break;
		case YSX_CAM_TIME_ALARM_TS:
		{
			sprintf(pline, "%s:%s", "ysx", "time_alarm_ts");
			g_enviro_struct.time_alarm_ts = iValue;
		}break;
		case YSX_CAM_TIME_ALARM_TE:
		{
			sprintf(pline, "%s:%s", "ysx", "time_alarm_te");
			g_enviro_struct.time_alarm_te = iValue;
		}break;
#endif
#ifdef IVS_SENSE
		case YSX_IVS_SENSE:
		{
		    EMGCY_LOG("set IVS sense: %d\n", iValue);
			sprintf(pline, "%s:%s", "ysx", "sensitivity");
			g_enviro_struct.sensitivity = iValue;
		}break;
#endif
		case YSX_STORAGE_RESOLUTION:
		{
		    EMGCY_LOG("set storage resolution: %d\n", iValue);
			sprintf(pline, "%s:%s", "ysx", "storage_resolution");
			g_enviro_struct.storage_resolution = iValue;
			g_enviro_struct.resolution_status = YSX_RESOLUTION_SET;
		}break;
		case YSX_AUTO_BITRATE:
		{
		    EMGCY_LOG("set auto bitrate: %d\n", iValue);
			sprintf(pline, "%s:%s", "ysx", "auto_bitrate");
			g_enviro_struct.auto_bitrate = iValue;
		}break;
	
		case YSX_FILL_LED_MODE:
		{
		   	sprintf(pline, "%s:%s", "ysx", "fill_mode");
			g_enviro_struct.t_fillMode= iValue;
		}
	   	break;
		case YSX_FILL_LED_OPEN:
		{
    	   	sprintf(pline, "%s:%s", "ysx", "fill_open");
    		g_enviro_struct.t_fillOpen= iValue;
		}
	   	break;
		case YSX_AI_MODE:
		{
    	   	sprintf(pline, "%s:%s", "ysx", "aiopen");
    		g_enviro_struct.t_aiMode= iValue;		
		}
	   	break;	
		case YSX_MAINS_FREQUENCY:
		{
    	   	sprintf(pline, "%s:%s", "ysx", "main_frequency");
    		g_enviro_struct.t_mainFrequency= iValue;		
		}
	   	break;	
	   	case YSX_SOUND_DETECTION: //声音侦测
	   	{
            sprintf(pline, "%s:%s", "ysx", "sound_detection");
			g_enviro_struct.sound_detection= iValue;
			LOG("sound_detection iValue %d , %d\n", iValue, g_enviro_struct.sound_detection);
	   	}
	   	break;
	   	case YSX_DETECT_ALARM_TIMER: //侦测告警录像时长
	   	{
            sprintf(pline, "%s:%s", "ysx", "detection_timer");
			g_enviro_struct.detection_timer= iValue;
			LOG("detection_timer iValue %d , %d\n", iValue, g_enviro_struct.detection_timer);
	   	}
		break;
		case YSX_RECORD_QUALITY:
		{
			sprintf(pline, "%s:%s", "ysx", "record_quality");
			g_enviro_struct.record_quality = iValue;
			LOG("record_quality %d\n",iValue);
		}
		break;
		case YSX_ALARM_RECORD_TIME:
		{
			sprintf(pline, "%s:%s", "ysx", "alarm_record_time");
			g_enviro_struct.alarm_record_time = iValue;
			LOG("alarm_record_time %d\n",iValue);
		}
		break;		
		default:
			break;
			
    }
		
	ini = iniparser_load(DEV_CONF);
	dictionary_set(ini, pline, str);
	inisync_file(ini, DEV_CONF);
	iniparser_freedict(ini);
}


//dst_data 目标保存内存
int GetYSXCtlString(int iType, char * dst_data, int dst_data_len)
{
	dictionary * ini;
    ini = iniparser_load(DEV_CONF);
	if(ini == NULL ){
		LOG("GetYSXCtlString Can not load %s\n",DEV_CONF);
		return -1;
	}

	char pline[128];

	switch(iType)
	{
		case YSX_RECORD_MODE:
		{
			const char *record_mode;
			sprintf(pline, "%s:%s", "ysx", "record_mode");
			record_mode = iniparser_getstring(ini, pline, RECORD_MODE_DEFAULT);
			strncpy(dst_data, record_mode, dst_data_len-1);	
		}
		break;
		case YSX_DETECT_REGION:
		{
            sprintf(pline, "%s:%s", "motion", "detect_region");
            strncpy(dst_data, iniparser_getstring(ini, pline, DETECT_REGION_DEFAULT), dst_data_len-1);	
		}
		break;
		
		default:
		break;
	}

	iniparser_freedict(ini);
	
	return 0;
}


void SetYSXCtlString( int iType, char *data, int data_len)
{
	//if(GetYSXCtl(iType) == iValue)
	//	return ;
	dictionary * ini;
	//pr_dbg("var:(%d,%d)\n", iType, iValue);

	char pline[128];
	memset(pline, '\0', sizeof(pline));

	switch(iType)
	{
		case YSX_RECORD_MODE:{
			sprintf(pline, "%s:%s", "ysx", "record_mode");
		}
		break;	
		case YSX_DETECT_REGION:
		{
            sprintf(pline, "%s:%s", "motion", "detect_region");
		}
		break;

		default:
		break;
	}
	
	ini = iniparser_load(DEV_CONF);
	dictionary_set(ini, pline, data);
	inisync_file(ini, DEV_CONF);
	iniparser_freedict(ini);

	return ;
}



#ifdef PATRACK
int Set_Point_to_conf(int iType, int iindex, int iValue)
{
	// pr_dbg(" var(%d,%d,%d)\n", iType, iindex, iValue);
	dictionary * ini;
	int i_ret = 0;
	char str[25];
	CLEAR(str);
	sprintf(str, "%d", iValue);
	char pline[128];
	CLEAR(pline);
	switch(iType)
	{

		case YSX_POINT_CPOINT_H:
		{
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "hori", iindex);
		}break;
		case YSX_POINT_CPOINT_V:
		{
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "vert", iindex);
		}break;
		case YSX_POINT_TRACK_H:
		{
			sprintf(pline, "%s:%s_%d", "ptz_ptrack", "hori", iindex);
		}break;
		case YSX_POINT_TRACK_V:
		{
			sprintf(pline, "%s:%s_%d", "ptz_ptrack", "vert", iindex);
		}break;
		case YSX_POINT_RECORD_H:
		{
			sprintf(pline, "%s:%s_%d", "ptz_precord", "hori", iindex);
		}break;
		case YSX_POINT_RECORD_V:
		{
			sprintf(pline, "%s:%s_%d", "ptz_precord", "vert", iindex);
		}break;
		case YSX_APOINT_RECORD_H:
		{
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "apoint_h", iindex);
		}break;
		case YSX_APOINT_RECORD_V:
		{
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "apoint_v", iindex);
		}break;
		default:
			pr_error("Can not load %d\n",iType);
			return -2;
			break;
	}
	ini = iniparser_load(PTRACK_CONF);
	if(ini == NULL ){
		pr_error("Can not load %s\n",PTRACK_CONF);
		return -2;
	}
//	if (-1 != iValue)
//		dictionary_set(ini, pline, str);
//	else
//		dictionary_unset(ini, pline);
//	pr_dbg("in %s", pline);

	i_ret = dictionary_set(ini, pline, str);
	if (i_ret != 0)
		pr_error(" i_ret: %d\n", i_ret);
	inisync_file(ini, PTRACK_CONF);
	iniparser_freedict(ini);
	return 0;
}

int Get_Point_from_conf(int iType, int iindex)
{
//	pr_dbg(" var(%d,%d)\n", iType, iindex);
	dictionary * ini;
	char pline[128];
	int i_ret = -1;
	CLEAR(pline);
	switch(iType)
	{
		case YSX_POINT_CPOINT_H:
		{
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "hori", iindex);
		}break;
		case YSX_POINT_CPOINT_V:
		{
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "vert", iindex);
		}break;
		case YSX_POINT_TRACK_H:
		{
			sprintf(pline, "%s:%s_%d", "ptz_ptrack", "hori", iindex);
		}break;
		case YSX_POINT_TRACK_V:
		{
			sprintf(pline, "%s:%s_%d", "ptz_ptrack", "vert", iindex);
		}break;
		case YSX_POINT_RECORD_H:
		{
			i_ret = 0;
			sprintf(pline, "%s:%s_%d", "ptz_precord", "hori", iindex);
		}break;
		case YSX_POINT_RECORD_V:
		{
			i_ret = 0;
			sprintf(pline, "%s:%s_%d", "ptz_precord", "vert", iindex);
		}break;
		case YSX_APOINT_RECORD_H:
		{
			i_ret = -2;
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "apoint_h", iindex);
		}break;
		case YSX_APOINT_RECORD_V:
		{
			i_ret = -2;
			sprintf(pline, "%s:%s_%d", "ptz_cpoint", "apoint_v", iindex);
		}break;
		default:
			pr_error("Can not load %d\n",iType);
			return -2;
			break;
	}
    ini = iniparser_load(PTRACK_CONF);
	if(ini == NULL ) {
		pr_error("Can not load %s\n",PTRACK_CONF);
		return -2;
	}
	i_ret = iniparser_getint(ini, pline, i_ret);
	iniparser_freedict(ini);
	return i_ret;
}
#endif
int Get_cfg_from_conf(int iType)
{
//	pr_dbg(" var(%d,%d)\n", iType, iindex);
	dictionary * ini;
    ini = iniparser_load(DEV_CONF);
	if(ini == NULL ){
		pr_error("Can not load %s\n",DEV_CONF);
		return -1;
	}
	int i_ret = 0;
	char pline[128];
	CLEAR(pline);
	switch(iType)
	{
#ifdef PATRACK
		case YSX_AUTOTURN:
		{
			sprintf(pline, "%s:%s", "ysx", "autoturn");
		}break;
		case YSX_ATRACK_EN:
		{
			sprintf(pline, "%s:%s", "ysx", "auto_track");
		}break;
		case YSX_PTRACK_EN:
		{
			sprintf(pline, "%s:%s", "ysx", "point_track");
		}break;
		case YSX_PTRACK_TT:
		{
			i_ret = 0;
			sprintf(pline, "%s:%s", "ptrack_time", "pt_times");
		}break;
		case YSX_PTRACK_TS:
		{
			i_ret = 25;
			sprintf(pline, "%s:%s", "ptrack_time", "pt_start");
		}break;
		case YSX_PTRACK_TE:
		{
			i_ret = 25;
			sprintf(pline, "%s:%s", "ptrack_time", "pt_end");
		}break;
#endif
		case YSX_MOTION:
		{
			sprintf(pline, "%s:%s", "motion", "enable");
		}break;
		case YSX_MOTION_B_YSX_C_EN:
		{
			i_ret = 0;
			sprintf(pline, "%s:%s", "bak", "dtc_bak");
		}break;
		case YSX_IR_MODE:
		{
			i_ret = 1;
			sprintf(pline, "%s:%s", "ysx", "ir_mode");
		}break;
		case YSX_IR_MODE_B_YSX_C_EN:
		{
			i_ret = 1;
			sprintf(pline, "%s:%s", "bak", "ir_bak");
		}break;
		default:
			break;
	}
	i_ret = iniparser_getint(ini, pline, i_ret);
	iniparser_freedict(ini);
	return i_ret;
}

void SetMotionTime(unsigned int tStart, unsigned int tEnd)
{
	g_enviro_struct.md_tStart= tStart;
	g_enviro_struct.md_tEnd= tEnd;
	char str[25], str1[25];
	sprintf(str, "%d", tStart);
	sprintf(str1, "%d", tEnd);
	dictionary * ini;
	ini = iniparser_load(DEV_CONF);
	char pline[128],pline1[128];
	memset(pline, '\0', sizeof(pline));
	sprintf(pline, "%s:%s", "ysx", "md_start");
	dictionary_set(ini, pline, str);
	memset(pline1, '\0', sizeof(pline1));
	sprintf(pline1, "%s:%s", "ysx", "md_end");
	dictionary_set(ini, pline1, str1);
	inisync_file(ini, DEV_CONF);
	iniparser_freedict(ini);
}


void GetMotionTime(unsigned int *tStart, unsigned int *tEnd)
{
	*tStart = g_enviro_struct.md_tStart;
	*tEnd = g_enviro_struct.md_tEnd;
}

void set_ap_mac_conf(char *val)
{
	FILE *fp;
	fp = fopen(APMAC_CONF,"w");
	if ((NULL == val) || (NULL == fp)){
		pr_error("in \n");
		return;
	}
	fprintf(fp,"%s",val);
	fclose(fp);
	return;
}

void get_ap_mac_conf(char *val)
{
	FILE *fp;
	fp = fopen(APMAC_CONF, "r");
	if ((NULL == val) || (NULL == fp)){
		pr_error("in \n");
		return;
	}
	fread(val,1,MAC_LEN,fp);
	fclose(fp);
	return;
}

int video_channel_get(int video_quality)
{
	if(video_quality < 3){
		return MAIN_CHN;
	}
	else if(video_quality > 3){
		return SECOND_CHN;
	}
	return -1;
}

