/*************************************************************************
	> File Name: melloc_debug.c
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Mon 29 Oct 2018 10:18:54 PM CST
 ************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "memory_debug.h"

//#define USE_MEM_DEBUG


pthread_mutex_t mem_lock;
volatile static int iAddrNum = 0;
volatile static int iTotalMallocSize = 0;
volatile static int iMallocInit = 0;
volatile static unsigned int malloc_count = 0;
volatile static unsigned int free_count = 0;

static MEM_DEBUG_ST stMemItemList[MAX_DEBUG_MEM_ITEM_LIST] = {0};


void * mem_malloc(size_t size)
{
	void * pAddr = NULL;
	int i = 0;

#ifdef USE_MEM_DEBUG
	if (iMallocInit == 0) {
		pthread_mutex_init(&mem_lock, NULL);
		iMallocInit = 1;
		iTotalMallocSize = 0;
		memset(&stMemItemList, '\0', sizeof(MEM_DEBUG_ST) * 
		MAX_DEBUG_MEM_ITEM_LIST);
	}
	
	pthread_mutex_lock(&mem_lock);
	pAddr = malloc(size);
	if (pAddr ) {
        malloc_count ++;
		if (size == 8192) {
			DPRINTK("malloc 8192 mem size addr:%x\n",pAddr);			
		}
	
		for (i = 0; i < MAX_DEBUG_MEM_ITEM_LIST; i ++) {
			if (stMemItemList[i].pAddr == 0) {
				stMemItemList[i].pAddr = pAddr;
				stMemItemList[i].iSize = size;
				iTotalMallocSize += size;
				iAddrNum++;
				break;
			}
		}

		
		if (i >= MAX_DEBUG_MEM_ITEM_LIST - 100 || i >= 1000) {
//			DPRINTK("i:%d\n", i);
//			DPRINTK("List size not enough\n");
//			exit(0);
		}	
	}
	pthread_mutex_unlock(&mem_lock);
#else
	pAddr = malloc(size);
	if (NULL == pAddr) {
		DPRINTK("malloc failed with: %s!\n", strerror(errno));
	}
#endif

	return pAddr;
}


void * mem_free(void * ptr)
{
	int i = 0;

#ifdef USE_MEM_DEBUG	
	if (iMallocInit == 0) {
		pthread_mutex_init(&mem_lock,NULL);
		iMallocInit = 1;
		iTotalMallocSize = 0;
	}
	
	pthread_mutex_lock(&mem_lock);
    if (ptr) {
        free(ptr);	
        free_count ++;
    		
    	for (i = 0; i < MAX_DEBUG_MEM_ITEM_LIST; i++) {
    		if (stMemItemList[i].pAddr == ptr) {
    			if (stMemItemList[i].iSize == 8192) {
    				DPRINTK("free 8192 mem size  add:%x\n",stMemItemList[i].pAddr);			
    			}
    			
    			iTotalMallocSize -= stMemItemList[i].iSize;
    			iAddrNum --;
    			stMemItemList[i].pAddr = 0;
    			stMemItemList[i].iSize = 0;	
				ptr = NULL;
    			break;
    		}
    	}		
    }
    else {
        fprintf(stderr, "mem point is null!!!!\n");
        exit(-1);
    }
	pthread_mutex_unlock(&mem_lock);
#else
	free(ptr);	
	ptr = NULL;
#endif

return (void *)NULL;
}


void mem_show(void)
{
	int  i = 0;
#ifdef USE_MEM_DEBUG
	pthread_mutex_lock(&mem_lock);
    DPRINTK("total: %d, %d(malloc)=%d(free), sub: %d\n", 
            iTotalMallocSize, 
            malloc_count, 
            free_count, 
            abs(malloc_count - free_count));
#if 0   
	for (i = 0; i < MAX_DEBUG_MEM_ITEM_LIST; i++){
		if (stMemItemList[i].pAddr != 0 ){
			DPRINTK("mem addr:[%d] addr:0x%x size:%d\n",i,stMemItemList[i].pAddr,stMemItemList[i].iSize);				
		}
	}		
#endif 
	pthread_mutex_unlock(&mem_lock);
	#endif
}

