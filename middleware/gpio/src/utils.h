/*************************************************************************
	> File Name: utils.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: 2018年09月22日 星期六 14时05分24秒
 ************************************************************************/

#ifndef _UTILS_H_
#define _UTILS_H_


#define EMERGENCY_DEBUG
#if defined  EMERGENCY_DEBUG
#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"
#define EMGCY_LOG(fmt...)   \
	        do {\
				printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0)  
#else
#define log_err(fmt...)   \
	        do {\
				printf("[E] [%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0) 
			
#define log_msg(fmt...)   \
	        do {\
				printf("{M] [%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0) 		
#endif 






#endif 
