/*************************************************************************
	> File Name: malloc_debug.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Mon 29 Oct 2018 10:19:04 PM CST
 ************************************************************************/

#ifndef _MALLOC_DEBUG_H_
#define _MALLOC_DEBUG_H_
#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"


#define MAX_DEBUG_MEM_ITEM_LIST  (5000)
#define DPRINTK(fmt...)   \
    do {\
        if(access("/tmp/mem_dbg", F_OK) == 0) {          \
            printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
            __FILE__, \
            __FUNCTION__,  \
            __LINE__);\
            printf(fmt);\
        }\
    }while(0)


				
typedef struct {
	void * pAddr;
	int iSize;
}MEM_DEBUG_ST;



void * mem_malloc(size_t size);
void * mem_free(void * ptr);
void mem_show(void);

#endif 
