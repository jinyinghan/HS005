#include "include/common_env.h"
#include "include/common_func.h"

#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include <sys/statfs.h>

#include "network.h"
#include "media.h"
#include "ysx_data_buf.h"
#include  "ff_muxer.h"

#ifdef USB_WIFI
#include <sys/reboot.h>
#endif



#define TIME_ZONE_FILE                  "/etc/TZ"
#ifdef USB_WIFI
void usb_dev_miss(void);
#endif
typedef void (*cmd_func)(int SID, int avIndex, char *buf, int type, int iMediaNo);
static int wifi_changing = 0;



av_buffer *video_live_buffer;
av_buffer *audio_live_buffer;
av_buffer *video_record_buffer;

typedef struct _gCmdFunc
{
	int type;
	cmd_func cfunc;
}G_CmdFunc;



static int save_timezone_info(char* timezone)
{
    FILE *fp = NULL;
    fp = fopen(TIME_ZONE_FILE,"wb");
    if(fp == NULL) {
        fprintf(stderr, "[%s] open %s failed!! \n", __FUNCTION__,TIME_ZONE_FILE);
        return -1;
    }
    else
	{
        fwrite(timezone,strlen(timezone),1,fp);
        fclose(fp);
    }
    return 0;
}

int file_exist(const char *file_name)
{
    if(access(file_name, F_OK) == 0)
        return 1;
    else
        return 0;
}
/*
int get_sd_path(char *path, int path_len)
{
    if (NULL == path) {
        printf("in %d\n", path_len);
        return -1;
    }
    
    FILE * fp = fopen("/tmp/sd_ok", "r");
    if (!fp) {
        path = '\0';
        return -1;
    }

    int ret = fread(path, 1, path_len, fp);
    path[ret - 1] = '\0';
    fclose(fp);
    //printf("################################# path %s\n", path);
    return 0;
}
*/

//获取sd卡容量
//mount_point：sd卡路径  
//totalsize：（输出型参数）总容量(M)
//freesize：（输出型参数）剩余容量(M)
int get_sd_strorage(char *mount_point,uint64_t* totalsize,uint64_t* freesize)
{
    struct statfs diskInfo; //系统stat的结构体

    if (statfs(mount_point, &diskInfo) == -1){   //获取分区的状态
        LOG("stat %s failed with: %s\n", mount_point, strerror(errno));
        return -1;
    }

    uint64_t blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
    *totalsize = (diskInfo.f_blocks * blocksize >> 20);
    *freesize  = (diskInfo.f_bfree  * blocksize >> 20); //剩余空间的大小

    return 0;
}

//检测SD卡是否需要格式化（没有文件系统）
//return 1:SD卡没有格式化  。0:SD卡已经格式化
int unformat_check()
{
    return 0;
}

//获取SD卡状态
int get_sd_status(void)
{
    VAVA_SD_STATUS status  =  VAVA_SD_STATUS_HAVECARD;
    
    if(access("/tmp/sd_ok",F_OK) != 0 && g_enviro_struct.is_formating_sd == 0){ //格式化SD卡时，sd_ok会被删掉
        status = VAVA_SD_STATUS_NOCRAD;
    }else{
        if(readonly_check()){
            status = VAVA_SD_STATUS_READONLY; //VAVA_SD_STATUS_BADCARD
            
    	}else if(unformat_check()){ //sd卡未格式化
            status = VAVA_SD_STATUS_UNFORMAT;
            
    	}else{
            uint64_t totalsize,freesize;
      	    char sdpath[SD_PATH_LEN];
      	    CLEAR(sdpath);
      	    get_sd_path(sdpath, SD_PATH_LEN); 
      	    //get_sd_path(g_enviro_struct.t_sdpath, sizeof(g_enviro_struct.t_sdpath));
      	    int ret = get_sd_strorage(sdpath,&totalsize,&freesize);
      	    if(ret){
                status = VAVA_SD_STATUS_BADCARD; //坏卡需要怎么判断？
            }
            if(freesize < 100){ //小于多少M，可以判断 SD卡为满？

                status = VAVA_SD_STATUS_FULL;
      		}	
    	}
    }

    return status;
}


bool checkCardOnline(char *sdcardDevice)
{
	char buff[50] = {0};
	char *p = NULL;
	int len = 0;
	if(sdcardDevice==NULL)
		return false;

	if(access("/tmp/sd_ok", R_OK) != 0) {
		return false;
	}
	FILE *fp = fopen("/tmp/sd_ok","r");
	if(fp==NULL) {
		memset(sdcardDevice,0,sizeof(sdcardDevice));
		return false;
	}
	fgets(buff,sizeof(buff),fp);
	fclose(fp);
	p = strstr(buff,"mmcblk");
	if(p==NULL) {
		memset(sdcardDevice,0,sizeof(sdcardDevice));
		return false;
	}
	len = strlen(p);
	p[len-1] = '\0';
	strcpy(sdcardDevice,p);
	return true;
}

int get_local_ip(const char *inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, 16, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}

int get_local_mac(const char *inf, char *mac)
{
    struct ifreq ifr;
    int sd;
	int i;

    bzero(&ifr, sizeof(struct ifreq));
    if( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("get %s mac address socket creat error\n", inf);
        return -1;
    }

    strncpy(ifr.ifr_name, inf, sizeof(ifr.ifr_name) - 1);

    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("get %s mac address error\n", inf);
        close(sd);
        return -1;
    }

    snprintf(mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    close(sd);

	for(i=0;i<18;i++)
	{
		if(mac[i]>='a'&&mac[i]<='z')
			mac[i]-=32;
	}

    return 0;
}




inline int GetUtf8charByteNum(unsigned char ch)
{
    int byteNum = 0;

    if (ch >= 0xFC && ch < 0xFE)
        byteNum = 6;
    else if (ch >= 0xF8)
        byteNum = 5;
    else if (ch >= 0xF0)
        byteNum = 4;
    else if (ch >= 0xE0)
        byteNum = 3;
    else if (ch >= 0xC0)
        byteNum = 2;
    else if (0 == (ch & 0x80))
        byteNum = 1;

    return byteNum;
}
inline int GetSsidLen(char *str)
{
#define SSID_LEN		0x20
    int tmp = 0;
    int len = 0;
    int byteNum = 0 ,lastByteNum=0;
    unsigned char ch;
    char *ptr = str;

    if (NULL == str)
        return len;

    while (*ptr != '\0' && len < SSID_LEN)
    {
        ch = (unsigned char)*ptr;
		tmp = byteNum;
        if (0 == (byteNum = GetUtf8charByteNum(ch)))
            return 0;

        ptr += byteNum;
        len += byteNum;
		if(len > byteNum)
		{
			lastByteNum = byteNum;
		}
    }
	if (len >= SSID_LEN)
        len -= (byteNum+lastByteNum);
    return len;
}




void wpa_handle(unsigned char *name, unsigned char *pass,unsigned char key)
{
	FILE *pFile;
	const char *wpa_file = FILE_WPA;

	remove(FILE_WPA_BAK);
	rename(FILE_WPA,FILE_WPA_BAK);

	pFile =	fopen(wpa_file, "w");
	if (pFile != NULL) {
		fprintf(pFile, "network={\n");
		fprintf(pFile, "ssid=\"%s\"\n",	name);


		if (key == 0) // open
			fprintf(pFile, "key_mgmt=NONE\n");
		else if(key == 1)
		{ // wpa
			fprintf(pFile, "scan_ssid=1\n");
			//fprintf(pFile,"key_mgmt=WPA-PSK\n");
			fprintf(pFile, "pairwise=CCMP TKIP\n");
			fprintf(pFile, "group=CCMP TKIP WEP104 WEP40\n");
			fprintf(pFile, "psk=\"%s\"\n", pass);
		}
		else //wep
		{
			int pass_len = 0;
			fprintf(pFile, "scan_ssid=1\n");
			fprintf(pFile, "key_mgmt=NONE\n");

			//目前仅兼容识别 Key0 密钥，其他密钥暂不支持
			pass_len = strlen(pass);
			//ASCII码
			if ((pass_len == 5) | (pass_len == 13)) 		//适配64/128位密钥
			{
				fprintf(pFile, "wep_key0=\"%s\"\n",pass);
			}
			//十六进制
			else if ((pass_len == 10) | (pass_len == 26))	//适配64/128位密钥
			{
				fprintf(pFile, "wep_key0=%s\n",pass); 
			}
			//fprintf(pFile, "wep_key0=%s\n",pass);
			fprintf(pFile, "wep_tx_keyidx=0\n");
			fprintf(pFile, "auth_alg=SHARED\n");
		}


		fprintf(pFile, "}");
		fclose(pFile);

	}
	else {
		fprintf(stderr, "[%s] open %s error\n", __FUNCTION__,wpa_file);
	}

}

#define CMD_LOAD_WIFI                   "modprobe 8188fu"
#define CMD_RELEASE_WIFI                "modprobe -r 8188fu"
#define REOAD_WIFI_DRIVER_MAX_CNT       5   

static int change_flag = 1;
static int exit_change_wifi = 0;
void change_wifi_timer(int sig)
{
    EMGCY_LOG("wait wifi route timeout, change wifi failed...\n");
    change_flag = 0;
}

int change_wifi(const char * ssid, const char * pwd)
{
    int ret = 0;
    struct stat st;
    remove("/tmp/resolv.conf");
	//network_set_bind(1);
	network_restart_wlan0();
    wifi_changing = 1;
    signal(SIGALRM, change_wifi_timer);
    alarm(100);
    do {
        memset(&st, '\0', sizeof(struct stat));
        ret = stat("/tmp/resolv.conf", &st);  
        if (ret == 0) {
            EMGCY_LOG("@@@@@ change wifi ok!\n");
			if(GetYSXCtl(YSX_VINFO_EN))
            AMCSystemCmd("sample_play 16000 /usr/notify/wifi_set.pcm"); 
            wifi_changing = 0;
			SetDeviceMode();
            return 2;    
        }
        usleep(200 * 1000);
    } while(change_flag);
    rename(FILE_WPA_BAK, FILE_WPA);
    return 1;
}

unsigned char WifiChange(const char * ssid, char * wifi_sw)
{
    char cmd[128] = {0};
    struct stat st;
    exit_change_wifi = 0;
	AMCSystemCmd("killall -q udhcpc");
	AMCSystemCmd("killall -q wpa_supplicant");
	AMCSystemCmd("killall -q hostapd");
	AMCSystemCmd("killall -q udhcpd");

    AMCSystemCmd("ifconfig wlan0 down");
    sleep(1);
    AMCSystemCmd("ifconfig wlan0 up");
    sleep(1);

    snprintf(cmd, sizeof(cmd) - 1, "wpa_supplicant -B -iwlan0 -c %s", FILE_WPA);
	AMCSystemCmd(cmd);
	EMGCY_LOG("@@@@@@@@ udhcpc start...\n");
    AMCSystemCmd("udhcpc -i wlan0 -b -R");
    EMGCY_LOG("@@@@@@@@ udhcpc end...\n");
    signal(SIGALRM, change_wifi_timer);  
    alarm(120);
    EMGCY_LOG("@@@@ alarm 20 sec..\n");

    while (exit_change_wifi == 0) 
    {
        if (stat("/tmp/resolv.conf", &st) == 0) {
            EMGCY_LOG("@@@@@@ change wifi ok!\n");
            return 1;
        }
        usleep(200 * 1000);
    }    
    return 0;
}

void SetZone(float iZone)
{//GMT-09:00

        char chZone[100];
        int iLen = 0;
        if (iZone > 0)
        {
              //  gZone = iZone;
                iLen = sprintf(chZone, "GMT-%d:00\n", abs(iZone));
        }
        else
        {
              //  gZone = 0-iZone;
                iLen = sprintf(chZone, "GMT+%d.00\n", abs(iZone));
        }
        chZone[iLen] = '\0';
        LOG("SetZone iLen %d\n", iLen);
        FILE *fZone;
        fZone = fopen("/etc/TZ", "wb+");
        if(fZone >= 0)
        {
                fwrite(chZone, iLen, 1, fZone);
                fclose(fZone);
        }
       // system("touch /etc/zone_sync_ok");
		g_enviro_struct.t_zone = iZone;
}

const int ym_dayofmonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const int rym_dayofmonth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
/*
STimeDay UTCToLocal()
{
	STimeDay st;
	return st;
}
*/
void UTCToLocal(STimeDay *sT)
{
//	printf("enter utctolocal\n");
	const int *dayofmonth = ym_dayofmonth;
	if((sT->year % 4 == 0 && sT->year % 100 != 0) || (sT->year % 400 == 0))
     {
                dayofmonth = rym_dayofmonth;
     }
	// printf("enter utctolocal 1\n");
	 sT->minute += g_enviro_struct.t_zone_m;
	// printf("enter utctolocal sT->minute %d\n", sT->hour);
	if(sT->minute >= 60)
    {
                sT->minute -= 60;
                sT->hour += 1;
    }
    else if(sT->minute < 0)
    {
                sT->minute  += 60;
                sT->hour -= 1;
    }
	sT->hour += g_enviro_struct.t_zone;
	if(sT->hour >= 24)
    {
                sT->hour -= 24;
                sT->day += 1;
    }
    else if(sT->hour < 0)
    {
                sT->hour  += 24;
                sT->day -= 1;
    }

	//printf("enter utctolocal sT->day %d dayofmonth[sT->month-1] %d\n",sT->day,dayofmonth[sT->month-1]);
	if(sT->day > dayofmonth[sT->month-1])
    {
                sT->day = 1;
                sT->month += 1;
    }
    else if(sT->day < 1)
    {
        if(sT->month > 1)
             sT->day = dayofmonth[sT->month-2];
        else
             sT->day = dayofmonth[11];

        sT->month -= 1;
    }
	// printf("enter utctolocal 4\n");
	 if(sT->month > 12)
     {
                sT->month = 1;
                sT->year += 1;
     }
     else if(sT->month < 1)
     {
                sT->month = 12;
                sT->year -= 1;
     }

}

void LocalToUTC(STimeDay *sT)
{
	const int *dayofmonth = ym_dayofmonth;
    if((sT->year % 4 == 0 && sT->year % 100 != 0) || (sT->year % 400 == 0))
	{
		dayofmonth = rym_dayofmonth;
	}
	//sT->hour -= (unsigned char)g_enviro_struct.t_zone;
	int hour = sT->hour;
	hour -= (unsigned char)g_enviro_struct.t_zone;
	//printf("hour %d\n", hour);
	//sT->hour  -= gZone;
	if(hour > 24)
	{
		sT->hour -= 24;
		sT->day += 1;
	}
	else if(hour < 0)
	{
		hour += 24;
		sT->hour = hour;
		sT->day -= 1;
	}
	else
		sT->hour  = hour;

	if(sT->day > dayofmonth[sT->month-1])
	{
		sT->day = 1;
		sT->month += 1;
	}
	else if(sT->day < 1)
	{
		if(sT->month > 1)
		{
			sT->day = dayofmonth[sT->month-2];
		}
		else
		{
			sT->day = dayofmonth[11];

		}
		sT->month -= 1;
	}
	if(sT->month > 12)
	{
		sT->month = 1;
		sT->year += 1;
	}
	else if(sT->month < 1)
	{
		sT->month = 12;
		sT->year -= 1;
	}
}


#ifdef USB_WIFI
bool usb_dev_check(void)
{
	char sTemp[6]={0},sRet[128]={0};
	bool b_ret = false;
	FILE *fp = NULL;
	pid_t pid;
	snprintf(sTemp,sizeof(sTemp),"lsusb");
	fp = ysx_popen(sTemp,"r",&pid);
	if(fp){
		do{
			fgets(sRet, sizeof(sRet), fp);
			pr_dbg("lsusb: %s\n", sRet);
			if (strstr(sRet, "Device") != NULL){
				if (strstr(sRet, "Device 001:") == NULL){
					b_ret = true ;
				}
			}
		}while(!feof(fp));
		ysx_pclose(fp,pid);
		fp = NULL;
	}
	else{
		char buffer[64] = {'\0'};
		sprintf(buffer, "ysx_popen usd_dev error:%s\n", strerror(errno));
		LOG(buffer);
	    ysx_pclose(fp,pid);

		return -1;
	}
	
	return b_ret;
}

void usb_dev_miss(void)
{//Device 002
    EMGCY_LOG("-->\n");
    static int miss_cnt = 0;
	int miss_max = 2;
	if (!usb_dev_check()) {
		AMCSystemCmd("gpio_opt -n 62 -m 1 -v 1");
		if (!usb_dev_check())
			miss_cnt ++;
	}
	
    EMGCY_LOG("########## miss_cnt: %02d, miss_max: %02d\n", miss_cnt, miss_max);
	
	if (miss_cnt >= miss_max){
		AMCSystemCmd("touch /etc/SNIP39/sys_start");
		AMCSystemCmd("gpio_opt -n 62 -m 1 -v 0");
		EMGCY_LOG("########## rmmod wifi driver!\n");
		AMCSystemCmd("rmmod 8188eu");
		pr_info("dev reboot\n");
		AMCSystemCmd("sync");
		AMCSystemCmd("killall -9 watchdog");
		reboot(RB_AUTOBOOT);
	}
}
#endif

