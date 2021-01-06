/*************************************************************************
  > File Name: network.c
  > Author: unixcc
  > Mail: 2276626887@qq.com 
  > Created Time: 2018???08???30??? ????????? 02???34???40?§’
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <sys/un.h>
#include <pthread.h>
#include  <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>

#include "network.h"


#define MAX_WAIT_TIME       1
#define MAX_NO_PACKETS     5
#define ICMP_HEADSIZE         8 
#define PACKET_SIZE           4096


struct timeval tvsend,tvrecv1,tvrecv2;
struct sockaddr_in dest_addr,recv_addr;
int sockfd;pid_t pid;


char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];



int self_system(const char * cmd)
{
	pid_t pid;                                                                                                                     
	int status;
	if (cmd == NULL) {
		return (1);
	}
	if ((pid = vfork())<0) {
		status = -1;
	}
	else if(pid == 0) {
		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		_exit(127); 
	}
	else {
		while(waitpid(pid, &status, 0) < 0)
		{
			if (errno != EINTR) {
				status = -1; 
				break;
			}
		}
	}
	return status; 
}



void alarmtimeout(int signo)
{	
	printf("Request Timed Out\n");
}

void tv_sub(struct timeval *out, struct timeval *in)
{       	
	if( (out->tv_usec-=in->tv_usec)<0)	
	{       		
		--out->tv_sec;		
		out->tv_usec+=1000000;	
	}	
	out->tv_sec-=in->tv_sec;
}

void _CloseSocket()
{	
	close(sockfd);	
	sockfd = 0;
}



unsigned short cal_chksum(unsigned short *addr,int len)
{       	
	int nleft=len;	
	int sum=0;	
	unsigned short *w=addr;	
	unsigned short answer=0;	

	while(nleft>1)	
	{       		
		sum+=*w++;		
		nleft-=2;	
	}	

	if (nleft==1) {		
		*(unsigned char *)(&answer)=*(unsigned char *)w;		
		sum+=answer;	
	}	

	sum=(sum>>16)+(sum&0xffff);	
	sum+=(sum>>16);	
	answer=~sum;	

	return answer;
}


int pack(int pkt_no,char*sendpacket)
{       	
	int packsize;	
	struct icmp *icmp;	
	struct timeval *tval;	
	icmp=(struct icmp*)sendpacket;	
	icmp->icmp_type=ICMP_ECHO;      //??????????ICMP????????	
	icmp->icmp_code=0;	
	icmp->icmp_cksum=0;	
	icmp->icmp_seq=pkt_no;	
	icmp->icmp_id=pid;			    //???????°????ID??ICMP??????	
	packsize=ICMP_HEADSIZE+sizeof(struct timeval);	
	tval= (struct timeval *)icmp->icmp_data;	
	gettimeofday(tval,NULL);	
	icmp->icmp_cksum=cal_chksum( (unsigned short *)icmp,packsize); 	

	return packsize;
}




int unpack(int cur_seq,char *buf,int len)
{    
	int iphdrlen;	
	struct ip *ip;	
	struct icmp *icmp;
	ip=(struct ip *)buf;	
	iphdrlen=ip->ip_hl<<2;				//??ip????????,??ip??????????±?????4	
	icmp=(struct icmp *)(buf+iphdrlen);		//????ip????,????ICMP????	
	len-=iphdrlen;						//ICMP??????ICMP????±?????????	
	if( len<8)		
		return -1;  	
	if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_id==pid) && (icmp->icmp_seq==cur_seq))		
		return 0;		
	else 
		return -1;
}


int network_send_packet(int pkt_no,char *sendpacket)
{    	
	int packetsize;       

	packetsize=pack(pkt_no,sendpacket); 

	gettimeofday(&tvsend,NULL);    

	if(sendto(sockfd,sendpacket,packetsize,0,(struct sockaddr *)&dest_addr,sizeof(dest_addr) )<0)	
	{      		
		printf("[NetStatus]  error : sendto error");		
		return -1;	
	}	

	return 1;
}



int network_recv_packet(int pkt_no,char *recvpacket)
{       		
	int n = 0;
	socklen_t fromlen;	
	fd_set rfds;	

	struct timeval  timer;

	timer.tv_sec = 3;
	timer.tv_usec = 0;

	FD_ZERO(&rfds);	
	FD_SET(sockfd,&rfds);

	signal(SIGALRM,alarmtimeout);

	fromlen=sizeof(recv_addr);	

	alarm(MAX_WAIT_TIME);

	gettimeofday(&tvrecv1,NULL);

	while(1)	
	{		
		select(sockfd+1, &rfds, NULL, NULL, &timer);		
		if (FD_ISSET(sockfd,&rfds))		
		{  			
			if( (n=recvfrom(sockfd,recvpacket,PACKET_SIZE,0,(struct sockaddr *)&recv_addr,&fromlen)) <0)    		
			{   			
				if(errno==EINTR)				
					return -1;	

				//perror("recvfrom error");				
				return -2;      		
			}		
		}

		if(unpack(pkt_no,recvpacket,n)==-1)
		{
			gettimeofday(&tvrecv2,NULL);

			if(tvrecv2.tv_sec - tvrecv1.tv_sec >= 10)
				return -1;
			else
				continue;
		}

		return 1;

	}
}





/*
 * ??  ??: ?ì?é???¨?????????????????¨??????
 * ??  ??:
 * ??????:
 */
int network_wan_available(const char * getwayIP, int * success)
{     			
	int i;
#ifdef _USE_DNS      
	char hostname[32];
	sprintf(hostname,"%s", "www.baidu.com");
	bzero(&dest_addr, sizeof(dest_addr));
	
	
	if ((host=gethostbyname(hostname)) == NULL) {		
		printf("[NetStatus]  error : Can't get serverhost info!\n");		
		return -1;	
	}	

	bcopy((char*)host->h_addr,(char*)&dest_addr.sin_addr,host->h_length);

#else 
		dest_addr.sin_addr.s_addr = inet_addr(getwayIP);
#endif	
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) 	
	{		
		printf("[NetStatus]  error : socket");		
		return -1;	
	}	
	
	int iFlag;

	if(iFlag = fcntl(sockfd,F_GETFL,0)<0)	
	{		
		printf("[NetStatus]  error : fcntl(sockfd,F_GETFL,0)");		
		_CloseSocket();		
		return -1;	
	}
	
	iFlag |= O_NONBLOCK;	
	
	if(iFlag = fcntl(sockfd,F_SETFL,iFlag)<0)	
	{		
		printf("[NetStatus]  error : fcntl(sockfd,F_SETFL,iFlag )");		
		_CloseSocket();		
		return 0;	
	}	
	
	pid=getpid();	
		
	for(i=0;i<MAX_NO_PACKETS;i++)	
	{					
			if(network_send_packet(i,sendpacket)<0)		
			{			
				printf("[NetStatus]  error : send_packet");			
				_CloseSocket();			
				return -1;		
			}
		
			if(network_recv_packet(i,recvpacket)>0)		
			{	
			       (*success)++;
			}			
	} 	

	_CloseSocket(); 
	
	return 0;
	
}




int get_gateway_addr(char * ip_buffer, const char * net_interface)
{
	FILE *gateway_ip_ponit = NULL;
	char line[128];
	char *s1 =  NULL;
	char *s2 = NULL;
	char *s3 = NULL;
	int i = 0;

	system("route  -n > /var/gateway.txt");
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





int network_get_gateway(char * const buf, const char * ift)
{
	FILE *gateway_ip_ponit = NULL;
	char line[128];
	char *s1 =  NULL;
	char *s2 = NULL;
	char *s3 = NULL;
	int i = 0;

	system("route  -n > /var/gateway.txt");
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

		if (NULL == (s3 = strstr(line, ift))) {
			memset((void *)line , 0 , 128);
			continue;
		}

		s1 = s1 + strlen("0.0.0.0");
		for (s1++; is_a_space(*s1); s1++);

		while((*s1)!=' ')
		{
			buf[i] = *s1;
			i ++;
			s1 ++;
		}
		break;
	}

	fclose(gateway_ip_ponit);
	return 0;
}





int get_dns_addr(char * dns1_ip, char * dns2_ip)
{
	FILE *dns_ip_point = NULL;
	char *line = NULL;
	char *s1 =  NULL;
	int i = 0;
	int num = 0;

	line =  (char *)mem_malloc(sizeof(char) * 128);
	if (line == NULL) {
		printf("Debug_Malloc buffer for read dns ip fail %s %d\n", __FUNCTION__,__LINE__);
		return -1;
	}

	dns_ip_point = fopen("/tmp/resolv.conf", "r");
	if (dns_ip_point == NULL) {
		printf("Open dns file failed \n");
		return -1;;
	}

	while (!feof(dns_ip_point))
	{
		i = 0;
		fgets((char *)line, 128, dns_ip_point);

		if (NULL == (s1 = strstr(line, "nameserver"))) {
			memset((void *)line , 0 , 128);
			continue;
		}

		num++;
		s1 = s1 + strlen("nameserver");

		for (s1++; is_a_space(*s1); s1++);
		while ((*s1)!=' ' && (*s1)!='\n')
		{     
			if (num==1) {
				dns1_ip[i] = *s1;                       
				i++;
				s1++;
			}
			if (num == 2) {
				dns2_ip[i] = *s1;                       
				i++;
				s1++;                    
			}
		}
		break;
	}

	fclose(dns_ip_point);

	if (strlen(dns1_ip)!= 0) {
		dns1_ip[strlen(dns1_ip)+1] = '\0';
		printf("DNS1_ip = %s\n", dns1_ip);
	}

	if (strlen(dns2_ip)!= 0) {
		dns2_ip[strlen(dns2_ip)+1] = '\0';
		printf("DNS2_ip = %s\n", dns2_ip);
	}
	mem_free(line);
	return 0;
}

int check_wifi_link(void)
{
	int ret = 0;
	int return_value =0;
	char gateway_ip[16];
	FILE * wifi_ture_false_link = NULL;
	char cmd_buf[128];
	char line_buf[128];
	char * s1 = NULL;

	memset(gateway_ip, 0, 16);
	memset(cmd_buf, 0, 128);

	ret  = get_gateway_addr(gateway_ip, "wlan0");
	if (strlen(gateway_ip) > 0) {
		sprintf(cmd_buf, "ping %s -w 5 | grep -E \"packet loss\"", gateway_ip); 
		wifi_ture_false_link = popen(cmd_buf, "r");
		if (wifi_ture_false_link == NULL) {
			printf("popen wifi_ture_false_link: cmd_buf failed \n");
			pclose(wifi_ture_false_link);
			return -1;
		}
		memset(line_buf, 0, 128);
		while (fgets((char *)line_buf, 128, wifi_ture_false_link) != NULL)
		{
			s1 = strstr(line_buf, "100%");
			if (s1 == NULL) {               
				return_value = 1;
			}
			memset(line_buf,0, 128);
		}
		pclose(wifi_ture_false_link);

	}
	else {
		return_value = -1;
	}

	return  return_value;
}




int network_eth_link_statsu(void)
{//-1 disconnect, 1 connect
	int ret = 0;
	int return_value =0;
	char gateway_ip[16];
	FILE * wifi_ture_false_link = NULL;
	char cmd_buf[128];
	char line_buf[128];
	char * s1 = NULL;

	memset(gateway_ip, 0, 16);
	memset(cmd_buf, 0, 128);

	ret  = get_gateway_addr(gateway_ip, "eth0");
	if (strlen(gateway_ip) > 0) {
		sprintf(cmd_buf, "ping %s -w 5 | grep -E \"packet loss\"", gateway_ip); 
		wifi_ture_false_link = popen(cmd_buf, "r");
		if (wifi_ture_false_link == NULL) {
			printf("popen wifi_ture_false_link: cmd_buf failed \n");
			pclose(wifi_ture_false_link);
			return -1;
		}
		memset(line_buf, 0, 128);
		while (fgets((char *)line_buf, 128, wifi_ture_false_link) != NULL)
		{
			s1 = strstr(line_buf, "100%");
			if (s1 == NULL) {               
				return_value = 1;
			}
			memset(line_buf,0, 128);
		}
		pclose(wifi_ture_false_link);

	}
	else {
		return_value = -1;
	}

	return  return_value;
}





void reset_wifi(void)
{
	system("ifconfig wlan0 down");
	usleep(50000);
	system("ifconfig wlan0 up");
	usleep(50000);
}




int get_device_net_info(char *net_interface_name, char * devIP, char * devMAC)
{
	struct sockaddr_in addr;
	struct sockaddr sa; 
	struct ifreq ifr;
	unsigned char mac[6];
	unsigned char ip_addr[4];
	unsigned char mask_addr[4];
	unsigned char broad_addr[4];

	char *ip_address;
	char *mask_address;
	char *broad_address;

	int sockfd;
	unsigned int   n_IP;


	memset(mac, 0, 6);
	memset(ip_addr, 0, 4);
	memset(mask_addr, 0, 4);
	memset(broad_addr, 0, 4);

	sockfd = socket(AF_INET,SOCK_DGRAM,0);

	if (net_interface_name == NULL) {
		return -1;
	}

	strncpy(ifr.ifr_name, net_interface_name, IFNAMSIZ-1); 
	if (ioctl(sockfd,SIOCGIFADDR,&ifr) == -1) {  
		printf("ioctl error for GET IPADDR\n");
		return  -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));

	memcpy(&addr, (struct sockaddr_in *)&(ifr.ifr_addr), sizeof(struct sockaddr_in));
	ip_address = inet_ntoa(addr.sin_addr);

	if (devIP != NULL)
		sprintf(devIP, " %s\n",ip_address);


	n_IP = ntohl(addr.sin_addr.s_addr); 	 	 
	memcpy(ip_addr, (char *)&n_IP, 4);

	if (ioctl(sockfd,SIOCGIFBRDADDR,&ifr) == -1) {
		printf("ioctl error for GET BRDADDR\n");
		close(sockfd);
		return  -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));  
	memcpy(&addr, (struct sockaddr_in *)&(ifr.ifr_addr), sizeof(struct sockaddr_in));
	broad_address = inet_ntoa(addr.sin_addr);		
	n_IP = ntohl(addr.sin_addr.s_addr);
	memcpy(broad_addr, (char *)&n_IP, 4);	 


	if (ioctl(sockfd,SIOCGIFNETMASK,&ifr) == -1) {
		printf("ioctl error for GET NETMASK\n");
		return  -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in)); 
	memcpy(&addr, (struct sockaddr_in *)&(ifr.ifr_addr), sizeof(struct sockaddr_in));
	mask_address = inet_ntoa(addr.sin_addr);
	n_IP = ntohl(addr.sin_addr.s_addr); 	 		 
	memcpy(mask_addr,(char *)&n_IP, 4);	 

	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
		printf("ioctl error for GET HWADDR \n");
		return  -1;
	}

	memcpy(&sa, &ifr.ifr_addr, sizeof(struct sockaddr_in));
	memcpy(mac, sa.sa_data, sizeof(mac));

	if (devMAC != NULL) {
		sprintf(devMAC,  "%X:%X:%X:%X:%X:%X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	close(sockfd);
	return 0;
}



void wifi_realtime_check_again(void)
{   
    int success = 0;
    char gatway[128] = {0};
    get_gateway_addr(gatway, "wlan0");
    printf("gateway: %s\n", gatway);
    network_wan_available(gatway, &success);
    printf("success: %d\n", success);   
            
}


int network_eth_wan_available(void)
{
    int success = 0;
    char gatway[128] = {0};
    get_gateway_addr(gatway, "eth0");
    network_wan_available(gatway, &success);
    return success;
}


int check_net_cable(const char * hwname)
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


int network_check_dev(const char * hwname)
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


void network_stop_wlan0(void)
{
    self_system(CMD_STOP_UDHCPD);
    usleep(50 * 1000);  
    self_system(CMD_STOP_HOSTAPD);
    usleep(50 * 1000); 
    self_system(CMD_STOP_UDHCPC);    
    usleep(50 * 1000);
    self_system(CMD_STOP_WLAN_WPA); 
    usleep(50 * 1000);
    self_system(CMD_WLAN_DONE);
    usleep(50 * 1000);
}

void network_reset_wlan0(void)
{
    self_system(CMD_WLAN_DONE);
    usleep(50 * 1000);
    self_system(CMD_WLAN_UP);
    usleep(50 * 1000);
}

void network_start_wlan0(void)
{
    self_system(CMD_START_WLAN_WPA);
    sleep(1);
    self_system(CMD_START_WLAN_UDHCPC);
}


void network_reset_eth(void)
{
    self_system(CMD_STOP_ETH);
    usleep(50 * 1000); 
    self_system(CMD_START_ETH);
    usleep(50 * 1000); 
}

void network_stop_eth(void)
{
    self_system(CMD_STOP_UDHCPC);
    usleep(50 * 1000); 
    self_system(CMD_STOP_ETH);
    usleep(50 * 1000); 
}

void network_start_eth(void)
{
    self_system(CMD_START_ETH_UDHCPC);
    usleep(50 * 1000); 
}


int get_file_size(const char * file_name)
{
    int ret = 0;
    FILE * fp = NULL;
    unsigned int filesize = 0;
    char buf[128] = {0};
	fp = fopen(file_name, "rb");
	if (fp == NULL ) {
		printf("open %s error!\n", file_name);
		goto finally;
	}

	fread(buf, 1, sizeof(buf) - 1, fp);
	printf("file content len: %d\n", strlen(buf));

	ret = fseek(fp, 0L, SEEK_END);
	if (ret != 0) {
		printf("fseek error!!\n");
		goto finally;
	}

	filesize = ftell(fp);
	if (filesize < 0) {
        printf("ftell fail!\n");
		goto finally;
	}
    rewind(fp);	

finally:
    if (fp) {
        fclose(fp);
    }
    return ret;
}

/* 
 * ??  ??: ?¨??????wpa_supplicant?????ó??????????????AP????????
 * ??  ??:
 * ??????:
 */
int network_is_apmode(void)
{   
    int ret = 0;
    char buf[128] = {0};
    FILE * fp = NULL;
	fp = fopen("/etc/SNIP39/wpa_supplicant.conf", "rb");
	if (fp == NULL ) {
		printf("open file error!\n");
		goto finally;
	}

	fread(buf, 1, sizeof(buf) - 1, fp);
    EMGCY_LOG("file content: %s\n", buf);
    //printf("file content: %s\n", buf);
    
finally:
    if (fp) {
        fclose(fp);
    }
    return strlen(buf);
}


/* 
 * ??  ??:
            1 ?ì?é?±?°??????ap????????
            2 ??????????????????,??????????
 * ??  ??:
 * ??????:
 */
void network_first_init(void)
{	
	int ret = 0;
	ret = network_check_dev("eth0"); 
    if (ret > 0) {
        return;
    }

	ret = network_is_apmode();			
	if (ret <= 1) {
		for (;;) {
			ret = network_is_apmode();	
			if (ret > 1) {
				sleep(10);
				return;
			}
			sleep(1);
		}	
	}

	return ;
}

#if 0
void network_first_init(void)
{   
    int ret = 0;
    ret = network_check_dev("eth0");    // 当前设备插有网线,直接退出
    if (ret > 0) {
        network_stop_eth();
        network_reset_eth();
        network_start_eth();
        return;
    }
    
    ret = network_is_apmode();          // 判断是否在ap热点模式, 声波二维码会会更新wpa_supplicant 文件
    if (ret <= 1) {
        for (;;) {
            // bug, if the condition can not be reach, always loop 
            ret = network_is_apmode();  // 等待切换wifi,一直bug,条件不成立会造成死循环
            if (ret > 1) {
                sleep(10);
                return;
            }
            sleep(1);
        }   
    }
    else {
        ret = network_check_dev("eth0");
        if (ret > 0) {
            network_stop_eth();
            network_reset_eth();
            network_start_eth();
            return;
        }
        // 可能通过ap绑定已经连上wifi
        ret = network_check_dev("wlan0");
        if (ret > 0) {
            return;
        }
        else {
            // 通过二维码或者声波绑定
            network_stop_wlan0();
            network_reset_wlan0();
            network_start_wlan0();
        }
    }
}
#endif

//#define API_TEST
#ifdef API_TEST
int main(int argc, char ** argv)
{
    int ret = 0;

#define CHECK_AP_MODE
#ifdef CHECK_AP_MODE
    while (1)
    {
        ret = network_is_apmode();
        printf("ret value: %d\n", ret);
        sleep(1);
    }
#endif 


//#define GET_GATEWAY
#ifdef GET_GATEWAY
    char buf[128] = {0};
    
    while (1)
    {       
        EMGCY_LOG("open eth0\n");
        network_stop_eth();
        network_reset_eth();
        network_start_eth();
        
        EMGCY_LOG("test eth0 gateway!\n");
        network_get_gateway(buf, "eth0");
        EMGCY_LOG("eth0 gateway: %s\n", buf);
        sleep(5);


        EMGCY_LOG("open wlan0\n");
        network_stop_wlan0();
        network_reset_wlan0();
        network_start_wlan0();
        network_get_gateway(buf, "wlan0");
        EMGCY_LOG("wlan0 gateway: %s\n", buf);    
        sleep(10);


        EMGCY_LOG("stop eth0\n");
        network_stop_eth();
        sleep(2);
//        network_reset_eth();
        EMGCY_LOG("test eth0 gateway!\n");
        network_get_gateway(buf, "eth0");
        EMGCY_LOG("eth0 gateway: %s\n", buf);
        sleep(5);


        EMGCY_LOG("stop waln0\n");
        network_stop_wlan0();
        sleep(2);
//        network_reset_wlan0();
        network_get_gateway(buf, "wlan0");
        EMGCY_LOG("wlan0 gateway: %s\n", buf);    
        sleep(5);

    }
#endif 



//#define WLAN_ON_OFF
#ifdef WLAN_ON_OFF
    EMGCY_LOG("-->\n");
    network_stop_wlan0();
    network_reset_wlan0();
    network_start_wlan0();
    sleep(20);
    

    EMGCY_LOG("-->\n");
    network_stop_wlan0();
    network_reset_wlan0();
    network_start_wlan0();

    
    EMGCY_LOG("-->\n");  
    sleep(20);
    network_stop_wlan0();
    network_reset_wlan0();
    network_start_wlan0();


    EMGCY_LOG("-->\n");
    sleep(20);
    network_stop_wlan0();
    network_reset_wlan0();
#endif 


//#define WLAN_LIN_STATUS
#ifdef WLAN_LIN_STATUS
    printf("@@@@ WLAN_LIN_STATUS test start\n");
    network_stop_wlan0();
    network_reset_wlan0();
    // ?????÷???????????±?ò??×è???????±??,?????????±????????????IP,?????á×????????ó?¨
    network_start_wlan0();
    printf("@@@@ WLAN_LIN_STATUS API start\n");
    
    while (1)
    {
        ret = check_wifi_link();
        EMGCY_LOG("wifi link status: %d\n", ret);
        sleep(1);
    }
#endif 


//#define ETH_WAN_TEST
#ifdef ETH_WAN_TEST
    network_reset_eth();
    network_start_eth();
    while (1)
    {
        ret = network_eth_wan_available();
        EMGCY_LOG("eth0 link status: %d\n", ret);
        if (ret) {
            EMGCY_LOG("eth0 can connect internet!\n");
        }
        else {
            EMGCY_LOG("eth0 can not connect internet!\n");
        }
        sleep(1); 
    }
#endif 



//#define ETH_WAN_LINK
#ifdef ETH_WAN_LINK
    network_reset_eth();
    network_start_eth();
    while (1)
    {
        ret = network_eth_link_statsu();
        if (ret < 0) {
            EMGCY_LOG("eth0 unlink\n");
        }
        else {
            EMGCY_LOG("eth0 linked\n");
        }
        sleep(1); 
    }
#endif 


//#define TEST_ETH
#ifdef TEST_ETH
    EMGCY_LOG("@@@@@ start eth\n");
    network_reset_eth();
    network_start_eth();

    sleep(10);
    EMGCY_LOG("@@@@@ stop eth\n");
    network_stop_eth();
    return;
#endif 


//#define NET_ADAPTDER
#ifdef NET_STATUS   

    // ?????ì??????×???
    int success = 0;
    while (1)
    {   
        memset(buf, '\0', sizeof(buf));
        get_gateway_addr(buf, "eth0");
        EMGCY_LOG("eth0 dateway: %s\n", buf);        
        sleep(1);

        memset(buf, '\0', sizeof(buf));
        get_gateway_addr(buf, "wlan0");
        EMGCY_LOG("wlan0 dateway: %s\n", buf);        
        sleep(1);
        
        ret = network_check_dev("eth0");
        EMGCY_LOG("eth0 status: %d\n", ret);
        sleep(1);

        wifi_realtime_check_again();
    }
#endif 



//#define NET_ADAPTDER
#ifdef NET_ADAPTDER
    int net_status = 0;
    int try_eth_count = 0;
    int try_eth_times = 10;
    unsigned int loop_times = 0;
    int eth_bad_count = 0;
    int wlan_bad_count = 0;
    int first_init = 1;

    network_stop_wlan0();
    network_reset_wlan0();
    network_start_wlan0();

    network_stop_eth();
    network_reset_eth();
    network_start_eth();

    while (1) 
    {
        // ?????ì?????ü??????×???
        ret = network_check_dev("eth0");
        if (ret) {                                  // ?????????????è±?
            if (loop_times % ETH_LINK_CHECK_TIME == 0) {                  
                ret = network_eth_link_statsu();    
                if (ret != 1) {                              // ?±?°????????????ping?¨
                    eth_bad_count ++;
                    EMGCY_LOG("eth_bad_count value: %d\n", eth_bad_count);
                    if (eth_bad_count % ETH_NET_CHECK_TIME == 0) {
                        network_stop_wlan0();
                        network_stop_eth();
                        network_reset_eth();
                        network_start_eth();
                        eth_bad_count = 0;
                    }
                }       
            }
        }
        else {                                  // ?????????è±?,????????wifi                 
            if (first_init) {
                ret = check_wifi_link();
                if (ret != 1) {                 // ????????????????
                    EMGCY_LOG("first init wlan0...\n");
                    network_stop_wlan0();
                    network_reset_wlan0();
                    network_start_wlan0();  
                }
                first_init = 0;
            }
            
            if (loop_times % WLAN_LINK_CHECK_TIME == 0) {                  
                ret = check_wifi_link();    
                if (ret != 1) {                              // ?±?°????????????ping?¨
                    wlan_bad_count ++;
                    EMGCY_LOG("wlan_bad_count value: %d\n", wlan_bad_count);
                    if (wlan_bad_count % WLAN_NET_CHECK_TIME == 0) {
                        network_stop_wlan0();
                        network_reset_wlan0();
                        network_start_wlan0();
                        wlan_bad_count = 0;
                    }
                }       
            }
        }
        
        loop_times ++;
        EMGCY_LOG("loop_times value: %d\n", loop_times);
        sleep(NET_ADAPTDER_INTER_TIME);
    }
#endif 


	return 0;
}
#endif

