#ifndef _ECHO_SOCKET_H_
#define _ECHO_SOCKET_H_
#define LENGTH_OF_LISTEN_QUEUE                      5//20
#define SOCKET_SERVER_IP                            "192.168.1.1"
#define SOCKET_PORT_S                               (24385)
#define ysx_free(x)	\
            do{\
                if(x != NULL) {                         \
                    free(x);                    \
                    x = NULL;                   \
                }                               \
            }while(0)
//#if 1
//#define pr_dbg(fmt,args...)	\
//            do{\
//                    printf("[LOG_DEBUG]");          \
//                    printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);  \
//                    printf(fmt, ##args);\
//            }while(0)
//#else
//#define pr_dbg(fmt,args...)
//#endif
//#define pr_error(fmt,args...)	\
//            do{\
//                    printf("[LOG_ERROR]");          \
//                    printf("FILE:%s FUNC:%s LINE:%d ",__FILE__,__FUNCTION__,__LINE__);  \
//                    printf(fmt, ##args);\
//            }while(0)

typedef enum{
	AP_COMPANY_MIN          = -1,
	AP_COMPANY_YSX,
	AP_COMPANY_GT,
	AP_COMPANY_XST,
	AP_COMPANY_AL,
	AP_COMPANY_NAD,
	AP_COMPANY_MAX,
}e_ysx_ap_comapny;

typedef enum{
	AP_PROJECT_MIN          = -1,
    AP_PROJECT_DG201,
    AP_PROJECT_BATTERY,
	AP_PROJECT_C201,
	AP_PROJECT_G201,
	AP_PROJECT_S201,
	AP_PROJECT_MAX,
}e_ysx_ap_project;

typedef enum{
	AP_RESULT_MIN           = -1,
	AP_RESULT_SUCCESS,
	AP_RESULT_FAILD,
	AP_RESULT_MAX,
}e_ysx_ap_result;

typedef enum{
    BUFF_LEN_SSID           = 0x20,
    BUFF_LEN_PWD            = 0x20,
    BUFF_LEN_UID            = 0x15,
    BUFF_LEN_SOCKET         = 0x400,
    BUFF_LEN_RESERVED       = 0xa,
}buff_len_e;

typedef struct {
    unsigned char ssid[BUFF_LEN_SSID];              //WiFi ssid
    unsigned char password[BUFF_LEN_PWD];           //if exist, WiFi password
    unsigned char mode;                             //refer to ENUM_AP_MODE
    unsigned char enctype;                          //refer to ENUM_AP_ENCTYPE
}s_ysx_ap_socket_receive;

typedef struct
{
    unsigned char result;                           //0: success 1: fail -1: undefined
    unsigned char uid[BUFF_LEN_UID];
    unsigned char res[BUFF_LEN_RESERVED];
}s_ysx_ap_socket_send;

#endif
