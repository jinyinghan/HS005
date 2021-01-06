/****************************************************************************************************************
 * filename     isvpush.h
 * describe     Sunvalley push sdk api define
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ***************************************************************************************************************/
#ifndef _I_SV_PUSH_H_
#define _I_SV_PUSH_H_
#include <sys/types.h>
typedef enum { //event type
    E_SV_EVENT_TYPE_NONE                                =   0x00,
    //connect
    E_SV_EVENT_TYPE_RTMP_INVALID_URL                    =   0x01,
    E_SV_EVENT_TYPE_RTMP_CONNECT_DNS_RESOVLE_FAILED     =   0x02,// Dns parser failed, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_SOCKET_CONNECT_FAILED          =   0x03,// Create socket connection failed, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_CONNECT_HANDSHAKE_FAILED       =   0x04,// Handshake failed, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_CONNECT_APP_FAILED             =   0x05,// Connected app failed, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_PUBLISH_STREAM_FAILED          =   0x06,// Publish stream failed, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_CONNECT_SET_TIMEOUT_FAILED     =   0x07,// Set timeout failed
    E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE                  =   0x08,// Close rtmp connection
    E_SV_EVENT_TYPE_RTMP_CONNECT_SUCCESS                =   0x09,// Rtmp connection success, wparam: 0, lparam: 0

    // logic
    E_SV_EVENT_TYPE_RTMP_INVALID_PARAMETER              =   0x50,// Invalid parameter
    E_SV_EVENT_TYPE_RTMP_AUDIO_ADTS_DATA_ERROR          =   0x51,// Adta data failed,  wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_VIDEO_START_CODE_ERROR         =   0x52,// Video data parser error
    E_SV_EVENT_TYPE_RTMP_RECV_AUDIO_DATA_ERROR          =   0x53,// Audio data parser error
    E_SV_EVENT_TYPE_RTMP_SEND_VIDEO_NO_SPS_PPS          =   0x54,// No sps and pps data, wparam: 0, lparam: 0
    E_SV_EVENT_TYPE_RTMP_SEND_ERROR_SPS_PPS             =   0x55,// error sps pps, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_SEND_ERROR_SPS                 =   0x56,// error sps, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_SEND_ERROR_PPS                 =   0x57,// error pps, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH              =   0x58,// Start media push
    E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH               =   0x59,// Pause media push(this error always due to frame buffer queue full)
    //E_SV_EVENT_TYPE_RTMP_DISCONNECT_WITH_CAMERA         =   0x59,// Cloud push sdk network connection disconnect with camera
    E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR    =   0x60,// Rtmp media data send thread exit with error reason, wparam: error code, lparam: 0
    E_SV_EVENT_TYPE_RTMP_SEND_ERROR_VPS                 =   0x61,// error vps, wparam: error code, lparam: 0
    // control event
    E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_OPEN           =   0x100,// echoshow notify device to open liver push, wparam:0, lparam devicesn:string ptr
    E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_CLOSE          =   0x101, // echoshow notify device to close live push, wparam:0, lparam devicesn:string ptr

    E_SV_EVENT_TYPE_PUSH_SEGMENT_END                    = 0x102 // segment push end, wparam: segment video duration, lparam: segment audio duration(in million seconds)
} E_SV_EVENT_TYPE;


// media data encrypt type
typedef enum 
{
    // No encrypt 
    E_SV_MEDIA_ENCRYPT_NONE                         = 0,
    // Key frame encrypt with aes 
    E_SV_MEDIA_ENCRYPT_AES_KEY_FRAME                = 1,
    // All frame encrypt with aes 
    E_SV_MEDIA_ENCRYPT_AES_ALL_FRAME                = 2
} E_SV_MEDIA_ENCRYPT_TYPE;

// metadata type
typedef enum
{
    // metadata start code
    E_SV_METADATA_TYPE_NONE                              = -1,
    // start push stream, wparam: tigger type, lparam:local timestamp ptr(int64_t*)
    E_SV_METADATA_TYPE_STREAM_START                      = 0,
    // stop push stream, wparam: alarm time(ms), lparam:local timestamp ptr(int64_t*)
    E_SV_METADATA_TYPE_STREAM_END                        = 1,
    // video encrypt mode, wparam:video enctrypt type, lparam:video entrypt string
    E_SV_METADATA_TYPE_VIDEO_ENCRYPT                     = 2,
    // audio encrypt mode, wparam:audio enctrypt type, lparam:audio entrypt string
    E_SV_METADATA_TYPE_AUDIO_ENCRYPT                     = 3,
    // stream bitrate, wparam:vbitrate, lparam:abitrate
    E_SV_METADATA_TYPE_STREAM_BITRATE                    = 4,
    // stream drop frame msg, wparam:video drop frame count, lparam:audio drop frame count
    E_SV_METADATA_TYPE_STREAM_DROP_FRAME                 = 5,
    // stream disconnect with camera, wparam: video connected, lparam: audio connected
    E_SV_METADATA_TYPE_STREAM_DISCONNECT                 = 6,
    // stream tigger type change wparam: older tigger type, lparam: new tigger type, should be send before stream end
    E_SV_METADATA_TYPE_TIGGER_TYPE_CHANGE                = 7
    
} E_SV_METADATA_TYPE;

// media data type
typedef enum
{
    E_SV_MEDIA_TYPE_UNKNOW      = -1,
    E_SV_MEDIA_TYPE_H264        = 0,
    E_SV_MEDIA_TYPE_H265        = 1,
    E_SV_MEDIA_TYPE_MP3         = 2,
    E_SV_MEDIA_TYPE_AAC         = 3,
    E_SV_MEDIA_TYPE_PRI_DATA    = 4,
    E_SV_MEDIA_TYPE_METADATA    = 5
} E_SV_MEDIA_TYPE;

// log level
typedef enum
{
	// verbose/(most detail) log level output
	E_LOG_LEVEL_VERB	= 0x01,
	// information log level output
	E_LOG_LEVEL_INFO	= 0x02,
	// main log level output
	E_LOG_LEVEL_MAIN	= 0x03,
	// warning log level output
	E_LOG_LEVEL_WARN	= 0x04,
	// error log level output
	E_LOG_LEVEL_ERROR	= 0x05,
	// disabe all log level output
	E_LOG_LEVEL_DISABLE	= 0x06
} E_SV_LOG_LEVEL;

// packet type define
#define SV_PACKET_TYPE_DEFAULT              0
#define SV_PACKET_TYPE_KEY_FRAME            1
#define SV_PACKET_TYPE_AES_ENCRYPT          2

// tigger type
typedef enum
{
    // unknown tigger type
    E_IPC_TIGGER_TYPE_UNKNOW    = -1,

    // human detection tigger
    E_IPC_TIGGER_TYPE_HUMAN     = 0,

    // moving detection tigger type default, hs002 is movement detection, hs003 and hs004 is human detection
    E_IPC_TIGGER_TYPE_DEFAULT   = 1,

    // face detection tigger
    E_IPC_TIGGER_TYPE_FACE      = 2,

    // human and face detection tigger
    E_IPC_TIGGER_TYPE_HUMAN_AND_FACE = 3,

    // reserved1 detection tigger
    E_IPC_TIGGER_TYPE_MOVE_DETECT = 4,

    // reserved2 detection tigger
    E_IPC_TIGGER_TYPE_RESERVED = 5,
    
    //live tigger type
    // debug tool live tigger type,
    E_IPC_TIGGER_TYPE_LIVE           = 100,

    // ios app client tigger
    E_IPC_TIGGER_TYPE_IOS_APP   = 101,

    // android app client tigger
    E_IPC_TIGGER_TYPE_ANDROID_APP = 102,

    // alexa echoshow tigger
    E_IPC_TIGGER_TYPE_ALEXA     = 103,

    // web tigger
    E_IPC_TIGGER_TYPE_WEB       = 104,

    // google assintant tigger
    E_IPC_TIGGER_TYPE_GOOGLE_ASSISTANT = 105

} E_SV_TIGGER_TYPE;

#define NEW_PUSH_SDK_API
// log output mode defines（如：SV_LOG_OUTPUT_MODE_CONSOLE&SV_LOG_OUTPUT_MODE_FILE）
#define SV_LOG_OUTPUT_MODE_NONE         0
#define SV_LOG_OUTPUT_MODE_CONSOLE      1
#define SV_LOG_OUTPUT_MODE_FILE         2

#define INVALID_CONNECTION_ID_VALUE     -1
typedef int(*event_callback)(long lCID, long eventType, long wparam, long lparam);
//#ifndef __cplusplus
//extern "C"{
//#endif
#ifdef __cplusplus
extern "C"{
#endif
/*****************************************************************************************************************
 * @describe Init resource once when loading sdk
 * @Param1 nMemory_Pool_Size : memory pool size in bytes, it must be big engough for one rtmp connection use
 * @Param2 nMax_Connection_Count : max push connection count,
 * @return Success return 0 , else return -1
 * @Remark Total use memory : nMemory_Pool_Size * nMax_Connection_Count
***************************************************************************************************************/
#ifdef NEW_PUSH_SDK_API
int SVPush_API_Initialize(int nMemory_Pool_Size, int nMax_Connection_Count);
#else
int SVPush_API_Initialize();
#endif

/***************************************************************************************************************
 * @describe Init log module
 * @param1 eLogLevel : Log level, see enum E_SV_LOG_LEVEL define above
 * @param2 nlogflag : Log flags, see log output mode defines above
 * @param3 plogpath : Log path, if log flags == SV_LOG_OUTPUT_MODE_FILE
 * @return Success return 0 , else return -1
 * @remark When nLogFlag&SV_LOG_OUTPUT_MODE_FILE == 0, file path can be null.
***************************************************************************************************************/
    
int SVPush_API_Init_log(E_SV_LOG_LEVEL eLogLevel, unsigned int  nLogFlag, const char* pLogPath);

/***************************************************************************************************************
 * @describe Init fetch token server Interface
 * @param1 ptoken_server_url :token server url in http format
 * @return Success return 0 , else return -1
***************************************************************************************************************/
    
int SVPush_API_Set_Token_Server_Interface(const char* ptoken_server_url);

/***************************************************************************************************************
 * @describe Deinit resource when sdk upload
 * @return Success return 0 , else return -1
 *************************************************************************************************************/
int SVPush_API_UnInitialize();

/**
 * @describe Connect to Sunvalley cloud restore streaming media server and read to push media stream
 * @param1 Url : Sunvalley streaming media server push url
 * @param2 pCallbackFun : Callback funtion of which will be call and notify when event happened
 * @return Success return connected lcid, else return < 0
 */
long SVPush_API_Connect(const char* Url, const char* token, event_callback pcbFun);

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server by app key and app stream name
 * @param       plientid : Sunvalley client id
 * @param       plientsecret : Sunvalley client secret
 * @param       pdeviceSN : Sunvalley deivec SN
 * @param       pEvenHandle : SDK callback message event handle, implement see IEventNotify
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
long SVPush_API_Connect_By_DeviceSN(const char* pclientid, const char* pclientsecret, const char* pdeviceSN, event_callback pcbfun);

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server by device token
 * @param       pdevice_token : Sunvalley device token
 * @param       pcbfun : SDK callback message event handle, implement see IEventNotify
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
long SVPush_API_Connect_By_Device_Token(const char* pdevice_token, event_callback pcbfun);

/**
 * @describe Connect to Sunvalley cloud restore streaming media server and read to push media stream
 * @param1 ptoken_url : Sunvalley live streaming media server push url
 * @param       pdeviceSN : Sunvalley deivec SN
 * @return Success return connected lcid, else return < 0
 */
long SVPush_API_Live_Connect(const char* ptoken_url, const char* pdeviceSN, event_callback pcbfun);

/***************************************************************************************************************
 * @describe Send media packet data thought sunvalley push sdk
 * @param lcid : Connected ID return by SVPush_API_Send_Packet
 * @param eMediaType : media data type
 * @param pData : Media data buffer
 * @param nSize : Media data buffer size
 * @param llTimeStamp : Pts of media data
 * @param keyframe : 0:not keyframe, 1:keyframe
 * @param encflag : 0: not encrypt, 1 aes encrypt
 * @return Success return 0 , else return -1
 * @remark
 * The data sequence is:
 * SPS
 * 000000016742802995A014016E40
 * PPS
 * 0000000168CE3880
 * IFrame
 * 0000000165B8041014C038008B0D0D3A071.....
 * PFrame
 * 0000000141E02041F8CDDC562BBDEFAD2F.....
 * User can send the SPS+PPS, then each frame:
 * SPS+PPS
 * h264_write_raw_frames('000000016742802995A014016E400000000168CE3880', size, stamp)
 * IFrame
 * h264_write_raw_frames('0000000165B8041014C038008B0D0D3A071......', size, stamp)
 * PFrame
 * h264_write_raw_frames('0000000141E02041F8CDDC562BBDEFAD2F......', size, stamp)
***************************************************************************************************************/
    
int SVPush_API_Send_Packet(long lcid, E_SV_MEDIA_TYPE eMediaType, char* pData, unsigned int uSize, int64_t llTimeStamp, int keyframe, int encflag);

/***************************************************************************************************************
 * @describe Send metadata thought sunvalley push sdk
 * @param lcid : Connected ID return by SVCloudPush_API_Connect
 * @param eMetadataType : metadata type specify enum E_SV_METADATA_TYPE
 * @param wParam : Explain param, such as error code of message
 * @param lParam : Message value, maybe ptr of special type, this value's type decides to message type
 * @return Success return 0 , else return -1
***************************************************************************************************************/
    
int SVPush_API_Send_Metadata(long lcid, E_SV_METADATA_TYPE eMetadataType, long wParam, long lParam);

/***************************************************************************************************************
 * @describe Current rtmp is connected
 * @param lcid : Connected ID return by SVCloudPush_API_Connect
 * @return Connected return 1 , else return 0
***************************************************************************************************************/

int SVPush_API_Is_Connected(long lcid);

/***************************************************************************************************************
 * @describe close push connection
 * @param lcid : connected ID return by SVCloudPush_API_Connect
 * @return Success return 0 , else return -1
***************************************************************************************************************/
    
int SVPush_API_Close(long lcid);

/***************************************************************************************************************
 * @describe Get push sdk version info
 * @param szVersionXml : Data buffer to receive version info
 * @param len : Length of data buffer
 * @return Success return version xml info length, else return <= 0
***************************************************************************************************************/

int SVPush_API_Version(char* szVersionXml, int len);

#ifdef __cplusplus
}
#endif
#endif

