#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <pthread.h>
#include "ysx_h264_udp.h"

int lan_search_sock = 0;
struct sockaddr_in lan_search_bindaddr;

int set_sock(int sock)
{
	int flag = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	
	return 0;
}



//初始化socket，在main函数中调用一次即可
//参数 ：指定本地端口
int ysx_h264_udp_socket_init(unsigned short port)
{
	int flags;

	lan_search_sock=socket(PF_INET,SOCK_DGRAM,0);
	if(lan_search_sock<0)
	{
		printf("init sock failed\n");
		lan_search_sock = 0;
		return -1;
	}
	flags = fcntl(lan_search_sock, F_GETFL);
	fcntl(lan_search_sock, F_SETFL, flags | O_NONBLOCK);

	set_sock(lan_search_sock);

	lan_search_bindaddr.sin_family=AF_INET;
	lan_search_bindaddr.sin_addr.s_addr= htonl(INADDR_ANY); 
	lan_search_bindaddr.sin_port=htons(port);

	if( bind(lan_search_sock,(struct sockaddr *)&lan_search_bindaddr,sizeof(struct sockaddr_in)) < 0)
	{
		printf("udp init err\n");
		close(lan_search_sock);
		return -2;
	}
	printf("udp init ok \n");
	return 0;
}

//指定PC端目的IP，默认端口为5555，buf为 h264帧数据； 改函数在vid pool读取缓存后调用
//ffplay -f h264 udp://127.0.0.1:5555
int ysx_h264_udp_send(char * ip_addr, char *buf, uint32_t len)
{
	int ret = 0;
	int tmp_len;
	struct sockaddr_in dst;
	dst.sin_family=AF_INET;
	//dst.sin_addr.s_addr= inet_addr("192.168.41.105"); // huawei cloud 
	dst.sin_addr.s_addr = inet_addr(ip_addr);//84.113
	dst.sin_port=htons(5555);
    tmp_len = len;

	 if(access("/tmp/h264_udp",F_OK)!=0)//文件不存在则不发送
	 {
		return ret;
	 }
	 	
	while( tmp_len > 0 )
	{
 		if( (ret = sendto(lan_search_sock, buf+(len-tmp_len), (1400>tmp_len)?tmp_len:1400 ,0,(struct sockaddr *)&dst, sizeof(dst))) < 0)
		{
			ret = -1;
			continue;
		} 
		tmp_len -= 1400;
	}
	
	return ret;
}


int ysx_h264_udp_socket_uinit(void)
{
	
	if(lan_search_sock> 0)
	{
		close(lan_search_sock);
		lan_search_sock = 0;
	}
	
	return 0;
}


