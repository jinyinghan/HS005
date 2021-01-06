#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>

#include "rtspservice.h"
#include "rtputils.h"
#include "rtsputils.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SAVE_NALU 1
typedef struct
{
    /**//* byte 0 */
    unsigned char u4CSrcLen:4;      /**//* expect 0 */
    unsigned char u1Externsion:1;   /**//* expect 1, see RTP_OP below */
    unsigned char u1Padding:1;      /**//* expect 0 */
    unsigned char u2Version:2;      /**//* expect 2 */
    /**//* byte 1 */
    unsigned char u7Payload:7;      /**//* RTP_PAYLOAD_RTSP */
    unsigned char u1Marker:1;       /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short u16SeqNum;
    /**//* bytes 4-7 */
    unsigned long u32TimeStamp;
    /**//* bytes 8-11 */
    unsigned long u32SSrc;          /**//* stream number is used here. */
} StRtpFixedHdr;

typedef struct
{
    //byte 0
    unsigned char u5Type:5;
    unsigned char u2Nri:2;
    unsigned char u1F:1;
} StNaluHdr; /**/ /* 1 BYTES */

typedef struct
{
    //byte 0
    unsigned char u5Type:5;
    unsigned char u2Nri:2;
    unsigned char u1F:1;
} StFuIndicator; /**/ /* 1 BYTES */

typedef struct
{
    //byte 0
    unsigned char u5Type:5;
    unsigned char u1R:1;
    unsigned char u1E:1;
    unsigned char u1S:1;
} StFuHdr; /**/ /* 1 BYTES */


typedef struct
{
    //byte 0
	unsigned char u6LayerId_h:1;
    unsigned char u6Type:6;
    unsigned char u1F:1;
	//byte 1
	 unsigned char u3TID:3;
	unsigned char u6LayerId_l:5;
	
} H265_StNaluHdr; /**/ /* 2 BYTES */

typedef struct
{
    //byte 0
	unsigned char u6LayerId_h:1;
    unsigned char u6Type:6;
    unsigned char u1F:1;
	//byte 1
	 unsigned char u3TID:3;
	unsigned char u6LayerId_l:5;
}H265_StFuIndicator; /**/ /* 2 BYTES */

typedef struct
{
    //byte 0
    unsigned char u6Type:6;
    unsigned char u1E:1;
    unsigned char u1S:1;
}H265_StFuHdr; /**/ /* 1 BYTES */


typedef struct _tagStRtpHandle
{
    int                 s32Sock;
    struct sockaddr_in  stServAddr;
    unsigned short      u16SeqNum;
    unsigned long long        u32TimeStampInc;
    unsigned long long        u32TimeStampCurr;
    unsigned long long      u32CurrTime;
    unsigned long long      u32PrevTime;
    unsigned int        u32SSrc;
    StRtpFixedHdr       *pRtpFixedHdr;
    StNaluHdr           *pNaluHdr;
    StFuIndicator       *pFuInd;
    StFuHdr             *pFuHdr;

    H265_StNaluHdr           *pH265NaluHdr;
    H265_StFuIndicator       *pH265FuInd;
    H265_StFuHdr             *pH265FuHdr;
    EmRtpPayload        emPayload;
#ifdef SAVE_NALU
    FILE                *pNaluFile;
#endif
} StRtpObj, *HndRtp;
unsigned int local_ip=0;
unsigned long server_port = RTP_DEFAULT_UDP_PORT;

/**************************************************************************************************
**
**
**
**************************************************************************************************/
unsigned int RtpCreate(unsigned int u32IP, int s32Port, EmRtpPayload emPayload)
{
    HndRtp hRtp = NULL;
    struct timeval stTimeval;
    struct ifreq stIfr;
	int s32Broadcast = 1;
    struct sockaddr_in addr;
	
    hRtp = (HndRtp)calloc(1, sizeof(StRtpObj));
    if(NULL == hRtp)
    {
        printf("Failed to create RTP handle\n");
        goto cleanup;
    }


    hRtp->s32Sock = -1;
    if((hRtp->s32Sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Failed to create socket\n");
        goto cleanup;
    }

    if(0xFF000000 == (u32IP & 0xFF000000))
    {
        if(-1 == setsockopt(hRtp->s32Sock, SOL_SOCKET, SO_BROADCAST, (char *)&s32Broadcast, sizeof(s32Broadcast)))
        {
            printf("Failed to set socket\n");
            goto cleanup;
        }
    }
	memset(&addr, 0, sizeof(addr));
	while(1)
	{
		addr.sin_port = htons(server_port);
	    addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		
	    hRtp->stServAddr.sin_family = AF_INET;
	    hRtp->stServAddr.sin_port = htons(s32Port);
	    hRtp->stServAddr.sin_addr.s_addr = u32IP;
	    bzero(&(hRtp->stServAddr.sin_zero), 8);
		if (bind(hRtp->s32Sock, (struct sockaddr *)&addr, sizeof(addr)))
		{
			printf("can't bind !!!!!!!!!!!!!!!!!!!!!!!!!!!");
			server_port++;
		}
		else
			break;
		
	}

    //初始化序号
    hRtp->u16SeqNum = 0;
    //初始化时间戳
    hRtp->u32TimeStampInc = 0;
    hRtp->u32TimeStampCurr = 0;

    //获取当前时间
    if(gettimeofday(&stTimeval, NULL) == -1)
    {
        printf("Failed to get os time\n");
        goto cleanup;
    }

    hRtp->u32PrevTime = stTimeval.tv_sec * 1000 + stTimeval.tv_usec / 1000;

    hRtp->emPayload = emPayload;

    //获取本机网络设备名
    strcpy(stIfr.ifr_name, "wlan0");
    if(ioctl(hRtp->s32Sock, SIOCGIFADDR, &stIfr) < 0)
    {
        //printf("Failed to get host ip\n");
        strcpy(stIfr.ifr_name, "wlan0");
        if(ioctl(hRtp->s32Sock, SIOCGIFADDR, &stIfr) < 0)
        {
            printf("Failed to get host eth0 or wlan0 ip\n");
            goto cleanup;
        }
    }

    hRtp->u32SSrc = htonl(((struct sockaddr_in *)(&stIfr.ifr_addr))->sin_addr.s_addr);
	local_ip = hRtp->u32SSrc;
    //hRtp->u32SSrc = htonl(((struct sockaddr_in *)(&stIfr.ifr_addr))->sin_addr.s_addr);
    printf("!!!!!!!!!!!!!!!!!!!!!!rtp create:addr:%x,port:%d,local%x\n",u32IP,s32Port,hRtp->u32SSrc);
#ifdef SAVE_NALU
    hRtp->pNaluFile = fopen("nalu.265", "wb+");
    if(NULL == hRtp->pNaluFile)
    {
        printf("Failed to open nalu file!\n");
        goto cleanup;
    }
#endif
    printf("<><><><>success creat RTP<><><><>\n");

    return (unsigned int)hRtp;

cleanup:
    if(hRtp)
    {
        if(hRtp->s32Sock >= 0)
        {
            close(hRtp->s32Sock);
        }

        free(hRtp);
    }

    return 0;
}
/**************************************************************************************************
**
**
**
**************************************************************************************************/
void RtpDelete(unsigned int u32Rtp)
{
    HndRtp hRtp = (HndRtp)u32Rtp;

    if(hRtp)
    {
#ifdef SAVE_NALU
        if(hRtp->pNaluFile)
        {
            fclose(hRtp->pNaluFile);
        }
#endif

        if(hRtp->s32Sock >= 0)
        {
            close(hRtp->s32Sock);
        }

        free(hRtp);
    }
}
/**************************************************************************************************
**
**
**
************************************************************************************************/

extern unsigned char vps_tmp[256];
extern unsigned char sps_tmp[256];
extern unsigned char pps_tmp[256];
extern unsigned char sei_tmp[256];

extern int vps_len;
extern int sps_len;
extern int pps_len;
extern int sei_len;

static int SendNalu264(HndRtp hRtp, char *pNalBuf, int s32NalBufSize)
{
    char *pNaluPayload;
    char *pSendBuf;
    int s32Bytes = 0;
    int s32Ret = 0;
    struct timeval stTimeval;
    char *pNaluCurr;
    int s32NaluRemain;
    unsigned char u8NaluBytes1;
    unsigned char u8NaluBytes2;
    pSendBuf = (char *)calloc(MAX_RTP_PKT_LENGTH + 100, sizeof(char));
    if(NULL == pSendBuf)
    {
        s32Ret = -1;
        goto cleanup;
    }

    hRtp->pRtpFixedHdr = (StRtpFixedHdr *)pSendBuf;
    hRtp->pRtpFixedHdr->u7Payload   = H264;
    hRtp->pRtpFixedHdr->u2Version   = 2;
    hRtp->pRtpFixedHdr->u1Marker    = 0;
    hRtp->pRtpFixedHdr->u32SSrc     = hRtp->u32SSrc;
    //计算时间戳
    hRtp->pRtpFixedHdr->u32TimeStamp = htonl(hRtp->u32TimeStampCurr * (90000 / 1000));
    //printf("timestamp:%lld\n",hRtp->u32TimeStampCurr);
    if(gettimeofday(&stTimeval, NULL) == -1)
    {
        printf("Failed to get os time\n");
        s32Ret = -1;
        goto cleanup;
    }

    //保存nalu首byte
    u8NaluBytes1= *(pNalBuf+4);
    //设置未发送的Nalu数据指针位置
    //pNaluCurr = pNalBuf + 5;//for h264
    //设置剩余的Nalu数据数量
    s32NaluRemain = s32NalBufSize;//for h264
	//if ((u8NaluBytes&0x1f)==0x7&&0) //for h264
	pNaluPayload = (pSendBuf + 12);//调过rtp前12字节的内容开始填充nalu数据
    pNaluCurr = pNalBuf;//for h265 6 -4 =2   
	if ((u8NaluBytes1&0x40)==0x40)//0x40 for h265
	{

		if(vps_len>0)
		{			
		    hRtp->pRtpFixedHdr->u16SeqNum   = htons(hRtp->u16SeqNum ++);
	        memcpy(pNaluPayload, vps_tmp, vps_len);//填充vps h265 比h264多了个vps
	        //printf("-----1111in SendNalu264 send vps------\n");
	        if(sendto(hRtp->s32Sock, pSendBuf, vps_len+12, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
	        {
	            s32Ret = -1;
	            goto cleanup;
	        }			
		    s32NaluRemain = s32NaluRemain - vps_len -4;//for h265 
		    pNaluCurr = pNaluCurr + 4 + vps_len;//for h265 6 -4 =2           	
		}
		
		if(sps_len>0)
		{	
        	hRtp->pRtpFixedHdr->u16SeqNum   = htons(hRtp->u16SeqNum ++);        
	        memcpy(pNaluPayload, sps_tmp, sps_len);//填充sps
	        //printf("-----2222in SendNalu264 send sps------\n");
	        if(sendto(hRtp->s32Sock, pSendBuf, sps_len+12, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
	        {
	            s32Ret = -1;
	            goto cleanup;
	        }
		    s32NaluRemain = s32NaluRemain - sps_len -4;//for h265 
		    pNaluCurr = pNaluCurr + 4 + sps_len;//for h265 6 -4 =2           	
		}
		if(pps_len>0)//.填充pps
		{			
        	hRtp->pRtpFixedHdr->u16SeqNum   = htons(hRtp->u16SeqNum ++);
	        memcpy(pNaluPayload, pps_tmp, pps_len);
			// printf("-----3333in SendNalu264 send pps------\n");		
	        if(sendto(hRtp->s32Sock, pSendBuf, pps_len+12, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
	        {
	            s32Ret = -1;
	            goto cleanup;
	        }
		    s32NaluRemain = s32NaluRemain - pps_len -4;//for h265 
		    pNaluCurr = pNaluCurr + 4 + pps_len;//for h265 6 -4 =2    
		}
		if(sei_len>0)//.填充sei
		{	
        	hRtp->pRtpFixedHdr->u16SeqNum   = htons(hRtp->u16SeqNum ++);        
	        memcpy(pNaluPayload,sei_tmp, sei_len);
			// printf("-----4444in SendNalu264 send sei------\n");	
	        if(sendto(hRtp->s32Sock, pSendBuf, sei_len+12, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
	        {
	            s32Ret = -1;
	            goto cleanup;
	        }
		    s32NaluRemain = s32NaluRemain - sei_len -4;//for h265 
		    pNaluCurr = pNaluCurr + 4 + sei_len;//for h265 6 -4 =2   
		}

		
	}
    //NALU包小于等于最大包长度，直接发送
    u8NaluBytes1 = *(pNaluCurr+4) ;
	u8NaluBytes2 = *(pNaluCurr+5) ;
	s32NaluRemain = s32NaluRemain - 6;
	pNaluCurr=pNaluCurr+6;
    if(s32NaluRemain <= MAX_RTP_PKT_LENGTH)
    {
        hRtp->pRtpFixedHdr->u1Marker    = 1;
        hRtp->pRtpFixedHdr->u16SeqNum   = htons(hRtp->u16SeqNum ++);
        //hRtp->pNaluHdr                  = (StNaluHdr *)(pSendBuf + 12);
        //hRtp->pNaluHdr->u1F             = (u8NaluBytes & 0x80) >> 7;/*set u1F bit[0] */
        //hRtp->pNaluHdr->u2Nri           = (u8NaluBytes & 0x60) >> 5; /*0x60:0110 0000set u1F bit[1-2] */
        //hRtp->pNaluHdr->u5Type          = u8NaluBytes & 0x1f;/*0001 1111*/

		 hRtp->pH265NaluHdr                  = (H265_StNaluHdr *)(pSendBuf + 12);//第一字节填f, Type
        hRtp->pH265NaluHdr->u1F             = (u8NaluBytes1 & 0x80)>>7  ;/*0 bit[0]*/
        hRtp->pH265NaluHdr->u6Type =(u8NaluBytes1& 0x7E)>>1; //(u8NaluBytes >>1)& 0x3f;/*def:0x3f,0x7e: 0111 1110 bit[1-6]*/
		//填第二byte :layid 、TID入hRtp->pH265NaluHdr
		 hRtp->pH265NaluHdr->u6LayerId_h= 0;
		 hRtp->pH265NaluHdr->u6LayerId_l= 0;
		 hRtp->pH265NaluHdr->u3TID= 0x01;

		//hRtp->pH265NaluHdr                  = (H265_StNaluHdr *)(pSendBuf + 14);//add
        pNaluPayload = (pSendBuf + 13);//13
        memcpy(pNaluPayload, pNaluCurr, s32NaluRemain);//pNaluCurr内容里大小为s32NaluRemain的长度拷贝到pNaluPayload里

        s32Bytes = s32NaluRemain + 13;//13
		printf("S");
		fflush(stdout);
        if(sendto(hRtp->s32Sock, pSendBuf, s32Bytes, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
        {
            s32Ret = -1;
            goto cleanup;
        }
#ifdef SAVE_NALU
       fwrite(pSendBuf, s32Bytes, 1, hRtp->pNaluFile);
#endif
    }
    //NALU包大于最大包长度，分批发送
    else
    {

		//指定fu indicator位置
		 hRtp->pH265FuInd       = (H265_StFuIndicator *)(pSendBuf + 12);//填 第一 byte
        hRtp->pH265FuInd->u1F       = (u8NaluBytes1 & 0x80)>>7; // (u8NaluBytes & 0x00) >> 7;s
        hRtp->pH265FuInd->u6Type= 49;//

		 //填第二byte
		 hRtp->pH265FuInd->u6LayerId_h=0;
		 hRtp->pH265FuInd->u6LayerId_l =0;
		 hRtp->pH265FuInd->u3TID =u8NaluBytes2&0x07;
				

        //指定fu header位置
        hRtp->pH265FuHdr            = (H265_StFuHdr *)(pSendBuf + 14);//13
        hRtp->pH265FuHdr->u6Type =(u8NaluBytes1 & 0x7e)>>1; //(u8NaluBytes >>1)& 0x3f;/*def:0x3f, 0011 1111  ,0x7e:0111 1110*/
        hRtp->pH265FuHdr->u1S=1; //(u8NaluBytes >>1)& 0x3f;/*def:0x3f, 0011 1111  ,0x7e:0111 1110*/
        hRtp->pH265FuHdr->u1E=0; //(u8NaluBytes >>1)& 0x3f;/*def:0x3f, 0011 1111  ,0x7e:0111 1110*/


        //指定payload位置
        pNaluPayload = (pSendBuf + 15);//14
        
        //当剩余Nalu数据多于0时分批发送nalu数据
        while(s32NaluRemain > 0)
        {
            /*配置fixed header*/
            //每个包序号增1
            hRtp->pRtpFixedHdr->u16SeqNum = htons(hRtp->u16SeqNum ++);
            hRtp->pRtpFixedHdr->u1Marker = (s32NaluRemain <= MAX_RTP_PKT_LENGTH) ? 1 : 0;		
   			hRtp->pH265FuHdr->u1E= (s32NaluRemain <= MAX_RTP_PKT_LENGTH) ? 1 : 0;

            s32Bytes = (s32NaluRemain < MAX_RTP_PKT_LENGTH) ? s32NaluRemain : MAX_RTP_PKT_LENGTH;

			
            memcpy(pNaluPayload, pNaluCurr, s32Bytes);

            //发送本批次
            s32Bytes = s32Bytes + 15;//14
			printf("s");
			fflush(stdout);
            if(sendto(hRtp->s32Sock, pSendBuf, s32Bytes, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
            {
                s32Ret = -1;
                goto cleanup;
            }
#ifdef SAVE_NALU
            fwrite(pSendBuf, s32Bytes, 1, hRtp->pNaluFile);
#endif
			
			hRtp->pH265FuHdr->u1S=0; //(u8NaluBytes >>1)& 0x3f;/*def:0x3f, 0011 1111  ,0x7e:0111 1110*/

            //指向下批数据
            pNaluCurr += MAX_RTP_PKT_LENGTH;
            //计算剩余的nalu数据长度
            s32NaluRemain -= MAX_RTP_PKT_LENGTH;
				
        }
    }

cleanup:
    if(pSendBuf)
    {
        free((void *)pSendBuf);
    }
	printf("\n");
	fflush(stdout);

    return s32Ret;
}
/**************************************************************************************************
**
**
**
**************************************************************************************************/
static int SendNalu711(HndRtp hRtp, char *buf, int bufsize)
{
    char *pSendBuf;
    int s32Bytes = 0;
    int s32Ret = 0;

    pSendBuf = (char *)calloc(MAX_RTP_PKT_LENGTH + 100, sizeof(char));
    if(NULL == pSendBuf)
    {
        s32Ret = -1;
        goto cleanup;
    }
    hRtp->pRtpFixedHdr = (StRtpFixedHdr *)pSendBuf;
    hRtp->pRtpFixedHdr->u7Payload     = G711;
    hRtp->pRtpFixedHdr->u2Version     = 2;

    hRtp->pRtpFixedHdr->u1Marker = 1;   //标志位，由具体协议规定其值。

    hRtp->pRtpFixedHdr->u32SSrc = hRtp->u32SSrc;

    hRtp->pRtpFixedHdr->u16SeqNum  = htons(hRtp->u16SeqNum ++);

    memcpy(pSendBuf + 12, buf, bufsize);

    hRtp->pRtpFixedHdr->u32TimeStamp = htonl(hRtp->u32TimeStampCurr);
    //printf("SendNalu711 timestamp:%lld\n",hRtp->pRtpFixedHdr->u32TimeStamp);
    s32Bytes = bufsize + 12;
    if(sendto(hRtp->s32Sock, pSendBuf, s32Bytes, 0, (struct sockaddr *)&hRtp->stServAddr, sizeof(hRtp->stServAddr)) < 0)
    {
        printf("Failed to send!");
        s32Ret = -1;
        goto cleanup;
    }
cleanup:
    if(pSendBuf)
    {
        free((void *)pSendBuf);
    }
    return s32Ret;
}
/**************************************************************************************************
**
**
**
**************************************************************************************************/
	
	FILE* tmp_file;
	int first_in = 1;

unsigned int RtpSend(unsigned int u32Rtp, char *pData, int s32DataSize, unsigned int u32TimeStamp)
{
    int s32NalSize = 0;
    char *pNalBuf, *pDataEnd;
    HndRtp hRtp = (HndRtp)u32Rtp;
    unsigned int u32NaluToken;

    hRtp->u32TimeStampCurr = u32TimeStamp;

    if(_h264 == hRtp->emPayload)
    {
        pDataEnd = pData + s32DataSize;
        //搜寻第一个nalu起始标志0x01000000
        for(; pData < pDataEnd-5; pData ++)//def:-5
        {
            memcpy(&u32NaluToken, pData, 4 * sizeof(char));
            if(0x01000000 == u32NaluToken)
            {
                //标记nalu起始位置
                pData += 4;
                pNalBuf = pData;
                break;
            }
        }
        //发送nalu
        for(; pData < pDataEnd-5; pData ++)//def:-5
        {
            //搜寻nalu起始标志0x01000000，找到nalu起始位置，发送该nalu数据
            memcpy(&u32NaluToken, pData, 4 * sizeof(char));
            if(0x01000000 == u32NaluToken)
            {
                s32NalSize = (int)(pData - pNalBuf);
				/*
            	if (first_in == 1)
        		{
        			tmp_file=fopen("tmpfile.mp4","wb");
					first_in = 0;
        		}
				fwrite(pNalBuf,s32NalSize,1,tmp_file);
				fflush(tmp_file);
                if(SendNalu264(hRtp, pNalBuf, s32NalSize) == -1)
                {
                    return -1;
                }*/

                //标记nalu起始位置
                pData += 4;
                pNalBuf = pData;
            }
        }//while

        if(pData > pNalBuf)
        {
            s32NalSize = (int)(pData - pNalBuf);
			
#ifdef DUMP_STREAM
					if (first_in == 1)
					{
						tmp_file=fopen("tmpfile.mp4","wb");
						first_in = 0;
					}
					fwrite(pData,s32DataSize,1,tmp_file);
					fflush(tmp_file);
#endif			

            if(SendNalu264(hRtp, pNalBuf, s32NalSize) == -1)
            {
                return -1;
            }
        }
    }
    else if(_h264nalu == hRtp->emPayload)
    {
#ifdef DUMP_STREAM
    	if (first_in == 1)
		{
			tmp_file=fopen("tmpfile.mp4","wb");
			first_in = 0;
		}
		fwrite(pData,s32DataSize,1,tmp_file);
		printf("s=%d\n",s32DataSize);
		fflush(tmp_file);
#endif			
        if(SendNalu264(hRtp, pData, s32DataSize) == -1)
        {
            return -1;
        }
    }
    else if(_g711 == hRtp->emPayload)
    {
        if(SendNalu711(hRtp, pData, s32DataSize) == -1)
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}
/**************************************************************************************************
**
**
**
**************************************************************************************************/

#ifdef __cplusplus
}
#endif
