#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "json/json.h"
#include "include/common_env.h"
#include "include/common_func.h"

#define WIFI_TEMP_FILE 	"/tmp/qrcode.txt"
/*ysx aud_dtc*/
#define WIFI_ENC_CODE   "/tmp/sound.txt"

extern G_StructDeviceflag g_flag_struct;

#define WIFI_HOP_CODE   "/tmp/have_connect"
enum WIFI_EASY_LINK_STATE{
	NONE_READY,
	QR_SCAN_READY,
	AUD_DTC_READY,
	HOSTAPD_READY,
	ALWAYS_READY,
	WIRED_READY,
	BT_READY,
};


typedef enum
{
	WIFI_SF_TYPE_OPEN = 0,
	WIFI_SF_TYPE_WEP = 1,
	WIFI_SF_TYPE_WPA = 2,
}ENUM_WIFI_SF_TYPE;


#define  BT_HOP_CODE  "/tmp/bt_ready"
char * get_file_content(const char * file_name)
{
    FILE * fp = fopen(file_name, "r");
    if(fp < 0) {
        LOG("can not open the file: %s\n", file_name);
        return 0;
    }

    int file_size = 0;
    fseek(fp, SEEK_CUR, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    char * buf = (char *)mem_malloc(file_size);
    memset(buf, 0, file_size);
    //size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
    fread(buf,sizeof(char), file_size ,fp);
    fclose(fp);

	int i;
	for(i=0;i<file_size;i++)
	{
		if(buf[i]== '\r' || buf[i]=='\n')
			buf[i]='\0';
	}

    return buf;
}

// ps: ssid  passwd size is 64 !!
int get_result(char *b64_codec_buf,  char *ssid, char *passwd, ENUM_WIFI_SF_TYPE *sf_type, char *ticket)
{
	int ret = -1;
	char *ret_string;
	struct json_object *js_obj=NULL, *members_obj=NULL;
	js_obj = json_tokener_parse(b64_codec_buf);
    if(!js_obj)
    {
        LOG("get_result parse failed:%s\n",b64_codec_buf);
        goto end;
    }

	members_obj = json_object_object_get(js_obj,"ssid");
    if(!members_obj)
    {
        goto end;
    }	
	ret_string = (char *)json_object_get_string(members_obj);
	snprintf(ssid,64-1,"%s",ret_string);
	LOG("ssid : %s\n",ssid);

	members_obj = json_object_object_get(js_obj,"password");
    if(!members_obj)
    {
        goto end;
    }	
	ret_string = (char *)json_object_get_string(members_obj);
	snprintf(passwd,64-1,"%s",ret_string);
	LOG("ssid : %s\n",passwd);

	members_obj = json_object_object_get(js_obj,"ticket");
    if(!members_obj)
    {
        goto end;
    }	
	ret_string = (char *)json_object_get_string(members_obj);
	snprintf(ticket,64-1,"%s",ret_string);
	LOG("ticket : %s\n",ticket);

	
	members_obj = json_object_object_get(js_obj,"securityType");
    if(!members_obj)
    {
        goto end;
    }		
	*sf_type = json_object_get_int(members_obj);
	LOG("securityType : %d\n",*sf_type);

	ret = 1;
	
end:
	if(js_obj)
		json_object_put(js_obj);
    return ret;
	
}



int check_proc(const char *procname)
{
	char buffer[64];
	memset(buffer, 0,  sizeof(buffer));
	FILE *read_fp = popen(procname, "r");
	if (read_fp != NULL)
	{
		fread(buffer, sizeof(char), sizeof(buffer), read_fp);
		pclose(read_fp);

		if (strlen(buffer) != 0)
		{
			LOG("procname exit !\n");
			return 1;
		}
	}

    return 0;
}

int check_ez_proc()
{
	char buffer[64];
	memset(buffer, 0, 64);
	FILE *read_fp = popen("ps | grep aud_dtc | grep -v grep", "r");
	if (read_fp != NULL)
	{
		fread(buffer, sizeof(char), 50, read_fp);
		if (strlen(buffer) == 0)
		{
			pclose(read_fp);
			return 0;
		}
	}
	pclose(read_fp);
	return 1;
}

enum WIFI_EASY_LINK_STATE get_wifi_easy_link_state(void)
{
    enum WIFI_EASY_LINK_STATE state;

    if(0 == access(WIFI_TEMP_FILE, F_OK))
        state = QR_SCAN_READY;
    else if(0 == access(WIFI_ENC_CODE, F_OK))
        state = AUD_DTC_READY;

    else if(0 == access(WIFI_HOP_CODE, F_OK))
        state = HOSTAPD_READY;
	else if(g_enviro_struct.net_mode == WIRED_MODE)
        state = WIRED_READY;

    else if(0 == access(BT_HOP_CODE, F_OK))
		state= BT_READY;
    else
        state = NONE_READY;
    return state;
}

unsigned char GetDeviceMode()
{
    int tMode = 0;
    FILE *fdefault;
    char buffer[25];

    if(access(CAMERA_STATE,F_OK) != 0)
        return 0;

	fdefault = fopen(CAMERA_STATE, "rb");
	if(fdefault)
	{
		fread(buffer,1,sizeof(buffer),fdefault);
		fclose(fdefault);
		if(strstr(buffer,"OperationMode=1"))
			tMode = 1;
	}
	return (unsigned char)tMode;
}

void SetDeviceMode()
{
    FILE *fdefault = fopen(CAMERA_STATE, "wb");
    if(fdefault) {
        fprintf(fdefault, "OperationMode=1");
        fclose(fdefault);
    }
	g_enviro_struct.is_bind = 1;
	//if(!g_enviro_struct.standalone_mode)
	//	network_set_bind(g_enviro_struct.is_bind);
}

#define HOSTAPD_TMP_FILE "/tmp/hostapd.conf"
int create_hostapd_conf(const char * ssid, const char * pwd)
{
	int ret = 0;
	FILE * fp = fopen(HOSTAPD_TMP_FILE, "wb");
	if (NULL == fp) {
		ret = errno > 0 ? errno : ret;
		LOG("open %s failed with %s!\n", HOSTAPD_TMP_FILE, strerror(ret));
		return -1;
	}

	fprintf(fp, "interface=wlan0\n");
	fprintf(fp, "driver=nl80211\n");
	fprintf(fp, "ssid=%s\n", ssid); 
	fprintf(fp, "hw_mode=g\n");
	fprintf(fp, "channel=1\n");
	fprintf(fp, "macaddr_acl=0\n");
	fprintf(fp, "ignore_broadcast_ssid=0\n");
	fprintf(fp, "auth_algs=1\n");

	if (pwd) {
		fprintf(fp, "wpa=2\n");
		fprintf(fp, "wpa_passphrase=12345678\n");
		fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
		fprintf(fp, "wpa_pairwise=TKIP\n");
		fprintf(fp, "rsn_pairwise=CCMP TKIP\n");
	}     
	fclose(fp);
	return 0;
}

#define BIND_SSID_HEAD          "ENKLOV Camera-"
int modify_ap_ssid(void)
{
	int ret = 0;
	if (file_exist(HOSTAPD_TMP_FILE)) {
		remove(HOSTAPD_TMP_FILE);
	}

    char SSID_NAME[32] = {'\0'};
	strncpy(SSID_NAME, BIND_SSID_HEAD, sizeof(BIND_SSID_HEAD));
	strncat(SSID_NAME, g_enviro_struct.t_uid, 6);
	SSID_NAME[sizeof(BIND_SSID_HEAD)+6] = '\0';

    LOG("create hostapd conf!\n");
	ret = create_hostapd_conf(SSID_NAME, (void *)0);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

//等待 配网线程
int WaitEzSetupOK()
{
	enum WIFI_EASY_LINK_STATE easy_link_state = NONE_READY;
	char *b64_codec_buf =NULL;
	char passwd[64] = {'\0'};
	char ssid[64] = {'\0'};
	char ticket[64] = {'\0'};
	int securityType = 0;
	int doublelight_status = 0;
	char name[256] = {'\0'};

    remove(WIFI_TEMP_FILE);
	remove(WIFI_ENC_CODE);
	remove(BT_HOP_CODE);
	remove(WIFI_HOP_CODE);
	int set_ap = 0;

	printf("g_enviro_struct.is_bind %d\n",g_enviro_struct.is_bind);
	LOG("DeviceMode->%d\n", g_enviro_struct.is_bind);	
	//没有绑定
	if (!g_enviro_struct.is_bind) {
		AMCSystemCmd("iwlist wlan0 scanning > /tmp/iwlist");
		sleep(1);
		LOG("NetworkMode->%d\n", g_enviro_struct.net_mode);

		//如果没有绑定 开启声波配对 则打开声波配对线程
		memset(name, 0, sizeof(name));
		sprintf(name, "ps | grep aud_dtc | grep -v grep");
		if (!check_proc(name)) {
			AMCSystemCmd("aud_dtc&");
	    }
		//如果没有绑定 开启二维码扫描 则打开声波配对线程
		/*
        memset(name,0,sizeof(name));
        sprintf(name, "ps | grep qr_scan | grep -v grep");
        if (!check_proc(name))
            AMCSystemCmd("qr_scan&");
		*/
		qr_scan_init();
		
		printf("ez_setup step 1\n");

	}
	else {// 已经绑定 直接联网
		easy_link_state = ALWAYS_READY;
		//ap_mode_band_uninitialization();
		AMCSystemCmd("wpa_supplicant -B -i wlan0  -c /etc/SNIP39/wpa_supplicant.conf");
        AMCSystemCmd("udhcpc -i wlan0&");
		return 1;
    }
	
	printf("ez_setup step 2\n");
	while(easy_link_state == NONE_READY)
	{
		ysx_setTimer(1, 0);  
		easy_link_state = get_wifi_easy_link_state();
		if (QR_SCAN_READY == easy_link_state) {  // 二维码扫描 结果解析
			b64_codec_buf = get_file_content(WIFI_TEMP_FILE);
			get_result(b64_codec_buf, ssid, passwd,(ENUM_WIFI_SF_TYPE *)&securityType,ticket);
			if (strlen(ssid) == 0)
				continue;

			remove(WIFI_TEMP_FILE);
			AMCSystemCmd("killall aud_dtc");
			break;
		}
		else if(AUD_DTC_READY == easy_link_state){

			b64_codec_buf = get_file_content(WIFI_ENC_CODE);
			get_result(b64_codec_buf, ssid, passwd,(ENUM_WIFI_SF_TYPE *)&securityType,ticket);

			if (strlen(ssid) == 0)
				continue;
			remove(WIFI_ENC_CODE);
			AMCSystemCmd("killall qr_scan");
			break;
		}

		else if(easy_link_state == HOSTAPD_READY) {
			remove(WIFI_HOP_CODE);
			set_ap = 1;
			doublelight_status = 1;
            g_flag_struct.wifi_conf_replay_flag = true;
			break;
		}
		else if(easy_link_state == WIRED_READY) {
			SetDeviceMode();
			return 2;
		}else if(easy_link_state == BT_READY) {
			
		    printf("ez_setup step 3\n");
			remove(BT_HOP_CODE);
			doublelight_status = 1;
            g_flag_struct.wifi_conf_replay_flag = true;
			break;
		}  
    }
    if(b64_codec_buf)
        mem_free(b64_codec_buf);


	if (g_flag_struct.wifi_conf_replay_flag) {
		g_flag_struct.wifi_conf_replay_flag = false;
		if(GetYSXCtl(YSX_VINFO_EN))
		AMCSystemCmd("sample_play 16000 /usr/notify/wifi_conf.pcm");
	}	

	if(set_ap)
		return 1;
	if (doublelight_status == 0) {
		LOG("ok ez-setup %s\n", ssid);
		printf("ok ez-setup %s\n", ssid);

		vava_set_ticket(ticket);

		if(securityType==WIFI_SF_TYPE_OPEN)
		{
			wpa_handle((unsigned char *)ssid, (unsigned char *)passwd, 0);
		}
		else if(securityType==WIFI_SF_TYPE_WPA)
		{
			wpa_handle((unsigned char *)ssid, (unsigned char *)passwd, 1);
		}
		else if(securityType==WIFI_SF_TYPE_WEP)
		{
			wpa_handle((unsigned char *)ssid, (unsigned char *)passwd, 2);//fix me!!
		}
	}
	
	SetDeviceMode();
	AMCSystemCmd("killall -q  udhcpd");
	AMCSystemCmd("killall -q  hostapd");
	usleep(50*1000);
	printf("ez_setup step 4\n");
	AMCSystemCmd("wpa_supplicant -B -i wlan0  -c /etc/SNIP39/wpa_supplicant.conf");
                    
    AMCSystemCmd("udhcpc -i wlan0&");
	
	return 1;
}


void *thread_EZ(void *arg)
{
    int i_ret = 0;
    prctl(PR_SET_NAME, "thread_EZ");

    i_ret = WaitEzSetupOK();
    if (i_ret == 2) {
        LOG("Skip ez-setup because of eth0 running\n");
    }
    else if (i_ret == 1) {
        LOG("ez configure ok\n");
    }
    else {
        LOG("in\n");
    }
    pthread_exit(0);
}

int init_ez_thread(void)
{
	int ret;
    pthread_t ThreadEZ_pid;
    if((ret = pthread_create(&ThreadEZ_pid, NULL, &thread_EZ, NULL)) < 0)
    {
        LOG( "Login Thread create fail, ret=[%d]\n", ret);
        return -1;
    }
    pthread_join(ThreadEZ_pid, 0);  // 如果配网没有成功   此处将不会往下走

	return 0;
}

pthread_t wiredbind_tid = 0;
unsigned char wiredbind_loop = 0;
pthread_t apmodebind_tid = 0;
unsigned char apmodebind_loop = 0;

void *pthread_wired_bind(void *arg)
{
	EMGCY_LOG("@@@ Start Wired Bind @@@\n");
	while(wiredbind_loop){
		if(g_enviro_struct.net_mode == WIRED_MODE){
			SetDeviceMode();
			break;
		}
		ysx_setTimer(2, 0);
	}

	pthread_exit(0);
}

int wired_mode_bind_initialization(void *arg)
{
    int i_ret = -1;
	wiredbind_loop = 1;
    i_ret = pthread_create(&wiredbind_tid, NULL, &pthread_wired_bind, NULL);
    if(i_ret < 0) {
        pr_error("pthread_wired_bind create fail, ret=[%d]\n", i_ret);
        return i_ret;
    }
    pthread_detach(wiredbind_tid);
    return i_ret;
}

void wird_mode_bind_uninitialization(void)
{
	wiredbind_loop = 0;
    if(wiredbind_tid)
        pthread_join(wiredbind_tid, NULL);
}
#if  1
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_head.h"
#include <errno.h>
typedef struct {
    unsigned char  socket_create;
    unsigned char  socket_wait;
    unsigned char  socket_rec;
    unsigned char  socket_sed;
}ap_mode_work_flag_s;

ap_mode_work_flag_s ap_band_flag = {0};
pthread_t apsocket_tid = 0;

void *pthread_apmode_socket(void *arg)
{
    prctl(PR_SET_NAME,"pthread_apmode_socket");
    pr_dbg("#### pthread_apmode_socket start ...\n");
    char cmd[256] = {0};
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct timeval tv = {3, 0}, socket_timeout_r = {3, 0}, socket_timeout_s = {3, 0};
    int new_server_socket = 0;
    int server_socket = 0;
    int fd = 0;
    int rec_len = 0;
    int sen_len = 0;
    int pos = 0;
    int len = 0;
    int i_cnt = 0;
    int i_err_no = 0;
    fd_set  readfds,tmpfds;
    s_ysx_ap_socket_receive *dev_ap_info_rec = NULL;
    s_ysx_ap_socket_send *dev_ap_info_sed = NULL;
    socklen_t length = 0;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr((const char *)SOCKET_SERVER_IP);
    server_addr.sin_port = htons(24385);
	//serverAddr.sin_port = htons(24385);
     //serverAddr.sin_addr.s_addr = inet_addr("192.168.1.1");
    while(ap_band_flag.socket_create) {

       
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            pr_error("Create Socket Failed!\n");
            usleep(3000*1000);
            continue;
        }

       
        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
            pr_error("Server Bind Port: %d Failed!\n", SOCKET_PORT_S);
            shutdown(server_socket,2);
            close(server_socket);
            usleep(3000*1000);
            continue;
        }

        if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE)) {
            pr_error("Server Listen Failed!\n");
            shutdown(server_socket,2);
            close(server_socket);
            usleep(3000*1000);
            continue;
        } else {
            break;
        }

    }
   
    FD_ZERO(&readfds);
    FD_SET(server_socket,&readfds);
    dev_ap_info_rec=(s_ysx_ap_socket_receive*)malloc(sizeof(s_ysx_ap_socket_receive));
    dev_ap_info_sed=(s_ysx_ap_socket_send*)malloc(sizeof(s_ysx_ap_socket_send));
    while(ap_band_flag.socket_wait) {


        tmpfds=readfds;
        length = sizeof(client_addr);
        if(select(FD_SETSIZE,&tmpfds,NULL,NULL,&tv)<1) {

            continue;
        }
        pr_dbg("select ok\n");
        for(fd = 0; fd < FD_SETSIZE; fd++) {

            if(FD_ISSET(fd,&tmpfds)) {

                if(fd==server_socket) {
                    pr_dbg("fd==server_socket\n");
                    new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
                    if (new_server_socket < 0) {
                        pr_error("Server Accept Failed!\n");
                        usleep(1000*1000);
                        continue;
                    } else {
                        pr_dbg("connect ok \n");
						
                        rec_len = sizeof(s_ysx_ap_socket_receive);
                        break;
                    }
                }
            }
        }
		#if 0
        if(rec_len == 0) {
          

            continue;
        }
        i_cnt = 0;
    	
        setsockopt(new_server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&socket_timeout_r, sizeof(struct timeval));
        while((i_cnt++ < 10) && (ap_band_flag.socket_rec) &&
            (pos < rec_len)) {

            len = recv(new_server_socket, dev_ap_info_rec+pos, (sizeof(s_ysx_ap_socket_receive)), 0);
            if(len < 0) {
                i_err_no = errno;
                memset(dev_ap_info_rec, 0, rec_len);
                pos = 0;

                pr_error("err %s, %s\n",strerror(errno), strerror(i_err_no));
                pr_error("Server Recieve Data Failed!\n");
                usleep(200*1000);

                continue;
            }
            pos += len;
            pr_dbg("in %d,%d,%d \n", len, rec_len, (strlen((char *)dev_ap_info_rec->ssid)));
        }
        i_cnt = 0;
        pr_info("recv over %s, %s, %d, %d\n", dev_ap_info_rec->ssid, dev_ap_info_rec->password,
                                             dev_ap_info_rec->mode, dev_ap_info_rec->enctype);
        if((strlen((char *)dev_ap_info_rec->ssid) == 0) ||
        (strlen((char *)dev_ap_info_rec->ssid) >= BUFF_LEN_SSID) ||
        (strlen((char *)dev_ap_info_rec->password) >= BUFF_LEN_PWD)) {
            pr_error("in\n");
            if(new_server_socket != 0) {
                close(new_server_socket);
                new_server_socket = 0;
            }
            continue;
        }
	
        wpa_handle((unsigned char *)dev_ap_info_rec->ssid, (unsigned char *)dev_ap_info_rec->password, dev_ap_info_rec->enctype);
        pos = 0;

        sen_len=sizeof(s_ysx_ap_socket_send);
        memset(dev_ap_info_sed, 0, sen_len);
        get_uid_from_flash((char*)dev_ap_info_sed->uid);
        dev_ap_info_sed->result     = AP_RESULT_SUCCESS;
        pr_dbg("send over %d, %s\n", dev_ap_info_sed->result, dev_ap_info_sed->uid);
        while((i_cnt++ < 10) && (ap_band_flag.socket_sed) &&
            (pos < sen_len)) {

            len = send(new_server_socket, dev_ap_info_sed+pos, (sizeof(s_ysx_ap_socket_send)),0);
            if(len < 0) {
                pos = 0;

                usleep(200*1000);
                pr_error("Server send Data Failed!\n");
                continue;
            }
            pos += len;
        }
		
        if(i_cnt >= 10) {
            pr_error("in\n");
            if(new_server_socket != 0) {
                close(new_server_socket);
                new_server_socket = 0;
            }
            continue;
        }
        i_cnt = 0;
		#endif
        sprintf(cmd, "touch %s", WIFI_HOP_CODE);
        AMCSystemCmd(cmd);
        if(new_server_socket != 0) {
            close(new_server_socket);
            new_server_socket = 0;
        }
        break;
    }
    if(server_socket != 0) {
        close(server_socket);
        server_socket = 0;
    }
    ysx_free(dev_ap_info_sed);
    ysx_free(dev_ap_info_rec);
    return ((void *)0);
}
int ap_mode_band_initialization(void *arg)
{
//    pr_dbg("in \n");
    int i_ret = -1;
    ap_band_flag.socket_create = 1;
    ap_band_flag.socket_wait = 1;
    ap_band_flag.socket_rec = 1;
    ap_band_flag.socket_sed = 1;
    i_ret = pthread_create(&apsocket_tid, NULL, &pthread_apmode_socket, NULL);
    if(i_ret < 0) {
        pr_error("pthread_apmode_socket create fail, ret=[%d]\n", i_ret);
        return i_ret;
    }
   // pthread_detach(apsocket_tid);  //����������Ǹ���ͻ�ɣ���Ҫע�͵�
   // return i_ret;
}

void ap_mode_band_uninitialization(void)
{
//    pr_dbg("in \n");
    ap_band_flag.socket_create = 0;
    ap_band_flag.socket_wait = 0;
    ap_band_flag.socket_rec = 0;
    ap_band_flag.socket_sed = 0;
    if(apsocket_tid)
        pthread_join(apsocket_tid, NULL);
}

#else
void *pthread_apmode_bind(void *arg)
{
	EMGCY_LOG("@@@ Start AP-Mode Bind @@@\n");
	while (apmodebind_loop)
	{
		printf("socket init\n");
		int welcomeSocket, newSocket,fd;
		struct sockaddr_in serverAddr;
		struct sockaddr_storage serverStorage;
		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		socklen_t addr_size;
		fd_set  readfds,tmpfds;

		welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(24385);
		serverAddr.sin_addr.s_addr = inet_addr("192.168.1.1");
		memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
		int n = 1;
		setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
		if (bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))<0) {	
			ysx_setTimer(2, 0);
			printf("bind Error\n");
			shutdown(welcomeSocket,2);
			close(welcomeSocket);
			continue;   
		}

		if (listen(welcomeSocket,5)<0) {
			printf("listen Error\n");
			ysx_setTimer(2, 0);
			shutdown(welcomeSocket,2);
			close(welcomeSocket);
			continue;
		}

		FD_ZERO(&readfds);
		FD_SET(welcomeSocket,&readfds);
		
		while (apmodebind_loop)
		{
			tmpfds = readfds;
			if (g_enviro_struct.is_bind) {
				printf("stop socket\n");
				break;
			}
			
			if (select(FD_SETSIZE, &tmpfds, NULL, NULL, &tv) < 1) {
				continue;
			}
			
			printf("select ok\n");
			for (fd = 0; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET(fd, &tmpfds)) {
					printf("FD_ISSET\n");
					if (fd == welcomeSocket) {
						printf("fd1==welcomeSocket\n");
						addr_size = sizeof(serverStorage);
						newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
						printf("%d\n",newSocket);
						if (newSocket<0) {
							continue;
                        }
						else {
							printf("connect ok \n");
							AMCSystemCmd("touch /tmp/have_connect");
							break;
						}
					}
				}
			}
			ysx_setTimer(0, 200);
		}

		printf("close\n");
		shutdown(newSocket,2);
		close(newSocket);
		shutdown(welcomeSocket,2);
		close(welcomeSocket);
		break;
	}
	
	pthread_exit(0);
}

int ap_mode_bind_initialization(void *arg)
{
    int i_ret = -1;
	apmodebind_loop = 1;
    i_ret = pthread_create(&apmodebind_tid, NULL, &pthread_apmode_bind, NULL);
    if(i_ret < 0) {
        pr_error("pthread_apmode_bind create fail, ret=[%d]\n", i_ret);
        return i_ret;
    }
    pthread_detach(apmodebind_tid);
    return i_ret;
}

void ap_mode_bind_uninitialization(void)
{
	apmodebind_loop = 0;
    if(apmodebind_tid)
        pthread_join(apmodebind_tid, NULL);
}
#endif
