/*
	文件名  ppcs_protocal.c
	描述 此文件 实现了  尚云p2p中的基础服务提供给 泽宝p2p传输协议服务《IPC与APP交互协议V1.0.2.pdf》
	参考尚云sdk和文档实现《P2P_Release_SDK_PPCS_4.0.0_20200206》
*/
#include <arpa/inet.h>
#include <pthread.h>
#include "PPCS_API.h"
#include "ppcs_service.h"
#include "log.h"

typedef struct
{
    int SessionID;
    int Channel;
} st_Session_CH;

#define SIZE_DID 			128	// Device ID Size
#define SIZE_APILICENSE 	24	// APILicense Size
#define SIZE_INITSTRING 	256	// InitString Size

#define REV_BUF_SIZE 4096
#define AV_CHECK_BUFFER (384*1024)

char DIDString[SIZE_DID] = {"VATE-000049-ETPEG"};//{"VATE-000029-ZMMEW"};//
char APILicense[SIZE_APILICENSE] = {"RJFUM:qULbseJh"};//:crckey //{"DTWPIC:qULbseJh"};//
char InitString[SIZE_INITSTRING]
	={"EEGDFHBIKAJMGAJPEOGDFBEOHLMKHEJKHFBJABCNFFMBKPKHCLFJCCKAGLONJMPBBDMJOICNKCJABCHBIBJMNHABMNLPFIHCAOGNCBEMNALDEKDOAKBHPAJHGJ"};

static pthread_t ppcs_thread;//ppcs监听线程

const char *getP2PErrorCodeInfo(int err)
{
    switch (err)
    {
        case 0: return "ERROR_PPCS_SUCCESSFUL";
        case -1: return "ERROR_PPCS_NOT_INITIALIZED"; // API didn't initialized
        case -2: return "ERROR_PPCS_ALREADY_INITIALIZED";
        case -3: return "ERROR_PPCS_TIME_OUT";
        case -4: return "ERROR_PPCS_INVALID_ID";//Invalid Device ID !!
        case -5: return "ERROR_PPCS_INVALID_PARAMETER";
        case -6: return "ERROR_PPCS_DEVICE_NOT_ONLINE";
        case -7: return "ERROR_PPCS_FAIL_TO_RESOLVE_NAME";
        case -8: return "ERROR_PPCS_INVALID_PREFIX";//Prefix of Device ID is not accepted by Server !!
        case -9: return "ERROR_PPCS_ID_OUT_OF_DATE";
        case -10: return "ERROR_PPCS_NO_RELAY_SERVER_AVAILABLE";
        case -11: return "ERROR_PPCS_INVALID_SESSION_HANDLE";
        case -12: return "ERROR_PPCS_SESSION_CLOSED_REMOTE";
        case -13: return "ERROR_PPCS_SESSION_CLOSED_TIMEOUT";
        case -14: return "ERROR_PPCS_SESSION_CLOSED_CALLED";
        case -15: return "ERROR_PPCS_REMOTE_SITE_BUFFER_FULL";
        case -16: return "ERROR_PPCS_USER_LISTEN_BREAK";//Listen break is called !!
        case -17: return "ERROR_PPCS_MAX_SESSION";//Exceed max session !!
        case -18: return "ERROR_PPCS_UDP_PORT_BIND_FAILED";//The specified UDP port can not be binded !!
        case -19: return "ERROR_PPCS_USER_CONNECT_BREAK";
        case -20: return "ERROR_PPCS_SESSION_CLOSED_INSUFFICIENT_MEMORY";
        case -21: return "ERROR_PPCS_INVALID_APILICENSE";//API License code is not correct !!
        case -22: return "ERROR_PPCS_FAIL_TO_CREATE_THREAD";//Fail to Create Thread !!
        case -23: return "ERROR_PPCS_INVALID_DSK";
        default: return "Unknown, Not the error value of P2P!";
    }
	return ;
} // getP2PErrorCodeInfo

static void showListenErrorInfo(int ret)
{
    switch (ret)
    {
    case ERROR_PPCS_NOT_INITIALIZED: LOG("API didn't initialized\n"); break;
    case ERROR_PPCS_TIME_OUT: LOG("Listen time out, No client connect me !!\n"); break;
    case ERROR_PPCS_INVALID_ID: LOG("Invalid Device ID !!\n"); break;
    case ERROR_PPCS_INVALID_PREFIX: LOG("Prefix of Device ID is not accepted by Server !!\n"); break;
    case ERROR_PPCS_UDP_PORT_BIND_FAILED: LOG("The specified UDP port can not be binded !!\n"); break;
    case ERROR_PPCS_MAX_SESSION: LOG("Exceed max session !!\n"); break;
    case ERROR_PPCS_USER_LISTEN_BREAK: LOG("Listen break is called !!\n"); break;
    case ERROR_PPCS_INVALID_APILICENSE: LOG("API License code is not correct !!\n"); break;
    case ERROR_PPCS_FAIL_TO_CREATE_THREAD: LOG("Fail tO Create Thread !!\n"); break;
    default: LOG("%s\n", getP2PErrorCodeInfo(ret)); break;
    }
	return ;
}

static void showNetwork(st_PPCS_NetInfo NetInfo)
{
	LOG("-------------- NetInfo: -------------------\n");
	LOG("Internet Reachable     : %s\n", (NetInfo.bFlagInternet == 1) ? "YES":"NO");
	LOG("P2P Server IP resolved : %s\n", (NetInfo.bFlagHostResolved == 1) ? "YES":"NO");
	LOG("P2P Server Hello Ack   : %s\n", (NetInfo.bFlagServerHello == 1) ? "YES":"NO");
	switch(NetInfo.NAT_Type)
	{
	case 0: LOG("Local NAT Type         : Unknow\n"); break;
	case 1: LOG("Local NAT Type         : IP-Restricted Cone\n"); break;
	case 2: LOG("Local NAT Type         : Port-Restricted Cone\n"); break;
	case 3: LOG("Local NAT Type         : Symmetric\n"); break;
    case 4: LOG("Local NAT Type         : Different Wan IP Detected!!\n", NetInfo.NAT_Type); break;
	}
	LOG("My Wan IP : %s\n", NetInfo.MyWanIP);
	LOG("My Lan IP : %s\n", NetInfo.MyLanIP);
	LOG("-------------------------------------------\n");

	return ;
}

static void dump_Sinfo(st_PPCS_Session Sinfo)
{
    char RemoteIP[16] = {};
    char *pRemoteIP = inet_ntoa(Sinfo.RemoteAddr.sin_addr);
    memcpy(RemoteIP, pRemoteIP, strlen(pRemoteIP));
    char MyLanIP[16] = {};
    char *pLanIP = inet_ntoa(Sinfo.MyLocalAddr.sin_addr);
    memcpy(MyLanIP, pLanIP, strlen(pLanIP));
	LOG("RemoteAddr=%s:%d, Mode=", RemoteIP, ntohs(Sinfo.RemoteAddr.sin_port));
    if (0 == Sinfo.bMode)
    {
        if (Sinfo.RemoteAddr.sin_addr.s_addr == Sinfo.MyLocalAddr.sin_addr.s_addr || 1 == isLANcmp(MyLanIP, RemoteIP)) LOG("LAN");
        else LOG("P2P\n");
    }
    else LOG("RLY\n");

	return ;
}

// -1:invalid parameter,0:not the same LAN Addresses,1:Addresses belonging to the same LAN.
int isLANcmp(const char *IP1, const char *IP2)
{
#define   YES   1
#define   NO   0
    short Len_IP1 = strlen(IP1);
    short Len_IP2 = strlen(IP2);
    if (!IP1 || 7 > Len_IP1 || !IP2 || 7 > Len_IP2) return -1;
    if (0 == strcmp(IP1, IP2)) return YES;
    const char *pIndex = IP1+Len_IP1-1;
    while (1)
    {
        if ('.' == *pIndex || pIndex == IP1) break;
        else pIndex--;
    }
    if (0 == strncmp(IP1, IP2, (int)(pIndex-IP1))) return YES;
    return NO;
}


static void *ppcs_cmd_service_thread(void *arg)
{
	int ret;
	INT32 SessionID;
	INT32 Channel = 0;// 命令通道 就是Channel 0
	unsigned char buf[REV_BUF_SIZE] = {0};
	INT32 ReadSize = REV_BUF_SIZE-1; // ReadSize: 期望要读取的数据大小，在每次 PPCS_Read 之前 ReadSzie 必须指定要读取的大小，非常重要!!
	INT32 TimeOut_ms = 200;
	int iGNo = -1;
	
	if(!arg)
	{
		LOG("ThreadRead exit for arg is NULL!!\n");
		goto quit_now;
	}
	SessionID = *(int *)arg;
	free(arg);
	
    if (SessionID < 0)
    {
        LOG("ThreadRead exit for Invalid SessionID(%d)!!\n", SessionID);
        goto quit_now;
    }

	LOG("SessionID is Coming [ %d ]\n",SessionID); 

	//如果Session有问题 立即关闭
	st_PPCS_Session Sinfo;
	if (ERROR_PPCS_SUCCESSFUL != (ret = PPCS_Check(SessionID, &Sinfo)))
	{   // connect success, but remote session closed.
        LOG("Session=%d,RemoteAddr=Unknown (PPCS_Check:%d)\n", SessionID, ret);
        PPCS_Close(SessionID); // 不能多线程对同一个 SessionID 做 PPCS_Close(SessionID)/PPCS_ForceClose(SessionID) 的动作，否则可能导致崩溃。
        LOG("-PPCS_Close(%d).\n", SessionID);
        goto quit_now;
	}

	//打印Sinfo信息
	dump_Sinfo(Sinfo);

	//add one user
	iGNo = common_add_user(SessionID);
	if(iGNo < 0)
		goto quit_now;

	
		
	while (1)
	{
		ReadSize = REV_BUF_SIZE -1;
		memset(buf, 0, sizeof(buf));
		INT32 ret = PPCS_Read(SessionID, Channel, (char*)buf, &ReadSize, TimeOut_ms);
	// PPCS_Read 返回 ERROR_PPCS_TIME_OUT(-3) :
	// 1. PPCS_Read 返回 -3 超时错误是正常现象，只是在设定超时时间内读取不到预设的大小，并非断线错误，需继续循环读取剩余数据。
	// 2. PPCS_Read 返回 -3 超时也有可能读到部分数据，需要对 ReadSize 做大小检测判断，本 sample code 直接累加 ReadSize 大小。
	// 3. ReadSzie 是实际反映读取的数据大小，若 PPCS_Read 读取不到数据，ReadSzie 变量会被清零，下一次 PPCS_Read 之前一定要重新赋值，否则 ReadSzie 传 0 进 PPCS_Read 会返回-5 错误（-5：无效的参数）。

		if (ERROR_PPCS_INVALID_PARAMETER != ret && ERROR_PPCS_INVALID_SESSION_HANDLE != ret && ReadSize)
		{
			
			Handle_IOCTRL_Cmd(SessionID, Channel, buf, ReadSize, iGNo);
			continue;
		}
		
        if (0 > ret && ERROR_PPCS_TIME_OUT != ret)
        {
            if (ERROR_PPCS_SESSION_CLOSED_TIMEOUT == ret) // 网络差导致断线。
            {
                LOG("\nPPCS_Read  Session=%d,CH=%d,ReadSize=%d Byte,ret=%d, Session Closed TimeOUT!!\n", SessionID, Channel,ReadSize, ret);
            }
            else if (ERROR_PPCS_SESSION_CLOSED_REMOTE == ret) // 对方主动关闭连接。
            {
                LOG("\nPPCS_Read  Session=%d,CH=%d,ReadSize=%d Byte,ret=%d, Session Remote Closed!!\n", SessionID, Channel,ReadSize, ret);
            }
            else LOG("\nPPCS_Read  Session=%d,CH=%d,ReadSize=%d Byte,ret=%d [%s]\n", SessionID, Channel, ReadSize, ret, getP2PErrorCodeInfo(ret));
            break;
        }
        else if (ERROR_PPCS_TIME_OUT == ret && 0 == ReadSize) // 读取超时，非断线错误，需继续读取
        {
            continue;
        }else{}
	}

	//uninit for common_add_user
	if(iGNo>=0)
		common_cancel_user(iGNo);
	
quit_now:
	PPCS_Close(SessionID);
	
	pthread_exit(0);
}

static void *ppcs_listen_thread(void *arg)
{
	int SessionID = -99;
	int ret = -1;
	unsigned int TimeOut_Sec = 120;
	unsigned short UDP_Port = 0;// PPCS_Listen 端口填 0 让底层自动分配。
	char bEnableInternet = 1;
	pthread_t ppcs_cmd_recv_thread_pid;
	
	while(1)
	{
		ret = PPCS_Listen(DIDString, TimeOut_Sec, UDP_Port, bEnableInternet, APILicense);
		if (ret < 0)
		{
			LOG("Listen failed: %d ", ret);  showListenErrorInfo(ret);
			//sleep(5);
			continue ;
		}
	    else
		{	//每个 >=0 的 SessionID 都是一个连接，本 sample 是单用户连接范例，多用户端连接注意要保留区分每一个 PPCS_Listen >=0 的 SessionID, 
			//并且 SessionID 不用时需要 PPCS_Close(SessionID)/PPCS_ForceClose(SessionID) 关闭连线释放资源。
			LOG("### SID %d\n",ret);
			SessionID = ret; 
	    }

		int *SID = malloc(sizeof(int));//for thread arg, so malloc is more save
		if (!SID)
		{ 
			LOG("SID - malloc failed!!\n"); 
			PPCS_Close(SessionID);
			pthread_exit(0); 
		}
		*SID = SessionID;
		
		
		//启动协议传输 需要的 所有线程 根据泽宝的协议文档 目前用到前4个Channel 只有Channel 0有下行数据 其他暂时只有上行数据 暂无线程
		if (0 != pthread_create(&ppcs_cmd_recv_thread_pid, NULL, &ppcs_cmd_service_thread, (void *)SID) )
		{
			LOG("create ThreadRead failed");
		}
		pthread_detach(ppcs_cmd_recv_thread_pid);

				
	}

	printf("exit  ppcs_listen_thread \n");
	pthread_exit(0);
}

void ppcs_service_set_param(char *didCode,char *initCode,char *crcKey)
{
	char *did_APILicense;
	char *did_ptr;
	if(didCode==NULL || initCode==NULL ||crcKey==NULL)
		return ;
	
	did_ptr = strtok_r(didCode, ",", &did_APILicense);//逗号被填充 0，did_ptr 指向 didCode同一个地址，did_APILicense指向逗号后面的

	strncpy(DIDString, did_ptr, SIZE_DID);
	strncpy(InitString, initCode, SIZE_INITSTRING);
	snprintf(APILicense,SIZE_APILICENSE,"%s:%s",did_APILicense,crcKey);
	
	LOG("ppcs_service_set_param\n DIDString[%s]\n InitString[%s]\n APILicense[%s]\n",DIDString, InitString, APILicense);
	return ;
}


int ppcs_service_check_buffer(const int SessionID, const int Channel)
{
	INT32 Check_ret = 0;
	UINT32 WriteSize = 0;
	Check_ret = PPCS_Check_Buffer(SessionID, Channel, &WriteSize, NULL);
	if(Check_ret < 0)
		return -1;

	if(WriteSize >= AV_CHECK_BUFFER)
	{
		printf("@@@@@@@   network is low @#####\n");
		return -2;
	}

	return 0;
	
}


int ppcs_service_write(const int SessionID, const int Channel, const void *data, const int len)
{
    INT32 ret = 0;
    INT32 Check_ret = 0;
    UINT32 WriteSize = 0;

	Check_ret = PPCS_Check_Buffer(SessionID, Channel, &WriteSize, NULL);

	//LOG("ThreadWrite PPCS_Check_Buffer: Session=%d,CH=%d,WriteSize=%d,ret=%d %s\n", SessionID, Channel, WriteSize, Check_ret, getP2PErrorCodeInfo(Check_ret));
	if(Check_ret < 0)
	{
		LOG("\nThreadWrite PPCS_Check_Buffer: Session=%d,CH=%d,WriteSize=%d,ret=%d %s\n", \
				SessionID, Channel, WriteSize, Check_ret, getP2PErrorCodeInfo(Check_ret));
		return -1;
	}

    // 写缓存的数据大小超过128KB/256KB，则需考虑延时缓一缓。
    // 如果发现 wsize 越来越大，可能网络状态很差，需要考虑一下丢帧或将码率，这是一个动态调整策略，非常重要!!
    //if (WriteSize < 256*1024 && TotalSize < TOTAL_WRITE_SIZE)
    if (WriteSize < AV_CHECK_BUFFER)
    {
		PPCS_Write(SessionID, Channel,(char *) data,len);
	}
	else
	{
		//fix me! 
		printf("#######   network is low @#####\n");
		return -2;
		
	}
		
	return ret;
}


int ppcs_service_read(const int SessionID, const int Channel, const void *data, const int len)
{
	INT32 ReadSize = len; // ReadSize: 期望要读取的数据大小，在每次 PPCS_Read 之前 ReadSzie 必须指定要读取的大小，非常重要!!
	INT32 TimeOut_ms = 20;

	INT32 ret = PPCS_Read(SessionID, Channel, (char*)data, &ReadSize, TimeOut_ms);
	
	if (ERROR_PPCS_INVALID_PARAMETER != ret && ERROR_PPCS_INVALID_SESSION_HANDLE != ret && ReadSize)
	{
		//存在有效数据
		printf("ReadSize %d\n",ReadSize);
		return ReadSize;
	}
	
	if (0 > ret && ERROR_PPCS_TIME_OUT != ret)
	{
		if (ERROR_PPCS_SESSION_CLOSED_TIMEOUT == ret) // 网络差导致断线。
		{
			LOG("\nPPCS_Read  Session=%d,CH=%d,ReadSize=%d Byte,ret=%d, Session Closed TimeOUT!!\n", SessionID, Channel,ReadSize, ret);
		}
		else if (ERROR_PPCS_SESSION_CLOSED_REMOTE == ret) // 对方主动关闭连接。
		{
			LOG("\nPPCS_Read  Session=%d,CH=%d,ReadSize=%d Byte,ret=%d, Session Remote Closed!!\n", SessionID, Channel,ReadSize, ret);
		}
		else LOG("\nPPCS_Read  Session=%d,CH=%d,ReadSize=%d Byte,ret=%d [%s]\n", SessionID, Channel, ReadSize, ret, getP2PErrorCodeInfo(ret));

	}
	else if (ERROR_PPCS_TIME_OUT == ret && 0 == ReadSize) // 读取超时，非断线错误，需继续读取
	{
		ret = 0;
	}else{}
	

	return ret;
}


void init_ppcs_service(void)
{
	int ret;

	//step 1: 确定是否已经完成DID同步 泽宝使用的DID是通过泽宝SN获取的
	//fix me

	LOG("DID=%s\n",DIDString);
	LOG("APILicense=%s\n",APILicense);
	LOG("InitString=%s\n",InitString);
	
	//step 2: show api version
	UINT32 APIVersion = PPCS_GetAPIVersion();
	LOG("PPCS P2P API Version: %d.%d.%d.%d\n",
	 			(APIVersion & 0xFF000000)>>24,
	 			(APIVersion & 0x00FF0000)>>16,
	 			(APIVersion & 0x0000FF00)>>8,
	 			(APIVersion & 0x000000FF)>>0);


	//step 3 . P2P Initialize
	ret = PPCS_Initialize((char *)InitString);
    LOG("PPCS_Initialize(%s) done! ret=%d\n", InitString, ret);
	if (ERROR_PPCS_SUCCESSFUL != ret && ERROR_PPCS_ALREADY_INITIALIZED != ret)
	{
        LOG("PPCS_Initialize: ret=%d [%s]\n", ret, getP2PErrorCodeInfo(ret));
		return ;
	}

	//step 4. Network Detect 网络监测失败理论上应该也可以开始监听 待确认 ？
	st_PPCS_NetInfo NetInfo;
	ret = PPCS_NetworkDetect(&NetInfo, 0);
	if (0 > ret) LOG("PPCS_NetworkDetect: ret=%d [%s]\n", ret, getP2PErrorCodeInfo(ret));
	showNetwork(NetInfo);


	//step 5. 启动 监听线程
	pthread_t ppcs_thread;
    // create thread to login because without WAN still can work on LAN
    if(ret = pthread_create(&ppcs_thread, NULL, &ppcs_listen_thread, NULL))
    {
        LOG("ppcs_demo fail, ret=[%d]\n", ret);
        return ;
    }
    pthread_detach(ppcs_thread);	
	
	
	return ;
}


void uninit_ppcs_service(void)
{
	PPCS_Listen_Break();
	PPCS_DeInitialize();
	LOG("PPCS_DeInitialize() done!\n");
	return ;
}
