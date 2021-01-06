#ifndef __YSX_H264_UDP_H_
#define __YSX_H264_UDP_H_

int ysx_h264_udp_socket_init(unsigned short port);

int ysx_h264_udp_send(char * ip_addr, char *buf, uint32_t len);

int ysx_h264_udp_socket_uinit(void);

#endif

