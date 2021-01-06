/*********************************************************************************************/
//+++++++++++++  BACK UP  FROM  MAIN.c
//+++++++++++++  BACK UP  FROM  MAIN.c
//+++++++++++++  BACK UP  FROM  MAIN.c
void* ripple_thread(void *arg){

        int ret = -1;
        int gain = 0;

        int register_value = 0;

        while(g_enviro_struct._loop){

                ret = IMP_ISP_Tuning_GetTotalGain(&gain); 
                if(ret != 0){

                        printf("error to get Gain\n");
                        sleep(1);
                        continue;
                }
                printf(">>===========================>> gain value %d %d\n",gain, 0xf & (gain >> 0x8));



                if(((0xf & gain >> 0x8) == 0x1) && (register_value != 0x2)){

                        ret = IMP_ISP_SetSensorRegister(0x3009,0x02);

                        if(ret != 0){
                                LOG("write sensor register err\n");
                                printf("write sensor register err\n");

                        }
                        printf("operation 1\n");

                }else if(((0xf & gain >> 0x8) == 0x8) && (register_value != 0x12)){

                        ret = IMP_ISP_SetSensorRegister(0x3009,0x12);
                        if(ret != 0){
                                LOG("write sensor register err\n");
                                printf("write sensor register err\n");	

                        }


                        printf("operation 2\n");
                }
                ret = IMP_ISP_GetSensorRegister(0x3009,&register_value);
                if(!ret)
                        printf("get sensor register value %d\n",register_value);


                sleep(2);





        }

}


#define  CHAR_SPACE	' '
#define  is_a_space(c)    (c  ==  CHAR_SPACE )

int read_gateway_ip_to_buffer(char * ip_buffer, const char * net_interface)
{
    FILE *gateway_ip_ponit = NULL;
    char line[128];
    char *s1 =  NULL;
    char *s2 = NULL;
    char *s3 =NULL;
    int i = 0;

    AMCSystemCmd("route  -n > /var/gateway.txt");
    gateway_ip_ponit = fopen("/var/gateway.txt", "r");

    if (gateway_ip_ponit == NULL) {
            printf(" Open dns_gateway file failed \n");
            return -1;
    }

    while(!feof(gateway_ip_ponit))
    {
            fgets((char *)line, 128, gateway_ip_ponit);

            if (NULL == (s1 = strstr(line, "0.0.0.0"))) {
                    memset((void *)line , 0 , 128);
                    continue;
            }

            if (NULL == (s2 = strstr(line, "UG"))) {
                    memset((void *)line , 0 , 128);
                    continue;
            }

            if (NULL == (s3 = strstr(line, net_interface))) {
                    memset((void *)line , 0 , 128);
                    continue;
            }

            s1 = s1 + strlen("0.0.0.0");
            for (s1++; is_a_space(*s1); s1++);

            while((*s1)!=' ')
            {
                    ip_buffer[i] = *s1;
                    i ++;
                    s1 ++;
            }
            break;
    }

    fclose(gateway_ip_ponit);
    return 0;
}


enum {
        NET_STATUS_ONLINE       = 0x01, 
        NET_STATUS_OFFLINE      = 0x02,
        NET_STATUS_ETH_BAD      = 0x03,
        NET_STATUS_WLAN_BAD     = 0x04,
};


#if 0
static unsigned char *IPCam_Hash256(unsigned char* in)
{
        SHA256_CTX ctx;
        unsigned char *out = malloc(SHA256_DIGEST_LENGTH);
        SHA256((unsigned char *)in, strlen(in), out);

        SHA256_Init(&ctx);
        SHA256_Update(&ctx, in, strlen(in));
        SHA256_Final(out, &ctx);
        OPENSSL_cleanse(&ctx, sizeof(ctx));

        return out;
}

static int IPCam_strToHex(unsigned char *ch,unsigned char *hex)
{
        if(ch == NULL)
                return -1;

        int i = 0;
        for(i = 0; i < 32; i++){
                sprintf(hex,"%02x", ch[i]);
                hex+=2;
        }
        if(hex == NULL)
                return -1;
        return 0;
}

static void Make_random_number(int *random_number)
{
        unsigned char *hash_data = IPCam_Hash256(g_enviro_struct.t_uid);
        unsigned char hex_data[128];
        memset(hex_data, 0, 128);	
        IPCam_strToHex(hash_data, hex_data);
        if(hash_data){
                free(hash_data);
                hash_data = NULL;
        }

        EMGCY_LOG("!!!!!!!!!!!!!!!!!!!!!!!hex_data=%s!!!!!!!!!!!!!!!!!!!!!!!\n", hex_data);
        unsigned int hex;
        sscanf(hex_data, "%8x", &hex);

        EMGCY_LOG("!!!!!!!!!!!!!!!!!!!!!!!hex=%x!!!!!!!!!!!!!!!!!!!!!!!\n", hex);
        srand(hex);		//void srand(unsigned int seed);
        *random_number = rand()%7200;
        printf("ota sleep time :%d\n",*random_number);
        EMGCY_LOG("!!!!!!!!!!!!!!!!!!!!!!!random=%d!!!!!!!!!!!!!!!!!!!!!!!\n", *random_number);
}


void *thread_fw(void *arg)
{
        time_t timep;
        struct tm *sync_start_time;
        char cmd[32];
        static int random = 0;
        Make_random_number(&random);

        while(g_enviro_struct._loop){
                time(&timep);
                time(NULL);
                //printf("+++++++++++++++++++++g_enviro_struct.server_env:%d\n",g_enviro_struct.server_env);
                sync_start_time = localtime(&timep);
                if(access("/tmp/update_now",F_OK) == 0 || (sync_start_time->tm_hour == 2 && sync_start_time->tm_min == 0 && sync_start_time->tm_sec <= 5)){
                        remove("/tmp/update_now");
                        memset(cmd, 0, sizeof(cmd));
                        printf("now goto ota but sleep time :%d\n",random);
                        ysx_setTimer(random, 0);
                        //sprintf(cmd, "fwdownload %d &", g_enviro_struct.server_env);
                        sprintf(cmd, "fwdownload %d &", 1);
                        LOG("### %s ###\n",cmd);
                        AMCSystemCmd(cmd);
                }
                ysx_setTimer(5, 0);
        }
        pthread_exit((void*)0);
}

void *thread_memshow(void *arg)
{	
        unsigned char first_play = 1;
        int ret = 0;
        unsigned int LoginInfo = 0;

        while(g_enviro_struct._loop){
                if(!g_enviro_struct.standalone_mode){
                        if (first_play) {
                                if(file_exist("/tmp/resolv.conf")){
                                        //if(GetYSXCtl(YSX_VINFO_EN))
                                        //     AMCSystemCmd("sample_play 16000 /usr/notify/wifi_set.pcm"); 
                                        first_play = 0;	
                                }
                        }
                }

                mem_show();
                ysx_setTimer(3, 0);
				
        
        }

        pthread_exit((void*)0);
}


void LocalToUTC_TUTK(STimeDay *sT)
{
        struct  timeval    tv;
        struct  timezone   tz;
        gettimeofday(&tv,&tz);

        struct tm *p= gmtime(&tv.tv_sec);
        sT->year = (1900+p->tm_year);
        sT->month = (1+p->tm_mon);
        sT->day = p->tm_mday;
        sT->wday = p->tm_wday;
        sT->hour = p->tm_hour;
        sT->minute = p->tm_min;
        sT->second = p->tm_sec;
}

int update_curwifi_mac(bool is_wifi);
#define AP_ALIVE_DTC_DELAY					0x3c//power consumption, 5*200ms
#define AP_SSID_DTC_DELAY					0x78



int extract_router_gw(char *ip)
{
    int i_ret = -1;
    FILE *fp = NULL;
    if (access("/tmp/resolv.conf",F_OK) == 0)
            fp = fopen("/tmp/route.conf","r");
    if(fp == NULL){
            pr_error("in \n");
            return i_ret;
    }
    char buffer[128] = {0};
    fread(buffer,1,sizeof(buffer),fp);
    fclose(fp);

    char *match = strstr(buffer,"0.0.0.0");
    if(match){

            match += strlen("0.0.0.0")+0x9;
            char *end = strstr(match," ");
            memcpy(ip,match,end-match);
            i_ret = 0;
    }
    return i_ret;
}

#if 1
//example:nameserver 192.168.31.1 # wlan0
int extract_router_ip(char *ip)
{
    FILE *fp = fopen("/tmp/resolv.conf","r");
    if(fp)
    {
            char buffer[128] = {0};
            fread(buffer,1,sizeof(buffer),fp);
            fclose(fp);

            char *match = strstr(buffer,"nameserver");
            if(match)
            {
                    match += strlen("nameserver")+1;
                    char *end = strstr(match,"#");
                    memcpy(ip,match,end-match);
                    return 0;
            }
    }
    return -1;
}
#endif
int network_is_ok(void)
{
    FILE *fp = fopen("/tmp/resolv.conf","r");
    if(fp){
            char buffer[128] = {0};
            fread(buffer,1,sizeof(buffer),fp);
            fclose(fp);
            if(strstr(buffer,"nameserver")){
                    return 0;
            }
    }
    return -1;
}
static char router_gw[20] = {0};
static char router_ip[20] = {0};

int internet_is_ok(void)
{
    //0: unlink, 1: link route, 2: link internet
    char sys_cmd[64];
    int net_flag = 2, ret = 0;
    //gw_nameserver_flag 0: default 1: gateway work 2: nameserver work
    static int gw_nameserver_flag = 0;
    if(net_flag) {
        CLEAR(sys_cmd);
        snprintf(sys_cmd,sizeof(sys_cmd),"ping -q -c 1 -w 2 %s >/dev/null", "8.8.8.8");
        ret = AMCSystemCmd(sys_cmd);
        if (ret != 0)
                net_flag --;
        else
                return net_flag;
    }
    if(!strlen(router_gw)) {
        // 提取路由gateway
        if(extract_router_gw(router_gw) < 0) {
                return net_flag;
        }
        gw_nameserver_flag = 0;
    }
    if(!strlen(router_ip)) {
        // 提取路由IP
        if(extract_router_ip((router_ip)) < 0) {
                return net_flag;
        }
        gw_nameserver_flag = 0;
    }
    //	pr_dbg("gateway %s, nameservr %s\n", router_gw, router_ip);
    switch(gw_nameserver_flag) {
        case 0:
        case 1: {
	                //            ping route nameserver
	                CLEAR(sys_cmd);
	                snprintf(sys_cmd,sizeof(sys_cmd),"ping -q -c 1 -w 2 %s >/dev/null", router_gw);
	                ret = AMCSystemCmd(sys_cmd);
	                if(ret == 0) {
	                        gw_nameserver_flag = 1;
	                        break;
	                }else {
	                        pr_info("ip:%s, ret:%d, flag: %d\n", router_gw, ret, net_flag);
	                }
	                if(gw_nameserver_flag == 1) {
	                        net_flag --;
	                        break;
	                }
                };
        case 2: {
                    //            ping route nameserver
                    CLEAR(sys_cmd);
                    snprintf(sys_cmd,sizeof(sys_cmd),"ping -q -c 1 -w 2 %s >/dev/null", router_ip);
                    ret = AMCSystemCmd(sys_cmd);
                    if (ret == 0) {
                            gw_nameserver_flag = 2;
                    }else {
                            pr_info("ip:%s, ret:%d, flag: %d\n", router_gw, ret, net_flag);
                            net_flag --;
                    }
                }break;
        default:
                pr_info("not support %d\n", gw_nameserver_flag);
                net_flag = 0;
                break;
    }
    //	pr_dbg("ret: %d\n", net_flag);
    return net_flag;
}


int wireless_route_alive()
{
    static uint32_t dead_cnt = 0;
    char sys_cmd[64];
    int ret = 0;

    if(!strlen(router_gw))
    {
        // 提取路由IP
        ret = extract_router_gw(router_gw);
        if(ret < 0)
        {
                return -1;
        }
    }

    CLEAR(sys_cmd);
    snprintf(sys_cmd,sizeof(sys_cmd),"ping -q -c 1 -w 2 %s >/dev/null",router_gw);
    ret = AMCSystemCmd(sys_cmd);
    //	pr_dbg("ip:%s, ret:%d\n", router_gw, ret);
    if(ret == 0)
        dead_cnt++;
    else
        dead_cnt = 0;

    if(dead_cnt > 15)
    {
        LOG("Can not connect to router ###\n");
        if (GetLedStatus() != EZ_STATUS)
                SetLed(1, EZ_STATUS);
        return 0;
    }
    return 1;
}


/*
 * ??  ??:
 * ??  ??:
 * ????ֵ:
 */
int Wired_Network_check(char *cmd)
{
    char result[64] = {'\0'};
    int ret = 0;

    FILE *fp1 = popen(cmd, "r");
    if (fp1) {
        if (fread(result, 1, sizeof(result), fp1) != 0) {
                ret = 1;
        }
        else {
            pclose(fp1);

            memset(result, 0, 64);
            FILE *fp2 = popen("ps | grep hostapd | grep -v grep", "r");
            if (fread(result, 1, sizeof(result),fp2) != 0)
                    ret = 2;
            else
                    ret = 0;
            pclose(fp2);
        }
        pclose(fp1);
    }
    else
            pr_error("in\n");
    return ret;
}


int check_wifi_ap_alive(char *mac)
{
    static int scan_cnt = 0;
    int scan_cnt_max = AP_ALIVE_DTC_DELAY;
    char sys_cmd[64];
    return 0;
    int ret = -1;
    if ((mac == NULL) || (strcmp(mac,"") == 0)){
        pr_error("in \n");
        return ret;
    }
    if (scan_cnt%scan_cnt_max == 0){
        scan_cnt = 0;
        CLEAR(sys_cmd);
        snprintf(sys_cmd,sizeof(sys_cmd),"iwlist wlan0 scan| grep \"%s\" >/dev/null",mac);
        ret = AMCSystemCmd(sys_cmd);
        if (ret != 0){
            if (AMCSystemCmd("ifconfig |grep wlan0 >/dev/null") != 0)
                if (AMCSystemCmd("ifconfig wlan0 up") != 0){
                        pr_error("in \n");
                }
            pr_error("mac:%s, ret:%d\n", mac, ret);
        }
    }
    scan_cnt ++;
    return ret;
}


int update_curwifi_mac(bool is_wifi)
{
    char sys_cmd[64];
    int ret = -1;
    int wifi_sw = 0;
    char sTemp[64]={0}, sRet[64]={0}, t_apmac[MAC_LEN]={0};
    FILE *fp, *read_fp;

    if (is_wifi){
        fp = fopen(FILE_WPA, "r");
        if (fp == NULL){
                return ret;
        }
        do {
            fgets(sTemp, sizeof(sTemp), fp);
            if (strstr(sTemp, "ssid=\"")){
                CLEAR(sRet);
                sscanf(sTemp, "%*[^=]=\"%[^\"]\"", sRet);
                if (strlen(sRet) <= SSID_LEN) {
                    if (strcmp(g_enviro_struct.t_apssid, sRet)){
                        CLEAR(g_enviro_struct.t_apssid);
                        memcpy(g_enviro_struct.t_apssid, sRet, strlen(sRet));
                        wifi_sw = 1;
                        ret = 0;
                    }
                }
                break;
            }
        } while(!feof(fp));
        fclose(fp);
    }

#if 0//disable ap mac iwlist
    //	pr_dbg("ssid %s, %s, sw %d\n", g_enviro_struct.t_apssid, sRet, wifi_sw);
    if ((strcmp(g_enviro_struct.t_apssid,"")) &&
                ((wifi_sw > 0) || (strcmp(g_enviro_struct.t_apmac,"") == 0))){
        CLEAR(sys_cmd);
        snprintf(sys_cmd,sizeof(sys_cmd),"iwlist wlan0 scan| grep -B 25 \"%s\"", g_enviro_struct.t_apssid);
        read_fp = popen(sys_cmd, "r");
        if (read_fp == NULL){
            if (AMCSystemCmd("ifconfig |grep wlan0 >/dev/null") != 0)
                if (AMCSystemCmd("ifconfig wlan0 up") != 0){
                        pr_error("in \n");
                }
            pr_error("in \n");
            return ret;
        }
        do{
            fgets(sTemp, sizeof(sTemp), read_fp);
            if (strstr(sTemp, "Address:")){
                CLEAR(sRet);
                sscanf(sTemp, "%*[^:]%*[^ ]%s",sRet);
                if (strlen(sRet) <= MAC_LEN){
                    CLEAR(t_apmac);
                    memcpy(t_apmac, sRet, strlen(sRet));
                }
            }
            else if(strstr(sTemp, "ESSID:\"")){
                CLEAR(sRet);
                sscanf(sTemp, "%*[^:]:\"%[^\"]\"",sRet);
                if (strlen(sRet) <= SSID_LEN){
                    if(!strcmp(g_enviro_struct.t_apssid, sRet)){
                        CLEAR(g_enviro_struct.t_apmac);
                        memcpy(g_enviro_struct.t_apmac, t_apmac, strlen(t_apmac));
                        set_ap_mac_conf(g_enviro_struct.t_apmac);
                        //						pr_dbg("mac %s\n", g_enviro_struct.t_apmac);
                        break;
                    }
                }
            }
        }while(!feof(read_fp));
        pclose(read_fp);
        ret = 0;
    }
#endif
        return ret;
}
int update_route_gw(void)
{
    int i_ret = -1;
    remove("/tmp/route.conf");
    if (access("/tmp/resolv.conf",F_OK) != 0)
            return i_ret;
    i_ret = AMCSystemCmd("route -n |grep UG > /tmp/route.conf &");
    return i_ret;
}
void update_route_ip_mac(void)
{
    CLEAR(router_gw);
    CLEAR(router_ip);
}

int heart_beat_packet(int *scon_flag, int *con_flag)
{
    static int route_cnt = 0;
    int i_ret = -1, cnt_max = 5;
    *con_flag = internet_is_ok();
    if (g_enviro_struct.t_online == 1){
        //			if (*con_flag == 2)
        if (g_media_struct.g_isVideoNum > 0){
            //video watching
            if (GetLedStatus() != VIDEO_START_STATUS)
                    i_ret = SetLed(1, VIDEO_START_STATUS);
        }
        else{
            if (GetLedStatus() != ON_LINE_STATUS)
                    i_ret = SetLed(1, ON_LINE_STATUS);
        }
        route_cnt = 0;
    }
    else{
        if (*con_flag == 0){
            route_cnt ++;
            if (route_cnt >= cnt_max){
                route_cnt = 0;
                *scon_flag = -1;
                //				*scon_flag = g_enviro_struct.t_online;
                if (GetLedStatus() != EZ_STATUS)
                        i_ret = SetLed(1, EZ_STATUS);
            }
        }
        else{
            if (g_media_struct.g_isVideoNum > 0){
                //video watching
                if (GetLedStatus() != VIDEO_START_STATUS)
                        i_ret = SetLed(1, VIDEO_START_STATUS);
            }
            else{
                if (GetLedStatus() != EZ_STATUS_ROUTE)
                        i_ret = SetLed(1, EZ_STATUS_ROUTE);
            }
            route_cnt = 0;
        }
    }
    return i_ret;
}

#ifdef CAM_ALARM_ON//programs on-off
static bool wday_is_ok(int wday)
{
        unsigned int day_tmp = 0x1;
        int day = 0;
        if (wday > 0)
                day_tmp = day_tmp << wday;
        else
                day_tmp = day_tmp << 7;
        day = GetYSXCtl(YSX_CAM_TIME_ALARM_DAY);
        if (day & day_tmp)
                return true;
        else
                return false;
}

static bool time_is_ok(int tm_hour, int tm_min)
{
        int time_s = 0, time_e = 0, time_l = 0;
        int invaild_time = 0;
        time_l = tm_hour*60 + tm_min;
        time_s = GetYSXCtl(YSX_CAM_TIME_ALARM_TS);
        time_e = GetYSXCtl(YSX_CAM_TIME_ALARM_TE);
        pr_dbg("time %d, %d, %d\n", time_l, time_s, time_e);
        if(time_s > time_e )	/*跨天*/
        {
                if((time_l >= time_e) &&
                                (time_l < time_s))
                        invaild_time = 1;
                pr_info(" time: %d\n",time_l);
        }else if(time_s < time_e ) /*0-24*/
        {
                if ((time_l < time_s) ||
                                (time_l >= time_e))
                        invaild_time = 1;
                pr_info(" time: %d\n",time_l);
        }else{
                ;/*start == end */
        }
        return (invaild_time == 1)?false:true;
}

void * thread_ysx_sys(void *arg)
{
        //get time
        struct timeval tv;
        struct timespec outtime;
        struct tm *t;
        int cam_en = 0;
        bool alarm_sw = 0;
        time_t tt;
        //wait and check the alarm
        while (1){
                pthread_mutex_lock(&ysx_sys_mutex);
                while (ysx_sys_flag){
                        while (1){
                                gettimeofday(&tv, NULL);
                                outtime.tv_sec = tv.tv_sec + 1;
                                outtime.tv_nsec = tv.tv_usec * 1000;
                                pthread_cond_timedwait(&ysx_sys_cond, &ysx_sys_mutex, &outtime);
                                time(&tt);
                                t = localtime(&tt);
                                pr_dbg("%4d年%02d月%02d日 周 %d %02d:%02d:%02d\n",
                                                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                                                t->tm_wday, t->tm_hour,t->tm_min, t->tm_sec);
                                cam_en = GetYSXCtl(YSX_C_EN);
                                alarm_sw = GetYSXCtl(YSX_CAM_TIME_ALARM_DAY)<<7 > 0? true:false;
                                if (alarm_sw){
                                        if ((cam_en == CAM_CLARM_ON) && (wday_is_ok(t->tm_wday)) && (time_is_ok(t->tm_hour,t->tm_min))){
                                                cam_en = CAM_CLARM_ON_OFF;
                                                break;
                                        }
                                        else if ((cam_en == CAM_CLARM_ON_OFF) && ((!wday_is_ok(t->tm_wday)) || (!time_is_ok(t->tm_hour,t->tm_min)))){
                                                cam_en = CAM_CLARM_ON;
                                                break;
                                        }

                                }
                        }
                        ysx_sys_flag = false;
                }
                ysx_sys_flag = true;
                pthread_mutex_unlock(&ysx_sys_mutex);
                //dev on-off
                SetYSXCtl(YSX_C_EN, cam_en);
        }
}
#endif

static bool fill_light_time_update_flag = false;
void set_fill_light_time_flag(bool en_flag)
{
        //    pr_dbg("in flag %d\n", en_flag);
        fill_light_time_update_flag = en_flag;
}

bool get_fill_light_time_flag(void)
{
        //    pr_dbg("in flag %d\n", light_time_update_flag);
        return fill_light_time_update_flag;
}


#define led_reboot_in  "/tmp/led_fill_open"
void *thread_flight(void *arg)
{
        time_t timep;
        struct tm *sync_start_time;
        int cStart_o,cStart_s, cEnd_o,cEnd_s;
        int i_ret=-1;
        int last_status=-1,fill_status=-1;

        struct timeval tv;
        long int starttime,endtime;
        uint32_t totaltime;
        while(g_enviro_struct._loop){

                i_ret = GetYSXCtl(YSX_C_EN);
                if((i_ret == CAM_CLARM_OFF) || (i_ret == CAM_CLARM_ON_OFF)||(GetYSXCtl(YSX_FILL_LED_MODE)==0)/*||(access("/tmp/sync_time",X_OK) != 0)*/) {
                        sleep(1);
                        // LOG("fill light thread to sleep\n");
                        continue;
                }
#ifdef USE_TIMING_FILL

                time(&timep);
                GetNightTime((int*)&cStart_o,(int*)&cStart_s, (int*)&cEnd_o,(int*)&cEnd_s);
                sync_start_time = localtime(&timep);
                if(access("/tmp/led_fill_open",F_OK)==0){
                        int ret=time_scope_set(cStart_o,cStart_s,cEnd_o,cEnd_s);
                        if(ret>0){
                                AMCSystemCmd("gpio_opt -n 90 -m 1 -v 1");
                                g_enviro_struct.f_light_st=1;
                                LedEnable(0);
                        }

                        remove("/tmp/led_fill_open");
                }


                if((sync_start_time->tm_hour ==cEnd_o)&&(sync_start_time->tm_min ==cEnd_s)&&(sync_start_time->tm_sec<=5))     
                {    
                        AMCSystemCmd("gpio_opt -n 90 -m 1 -v 0");
                        g_enviro_struct.fi_light_st=0;
                        LedEnable(GetYSXCtl(YSX_LIGHT_EN));
                }else if((sync_start_time->tm_hour == cStart_o) && (sync_start_time->tm_min == cStart_s)&&(sync_start_time->tm_sec<=5)){
                        AMCSystemCmd("gpio_opt -n 90 -m 1 -v 1");
                        g_enviro_struct.f_light_st=1;
                        LedEnable(0);
                }    

                sleep(5);
#else

                if(QCamGetModeStatus() == QCAM_MODE_NIGHT && (!get_fill_light_status())){
                        set_fill_light_time_flag(true);
                        // AMCSystemCmd("gpio_opt -n 90 -m 1 -v 1");
                        set_flight_mode(1);
                        set_fill_light_status(1);
                        LOG("get fill light status %d\n",g_enviro_struct.f_light_st);
                        // g_enviro_struct.f_light_st=1;
                        LedEnable(0);
#if 0
                        if(get_fill_light_status() == 1){
                                if(get_isp_running_mode() != 0)//1´ú±풹ͭģʽ
                                        set_isp_running_mode(1);

                        }
#endif

                }else if(QCamGetModeStatus() == QCAM_MODE_DAY && get_fill_light_status()){
                        // AMCSystemCmd("gpio_opt -n 90 -m 1 -v 0");
                        set_flight_mode(0);
                        //g_enviro_struct.f_light_st=0;
                        set_fill_light_status(0);
                        LOG("get fill light status %d\n",g_enviro_struct.f_light_st);
                        LedEnable(GetYSXCtl(YSX_LIGHT_EN));



                }
#if 0
                gettimeofday(&tv,NULL);
                endtime =tv.tv_sec;
                if(get_fill_light_time_flag()) {
                        starttime = endtime;
                        set_fill_light_time_flag(false);
                }
                //printf("%ld %ld \n",starttime,endtime);
                totaltime = (unsigned int)(endtime - starttime);
                if((totaltime >= FULL_COLOR_TIMTOUT) && get_fill_light_status()){


                        // AMCSystemCmd("gpio_opt -n 90 -m 1 -v 0");
                        set_flight_mode(0);
                        set_fill_light_status(0);
                        LOG("get fill light status %d\n",g_enviro_struct.f_light_st);
                        // g_enviro_struct.f_light_st=0;
                        LedEnable(GetYSXCtl(YSX_LIGHT_EN));

                }
#endif
                sleep(1);   

#endif
        } 
        pthread_exit(0);
}



#if 0
int check_nic()  
{  
        struct ifreq ifr;  
        int skfd = socket(AF_INET, SOCK_DGRAM, 0);  

        strcpy(ifr.ifr_name, "eth0");  
        if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)  
        {  
                printf("ioctl net fail\n");
                return 1;  
        }  
        close(skfd);
        if(ifr.ifr_flags & IFF_RUNNING)  
                return 0;  // 网卡已插上网? 


        return 1;

}

void *Network_Adapter(void *arg)
{
        int ret = -1, last_status = -1;

        while(g_enviro_struct._loop)
        {           
                ret = check_nic();
                if (!ret)
                {
                        if(ret != last_status)
                        {
                                last_status = ret;
                                AMCSystemCmd("ifconfig wlan0 down");
                                AMCSystemCmd("killall udhcpc");
                                AMCSystemCmd("killall wpa_supplicant");
                                AMCSystemCmd("udhcpc -i eth0&");
                                printf("lzf net wire plug in\n");
                        }           
                }
                else
                {
                        if (ret != last_status)
                        {
                                last_status = ret;
                                AMCSystemCmd("ifconfig wlan0 up");
                                AMCSystemCmd("ifconfig eth0 0.0.0.0");
                                AMCSystemCmd("killall udhcpc");
                                AMCSystemCmd("killall wpa_supplicant");
                                if(access("/etc/SNIP39/wpa_supplicant.conf",F_OK)==0){

                                        AMCSystemCmd("wpa_supplicant -i wlan0 -Dnl80211 -c /etc/SNIP39/wpa_supplicant.conf -B");
                                        //system("wpa_supplicant -Dwext -iwlan0 -c/etc/wpa_supplicant.conf&");
                                        AMCSystemCmd("udhcpc -i wlan0&");
                                }
                                printf("lzf net wireless plug out\n");
                        }

                }

                sleep(2);
        }

}
#endif

#if 0  // back from  main.c 后续可能有空
#include  <fcntl.h>

#include  <sys/ioctl.h>

#include  <sys/socket.h>
#include  <linux/if.h>
#include  <linux/sockios.h>
#include  <linux/ethtool.h>


typedef enum { IFSTATUS_UP, IFSTATUS_DOWN, IFSTATUS_ERR } interface_status_t;


interface_status_t interface_detect_beat_ethtool(int fd, char *iface)  
{  
    struct ifreq ifr;  
    struct ethtool_value edata;  
     
    memset(&ifr, 0, sizeof(ifr));  
    strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);  
  
    edata.cmd = ETHTOOL_GLINK;  
    ifr.ifr_data = (caddr_t) &edata;  
  
    if (ioctl(fd, SIOCETHTOOL, &ifr) == -1)  
    {  
        perror("ETHTOOL_GLINK failed ");  
        return IFSTATUS_ERR;  
    }  
  
    return edata.data ? IFSTATUS_UP : IFSTATUS_DOWN;  
}  

int detect_mii(int skfd, char *ifname)
{
   struct ifreq ifr;
   unsigned short *data, mii_val;
   unsigned phy_id;

   /* Get the vitals from the interface. */
   strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

   if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0)
      {
         fprintf(stderr, "SIOCGMIIPHY on %s failed: %s\n", ifname, strerror(errno));
         (void) close(skfd);
         return 2;
      }

   data = (unsigned short *)(&ifr.ifr_data);
   phy_id = data[0];
   data[1] = 1;

   if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0)
     {
        fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name, strerror(errno));
        return IFSTATUS_ERR;
     }

   mii_val = data[3];
   return(((mii_val & 0x0016) == 0x0004) ? IFSTATUS_UP : IFSTATUS_DOWN);
}

static int network_check_dev(const char * hwname)
{
    int ret = 0;
    int sockfd = 0; 
    struct ifreq ifr;
    
    strcpy(ifr.ifr_name, hwname);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd <= 0) {
        ret = errno > 0 ? errno : ret;
        EMGCY_LOG("create socket failed with: %s!\n", strerror(ret));
        return -1;
    }

    ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    if (ret < 0) {
        ret = errno > 0 ? errno : ret;
        EMGCY_LOG("iotcl failed with: %s!\n", strerror(ret));
        close(sockfd);
        return 0;
    }

    if (ifr.ifr_flags & IFF_RUNNING) {
        close(sockfd);
        return 1; 
    }

    close(sockfd);
    return 0;    
}

int wlan0_check_state(const char*dev_name)
{
	#if 0
	#if 1
	interface_status_t status; 
 	int fd,ret = -1;  
      
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)  
    {  
        perror("socket ");  
        return -1 ;  
    } 
	
    if(status = interface_detect_beat_ethtool(fd, dev_name) == IFSTATUS_ERR){

		status = detect_mii(fd,dev_name);
		
	}
	
    close(fd);  
	
    switch (status)  
    {  
        case IFSTATUS_UP:  
            printf("%s : link up\n",dev_name);
			ret = 1;
            break;  
          
        case IFSTATUS_DOWN:  
            printf("%s : link down\n", dev_name);  
			ret = 0;
            break;  
          
        default:  
            printf("Detect Error\n");  
            break;  
    }  
  	#else
	FILE*fp = NULL;
    char carrier_path[128] = {0};  
    char buf[32] = {0};
	interface_status_t status;  
	int ret = -1;
    memset(carrier_path, 0, sizeof(carrier_path)); 
	memset(buf, 0, sizeof(buf));
	
    snprintf(carrier_path, sizeof(carrier_path), "/sys/class/net/%s/carrier", dev_name);  
    if ((fp = fopen(carrier_path, "r")) != NULL)  
    {  
        while (fgets(buf, sizeof(buf), fp) != NULL)  
        {  
            if (buf[0] == '0')  
            {  
                status = IFSTATUS_DOWN;  
            }  
            else if(buf[0] == '1') 
            {  
                status = IFSTATUS_UP;  
            }else{

			 status = IFSTATUS_ERR;	

			}  
        }  
    }  
    else  
    {  
        printf("open carrier fail\n");  
		status = IFSTATUS_ERR;
    }
	
	if(fp)
     fclose(fp);  
	
	switch (status)  
    {  
        case IFSTATUS_UP:  
            printf("%s : link up\n",dev_name);
			ret = 1;
            break;  
          
        case IFSTATUS_DOWN:  
            printf("%s : link down\n", dev_name);  
			ret = 0;
            break;  
          
        default:  
            printf("Detect Error\n"); 
			ret = -1;
            break;  
    }  

	#endif
    #else
	int ret = network_check_dev(dev_name);

	#endif
    return ret;
}

int Inside_Network_check(char *cmd)
{
        char result[64];
        int ret = 0;

        FILE *fp = popen(cmd,"r");
        if(fp)
        {
                if(fread(result,1,sizeof(result),fp) != 0)
                        ret = 1;
                else
                        ret = 0;
                pclose(fp);
        }
        return ret;
}

void GetCurWifi()
{
        //char sRet[260];
        char sTemp[260];
        AMCSystemCmd("iwconfig wlan0 | grep ESSID > /tmp/ssid" );
        //system("cat /etc/wpa_supplicant.conf | grep psk > /tmp/spass");

        FILE *fp = fopen("/tmp/ssid", "rb");
        if (!fp)
                return ;

        fgets(sTemp, 260, fp);
        sscanf(sTemp, "%*[^:]:\"%[^\"]\"",g_enviro_struct.t_wifissid);
        printf("ssid cur %s\n", g_enviro_struct.t_wifissid);
        fclose(fp);

}



void *Route_Detection(void *arg)
{
	prctl(PR_SET_NAME,"Route_Detection");

	int ret = -1,i_ret = -1, cmd_ret= -1;

	char *dev_name = "wlan0";
	char chk_cmd[64];
	memset(chk_cmd, 0, sizeof(chk_cmd));
    //sprintf(chk_cmd,"iwconfig wlan0 | grep %s", g_enviro_struct.t_wifissid);
	while(g_enviro_struct._loop){
		if(access("/tmp/resolv.conf",F_OK) == 0)
			break;
		usleep(50*1000);
	}
	
	sleep(30);
	
	GetCurWifi();
	
	static int try_cnt = 0;
	while(g_enviro_struct._loop){
		#if 1
		memset(chk_cmd, 0, sizeof(chk_cmd));
		if(strlen(g_enviro_struct.t_wifissid)){
		sprintf(chk_cmd,"iwconfig wlan0 | grep %s", g_enviro_struct.t_wifissid);
		cmd_ret = Inside_Network_check(chk_cmd);
		}
		#endif
		ret = wlan0_check_state(dev_name);

		printf("[%s %d] ret:%d cmd_ret:%d\n",__FUNCTION__,__LINE__,ret,cmd_ret);

		
		if(cmd_ret == 1 && ret != 1){
			if(try_cnt++ > 5){
			  network_restart_wlan0();
			  try_cnt = 0;
			}
		}else
			try_cnt = 0;
	
		switch(ret){
		case 1:
		
		if(g_enviro_struct.t_online == 1){
			
		 printf("1 set blue always\n");
		 if(GetLedStatus() != ON_LINE_STATUS)
	     i_ret = SetLed(1, ON_LINE_STATUS);
			
		}else{
		 #if 0
		 if (g_media_struct.g_isVideoNum > 0){
			    
				if(GetLedStatus() != VIDEO_START_STATUS)
				i_ret = SetLed(1, VIDEO_START_STATUS);
				
			}
			else{
				
				if(GetLedStatus() != EZ_STATUS_ROUTE)
				   i_ret = SetLed(1, EZ_STATUS_ROUTE);
				
		   }
		 #endif
           printf("2 set blue always\n");		 	
		   if(GetLedStatus() != ON_LINE_STATUS)
	        i_ret = SetLed(1, ON_LINE_STATUS);
		   }
				break;
		case 0:
		
			printf(" set blue flick\n");
			if (GetLedStatus() != VIDEO_START_STATUS)
				i_ret = SetLed(1, VIDEO_START_STATUS);

				break;	
		
		default:
			   break;		
		}
		sleep(15);
	}
 
	LOG("Route_Detection exit\n");

}
#endif 


#endif
