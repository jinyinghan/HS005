#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common_env.h"
#include "common_func.h"
#include "network_detection.h"


G_StructEnviroment g_enviro_struct;
G_StructMedia g_media_struct;
G_StructDeviceflag g_flag_struct;


static int guest_online_num = 0;

//return -1 Fail  >=0 is index
int common_add_user(int sid)
{
	int i,iGNo;
    /*保存连接到数组空闲成员中*/
    for (i=0; i< 10; i++)
    {
        if (g_media_struct.g_media_info[i].sessionid == -1)
        {
            iGNo = i;
            break;
        }
    }
    g_media_struct.g_media_info[iGNo].sessionid = sid;
	
    pthread_mutex_lock(&g_enviro_struct.t_Mutex);
    g_enviro_struct.t_online_num++;
    LOG("use coming total[%d] No %d \n", g_enviro_struct.t_online_num, iGNo);

    /*客人超过4个禁止登录 , 防止主人不能登录*/
    guest_online_num ++ ;
    if( guest_online_num > MAX_USER_NUMBER )
    {
        LOG("guest reach max count %d\n",guest_online_num);
        return -1;
    }
	pthread_mutex_unlock(&g_enviro_struct.t_Mutex);
	
	return iGNo;
}


void common_cancel_user(int iGNo)
{
    /*关闭video传输*/
    pthread_mutex_lock(&g_media_struct.g_media_info[iGNo].va_Mutex);
    g_media_struct.g_media_info[iGNo].is_VA = 0;
    pthread_mutex_unlock(&g_media_struct.g_media_info[iGNo].va_Mutex);
    //LOG("2use leave total[%d] No %d\n", g_enviro_struct.t_online_num, iGNo);

    /*关闭video传输*/
    pthread_mutex_lock(&g_media_struct.g_media_info[iGNo].aa_Mutex);
    g_media_struct.g_media_info[iGNo].is_AA = 0;
    pthread_mutex_unlock(&g_media_struct.g_media_info[iGNo].aa_Mutex);
    //LOG("3use leave total[%d] No %d \n", g_enviro_struct.t_online_num, iGNo);


    /*关闭video传输*/
    pthread_mutex_lock(&g_media_struct.sa_Mutex);
    g_media_struct.is_SA = 0;
    pthread_mutex_unlock(&g_media_struct.sa_Mutex);
    //LOG("4use leave total[%d] No %d\n", g_enviro_struct.t_online_num, iGNo);

    //LOG("stop sd play\n");
    /*关闭video传输*/
	/*
    pthread_mutex_lock(&g_media_struct.pa_Mutex);
    g_media_struct.playch = -1;
    g_media_struct.now_pavIndex = -1;
    g_media_struct.bPlayStatus = PLAY_STOP;
    pthread_mutex_unlock(&g_media_struct.pa_Mutex);
	*/
	stop_sd_playback(&g_media_struct.g_media_info[iGNo].pb_info);

    /*删除连接记录*/
    g_media_struct.g_media_info[iGNo].sessionid = -1;
    pthread_mutex_lock(&g_enviro_struct.t_Mutex);
    if (g_enviro_struct.t_online_num > 0)
        g_enviro_struct.t_online_num--;
	
    guest_online_num --;
    if(guest_online_num < 0)
        guest_online_num = 0;
    pthread_mutex_unlock(&g_enviro_struct.t_Mutex);
    LOG("use leave total[%d] No %d \n", g_enviro_struct.t_online_num, iGNo);

	return ;
}


void get_uid_from_flash(char *uid)
{
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));

    FILE *read_fp = popen("nvram_get UID", "r");
    if (read_fp != NULL)
    {
            fread(buffer, sizeof(char), MAX_UID_SIZE, read_fp);
            pclose(read_fp);

            if (strlen(buffer) != 0)
            {
                    strcpy(uid,buffer);
            }
    }
    return;
}


void DeInitAVInfo()
{
    memset(&g_enviro_struct, 0, sizeof(G_StructEnviroment));
    memset(&g_media_struct, 0, sizeof(G_StructMedia));
}


void  get_fw_version(char*iversion)
{

	FILE *fversion = fopen(VERSIONCONFIG, "rb");
	if(!fversion)
	{
		strcpy(iversion,"020000");
	}
	fscanf(fversion, "%s",iversion);
	fclose(fversion);
}


int get_timezone()
{
    int timezone = 0;
    time_t t1, t2 ;
    struct tm *tm_local, *tm_utc;

    time(&t1);
    t2 = t1;
    tm_local = localtime(&t1);
    t1 = mktime(tm_local) ;

    tm_utc = gmtime(&t2);
    t2 = mktime(tm_utc);

    timezone = (t1 - t2) / 60;

    return timezone;
}


int GetEnviroStruct()
{
        InitDeviceSetting();
        char *uid;

        uid = g_enviro_struct.t_uid;

        //CLEAR(uid);
        memset(g_enviro_struct.t_uid,0,sizeof(g_enviro_struct.t_uid));
		
        get_uid_from_flash(uid);
        if (strlen(uid) < MAX_UID_SIZE) {
                LOG("#### UID is not burned \n");
                return -1;
        }
        // ysx_mqtt_init(uid);


        g_enviro_struct.t_online 	 = 0;
        g_enviro_struct.t_online_num = 0;
        g_enviro_struct.t_sdpathlen  = 0;
        g_enviro_struct.t_sdtotal    = 0;
        g_enviro_struct.t_sdfree     = 0;
        g_enviro_struct.t_zone 		 = 0;
        g_enviro_struct.t_ledstatus  = 0;
		get_fw_version(g_enviro_struct.t_version);

        LOG("uid[%s] version[%s]\n",uid,g_enviro_struct.t_version);
        pthread_mutex_init(&g_enviro_struct.t_Mutex,NULL);			/*环境变量设置线程锁*/
        strcpy((char *)g_enviro_struct.t_name, "admin");			/*配置登录名和密码*/

        int timezone = get_timezone();
        g_enviro_struct.t_zone 	 = timezone / 60;
        g_enviro_struct.t_zone_m = timezone % 60; 
        if (access("/tmp/sd_ok", F_OK) == 0) {
                get_sd_path(g_enviro_struct.t_sdpath, sizeof(g_enviro_struct.t_sdpath));
                LOG("sd path: '%s'\n", g_enviro_struct.t_sdpath);
        }
        LOG("timezone=%d ,tz_hour : %d, tz_minute : %d\n",timezone,g_enviro_struct.t_zone,g_enviro_struct.t_zone_m);

		
		
        return 0;
}

     

int get_online_num(void)
{
    int i_cnt = 0, i_ret = 0;
    for(i_cnt=0; i_cnt<6; i_cnt++) {
        if (g_media_struct.g_media_info[i_cnt].is_VA != 0) {
                i_ret++;
        }
    }
    //    pr_dbg("online num %d\n", i_ret);
    g_media_struct.g_isVideoNum = i_ret;
    return i_ret;
}


int dev_network_cb(NETWORK_MODE_E net_mode, NETWORK_STATUS_E net_status)
{
        g_enviro_struct.net_mode = net_mode;
        g_enviro_struct.net_status = net_status;
        EMGCY_LOG("@@@ network_mode=%d, network_status=%d @@@\n", net_mode, net_status);

        if(g_enviro_struct.wifi_switch_st == 0){
                if(g_enviro_struct.net_mode == WIRELESS_MODE){
                        if((g_enviro_struct.net_status == NET_CONNECT_LAN)||(g_enviro_struct.net_status == NET_CONNECT_WAN)||(g_enviro_struct.net_status == NET_GET_IPADDR)){	
                                g_enviro_struct.wifi_switch_st = 2;
                                AMCSystemCmd("sample_play 16000 /usr/notify/wifi_set.pcm"); 
                        }
                }else if(g_enviro_struct.net_mode == WIRED_MODE){
                        if((g_enviro_struct.net_status == NET_CONNECT_LAN)||(g_enviro_struct.net_status == NET_CONNECT_WAN)||(g_enviro_struct.net_status == NET_GET_IPADDR)){
                                g_enviro_struct.wifi_switch_st = 1;
                                AMCSystemCmd("sample_play 16000 /usr/notify/wifi_set.pcm"); 
                        }
                }
        }

        return 0;
}

int iwlist_wifi_scan(void *system_data)
{
    #define WIFI_SCAN_AP "/tmp/qWIFI_SCAN.conf"
    
    char sTemp[260];
    char sRet[260];
    scan_ap_t *ap_list = (scan_ap_t*)system_data; 
    
    //执行shell命令，获取一手数据到指定位置
    memset(sTemp,0,sizeof(sTemp));
    sprintf(sTemp, "iwlist wlan0 scanning > %s", WIFI_SCAN_AP);
    system(sTemp);
    memset(sTemp,0,sizeof(sTemp));
    FILE *fp = fopen(WIFI_SCAN_AP, "rb");
    if (!fp)
        return false;
        
    int devicecount = 0, ap_scan_cnt = 0;
    bool ap_info_valid_flag = false;
    int first_vaild = 0;
   
    /*
        {"result":"ok","apnum":24,"endflag":0,"aplist":
        [{"ssid":"YSX-2.4G-008","signal":0,"encrypt":1}]}
    */
    while(1)
    {
        fgets(sTemp, sizeof(sTemp), fp); // get one line
        if(strstr(sTemp, "Cell"))
        {
            if (first_vaild > 0)
            {
                ap_info_valid_flag = true;
            }
            first_vaild ++;
        }
		    
        if ((ap_info_valid_flag) ||
            ((feof(fp)) && (first_vaild > 0)))
        {
            devicecount++;

            if (!ap_info_valid_flag)
            	break;
            ap_info_valid_flag = false;
            
        }
        //文件结束则返回非零值，但为什么放中间呢？
        if (feof(fp) || (devicecount > sizeof(ap_list->aplist)/sizeof(aplist_type)) )
            break;

        //因为是行解析，故解析顺序：SSID->加密方式->信号强度
        //SSID解析
        if(strstr(sTemp, "ESSID:\"")) 
        {
            memset(sRet,0,sizeof(sRet));
            sscanf(sTemp, "%*[^:]:\"%[^\"]\"",sRet);
            if (strlen(sRet)) 
            {
                if(strlen(sRet) >= 32)
                {
                    printf("\n ssid error...\n");
                    //YSX_LOG(LOG_APP_ERROR, "ssid:%s\n", sRet);
                    continue;
                }
                else
                {
                    strncpy(ap_list->aplist[devicecount].ssid, sRet, sizeof(ap_list->aplist[0].ssid));
                }
           }
           continue;
        }
        
        //enctype加密方式解析
        if(strstr(sTemp, "Encryption key:"))
        {
            memset(sRet,0,sizeof(sRet));
            sscanf(sTemp, "%*[^:]:%s%*[^]",sRet);
            if(!strcmp(sRet, "off"))
            {
                ap_list->aplist[devicecount].encrypt = 0;
            }
            else //if(strcmp(sRet, "on"))
            {
                //可能会出现误判（wep加密方式）！
                ap_list->aplist[devicecount].encrypt = 2;		//WEP加密方式
                continue;
            }
        }
                
        if(strstr(sTemp, "Group Cipher"))
        {
            memset(sRet,0,sizeof(sRet));
            sscanf(sTemp, "%*[^:]%*[^ ]%s",sRet);
            ap_list->aplist[devicecount].encrypt = 1;		//WPA加密方式				            
            continue;
        }
        
        //Signall信号强度解析
        if(strstr(sTemp, "Signal level=") ) 
        {
            memset(sRet,0,sizeof(sRet));
            sscanf(sTemp, "%*[^S]%*[^=]=%[^/]",sRet);
            // 89/100 ==> -11db
            ap_list->aplist[devicecount].signal = strtol(sRet,0,10) - 100;

            continue;
        }

    }
    fclose(fp);
	remove(WIFI_SCAN_AP);
    strncpy(ap_list->result,"ok",3);
    ap_list->apnum = devicecount;

    return true;
    
}


