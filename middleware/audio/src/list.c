#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "list.h"


#ifndef LINUX_HOST
CCriticalSection sync;
static CRITICAL_SECTION g_cs;
#endif 

int sll_init_list(SINGLY_LINKED_LIST_INFO_ST  * list_info, int max_item_num, int data_max_size)
{
	memset(list_info, 0x00, sizeof(SINGLY_LINKED_LIST_INFO_ST));
	list_info -> max_item_num = max_item_num;
	list_info -> max_item_size = data_max_size;
#if defined (LINUX_HOST)
	pthread_mutex_init(&list_info->lock, NULL);
#elif defined (WIN32)
//	list_info->mutex(0,0,0);
	InitializeCriticalSection(&g_cs); 
#endif 
	return 1;
}


int sll_destroy_list(SINGLY_LINKED_LIST_INFO_ST  * list_info)
{
#if defined (LINUX_HOST)
	pthread_mutex_lock(&list_info->lock);
#elif defined (WIN32)
	//CSingleLock singlelock(&list_info->mutex);
	//singlelock.Lock();
	//sync.Lock();
	EnterCriticalSection(&g_cs);
#endif
    
    fprintf(stderr, "queue total data size: %d\n",  list_info -> all_data_size);

    unsigned int queue_data_size = list_info -> all_data_size;
    unsigned int total_data_size = 0;

    
	if (list_info -> list != NULL) {
		SINGLY_LINKED_LIST_ST * del = NULL;
		SINGLY_LINKED_LIST_ST * cur = list_info->list;
		while(cur -> next != NULL)
		{
			del = cur;
			cur = cur -> next;

#ifdef debug_log
		    DPRINTK("del item id=%d len=%d\n",del->id,del->len);
#endif
			
			if (del -> data) {
                fprintf(stderr, "find item index: %d, len: %d\n", del -> id, del -> len);
                total_data_size += del -> len;
				free(del -> data);
			}
			free(del);								
		}

		if (cur) {
			del = cur;
#ifdef debug_log
	        DPRINTK("del item id=%d len=%d\n",del->id,del->len);
#endif
			if (del -> data) {
				free(del -> data);
                total_data_size += del -> len;
			}
			free(del);		
		}
		
	}

    fprintf(stderr, "free all data size: %d\n", total_data_size);
    if (queue_data_size != total_data_size) {
        fprintf(stderr, "############### ERROR!!!, MEMORY LEAK ###############");
    }

    
#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 	
	memset(list_info, 0x00, sizeof(SINGLY_LINKED_LIST_INFO_ST));
	return 1;
}


inline int sll_add_list_item(SINGLY_LINKED_LIST_INFO_ST  * list_info, char * data, int len)
{
    SINGLY_LINKED_LIST_ST * list_st = NULL;	
    int ret = 0;
    int add_id = 0;
    struct timeval sendStartTime;
    struct timeval sendEndTime;

#if defined (LINUX_HOST)
	pthread_mutex_lock(&list_info->lock);
#elif defined (WIN32)
	EnterCriticalSection(&g_cs);
#endif
 
    if (list_info -> item_num + 1 > list_info -> max_item_num) {
        ret = SLL_ITEM_ENOUGH_ERR;
        DPRINTK("create err\n");
        goto err;
    }	

    /* 初始化节点 */
    list_st = (SINGLY_LINKED_LIST_ST *)malloc(sizeof(SINGLY_LINKED_LIST_ST));
    if (list_st == NULL) {
        DPRINTK("malloc err\n");
        goto err;
    }

    memset(list_st, 0x00, sizeof(SINGLY_LINKED_LIST_ST));
    list_st -> data = (char *)malloc(len);
    if (list_st -> data == NULL) {
        DPRINTK("malloc err\n");
        goto err;
    }
#ifdef USE_MEMCPY
	memcpy(list_st->data,data,len);
#else
	memmove(list_st->data,data,len);
#endif
	list_st -> len = len;		
	list_st -> id = list_info -> seq_no; //为帧序

	/* 初始化表头 */
	list_info -> seq_no ++;	
	list_info -> head_id = list_st -> id;
	list_info -> item_num ++;

	// 当前队列为空
	if (list_info -> list == NULL) {
		list_info -> list = list_st;
	}
	else {								//当前队列不为空
	    /* 从链表的底部增加节点 */
		SINGLY_LINKED_LIST_ST * cur = list_info -> list;
		while(cur -> next != NULL)
		{
			cur = cur -> next;
		}
		cur -> next = list_st;
	}	

	list_info -> all_data_size += len;
#ifdef debug_log
	DPRINTK("add item id=%d len=%d\n",list_st->id,list_st->len);
#endif

	add_id = list_st -> id;

#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 		

	return add_id;


err:
	if (list_st) {
		if (list_st->data) {
			free(list_st->data);
		}
		free(list_st);		
	}

#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 	

	return ret;
}


inline int sll_del_list_tail_item(SINGLY_LINKED_LIST_INFO_ST  * list_info)
{
    SINGLY_LINKED_LIST_ST * list_st = NULL;	
    int ret = 1;
    int data_len = 0;

#if defined (LINUX_HOST)
	pthread_mutex_lock(&list_info->lock);
#elif defined (WIN32)
	//CSingleLock singlelock(&list_info->mutex);
	//singlelock.Lock();
	//sync.Lock();
	EnterCriticalSection(&g_cs);
#endif	
    if (list_info -> item_num - 1 < 0) {
        ret = 1;		
        goto err;
    }	
	
	// 当前队列为空不采取任何操作
    if (list_info -> list == NULL) {	

    }
    else {
        SINGLY_LINKED_LIST_ST * cur = list_info -> list;
        if (cur) {		
#ifdef debug_log
            DPRINTK("del item id=%d len=%d\n",cur->id,cur->len);
#endif
            list_info -> list = cur -> next;
            data_len = cur->len;
			if (cur -> data) {
				free(cur -> data);
			}
            free(cur);		
        }
    }		

    list_info -> tail_id++;
    list_info -> item_num--;
    list_info -> all_data_size -= data_len;
#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 	

    return 1;
    
err:
    if (list_st) {
		if (list_st -> data) {
			free(list_st -> data);
		}
        free(list_st);		
    }
#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 		
    return ret;
}


#define MAX_VIDEO_FRAME_SIZE	(512 * 1024)

#if 0
inline int sll_get_list_item(SINGLY_LINKED_LIST_INFO_ST  * list_info, 
			char * dest_buf, int * len, int item_id)
{
//    printf("find node id: %d\n", item_id);
	SINGLY_LINKED_LIST_ST * list_st = NULL;		
	int ret = 0;	

#if defined (LINUX_HOST)
	pthread_mutex_lock(&list_info->lock);
#elif defined (WIN32)
	//CSingleLock singlelock(&list_info->mutex);
	//singlelock.Lock();
	//sync.Unlock();
	EnterCriticalSection(&g_cs);
#endif	

    /* 先做判断，看当前的链表头是否有效 */
	if( list_info->item_num - 1 < 0 ){
		ret = SLL_ITEM_EMPTY_ERR;		
		goto err;
	}	
	if(list_info->list == NULL ){		
		ret = SLL_ITEM_EMPTY_ERR;		
		goto err;
	}
	else{
	    /* 查找到匹配的节点 */
		SINGLY_LINKED_LIST_ST * cur = list_info -> list;
		int find_available_item = 0;	
		
		if(cur -> id == item_id ){
			find_available_item = 1;
		}
		else{
			while(cur -> next != NULL ){
				cur = cur -> next;
				if(cur -> id == item_id){
					find_available_item = 1;
					break;
				}
			}
		}	


		if(find_available_item == 0 ){
			ret = SLL_ITEM_EMPTY_ERR;		
			goto err;
		}
		if(MAX_VIDEO_FRAME_SIZE < cur -> len){
			ret = SLL_ITEM_BUF_NOT_ENOUGH_ERR;		
			goto err;
		}

		memcpy(dest_buf, cur->data, cur->len);
//		* len = cur -> len;
		ret = cur->len;		    
		#ifdef debug_log
		DPRINTK("get item id=%d len=%d\n",cur->id,cur->len);
		#endif
	}		


#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 	

	return ret;
err:
#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 		
	return ret;
}
#endif 



int sll_get_list_item(SINGLY_LINKED_LIST_INFO_ST  * list_info, char * dest_buf, int dest_max_len, int item_id)
{
	SINGLY_LINKED_LIST_ST * list_st = NULL;		
	int ret = 0;	

#if defined (LINUX_HOST)
	pthread_mutex_lock(&list_info->lock);
#elif defined (WIN32)
	EnterCriticalSection(&g_cs);
#endif

	if (list_info -> item_num - 1 < 0) {
		ret = SLL_ITEM_EMPTY_ERR;		
		goto err;
	}	

	if (list_info->list == NULL) {		
		ret = SLL_ITEM_EMPTY_ERR;		
		goto err;
	}
	else {
		SINGLY_LINKED_LIST_ST * cur = list_info->list;
		int find_available_item = 0;	
		
		if (cur->id == item_id) {
			find_available_item = 1;
		}
		else {
			while( cur->next != NULL )
			{
				cur = cur->next;
				if (cur->id == item_id) {
					find_available_item = 1;
					break;
				}
			}
		}	

		if (find_available_item == 0) {
			ret = SLL_ITEM_EMPTY_ERR;		
			goto err;
		}

		if (dest_max_len < cur->len) {
			ret = SLL_ITEM_BUF_NOT_ENOUGH_ERR;		
			goto err;
		}

#ifdef USE_MEMCPY
	memcpy(dest_buf,cur->data,cur->len);
#else
	memmove(dest_buf,cur->data,cur->len);
#endif

		ret = cur->len;		

#ifdef debug_log
		DPRINTK("get item id=%d len=%d\n",cur->id,cur->len);
		#endif
	}		


#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
#elif defined (WIN32)
	LeaveCriticalSection(&g_cs);
#endif 

	return ret;
err:

#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	LeaveCriticalSection(&g_cs);
#endif 	
	return ret;
}



inline int sll_get_tail_list_item_no(SINGLY_LINKED_LIST_INFO_ST  * list_info,unsigned int * iListTailNo)
{
	SINGLY_LINKED_LIST_ST * list_st = NULL;		
	int ret = 0;	

#if defined (LINUX_HOST)
	pthread_mutex_lock(&list_info->lock);
#elif defined (WIN32)
	//CSingleLock singlelock(&list_info->mutex);
	//singlelock.Lock();
	//sync.Lock();
	EnterCriticalSection(&g_cs);
#endif	

	if (list_info->item_num - 1 < 0) {
		ret = SLL_ITEM_EMPTY_ERR;		
		goto err;
	}	

	if (list_info->list == NULL) {		
		ret = SLL_ITEM_EMPTY_ERR;		
		goto err;
	}
	else {
		SINGLY_LINKED_LIST_ST * cur = list_info->list;	
		*iListTailNo = cur->id;
	}	

#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 		

	return ret;
err:

#if defined (LINUX_HOST)
	pthread_mutex_unlock(&list_info->lock);	
	pthread_mutex_destroy(&list_info->lock);	
#elif defined (WIN32)
	//singlelock.Unlock();
	//sync.Unlock();
	LeaveCriticalSection(&g_cs);
#endif 			
	return ret;
}


inline int sll_list_item_num(SINGLY_LINKED_LIST_INFO_ST  * list_info)
{
	return list_info->item_num;
}

int sll_list_enable_add_item(SINGLY_LINKED_LIST_INFO_ST  * list_info)
{	

	if (list_info->all_data_size >= list_info->max_item_size) {
//		log_err("%d >= %d\n",list_info->all_data_size, list_info->max_item_size);
		return 0;
	}

	if (list_info->item_num < list_info->max_item_num) {
		return 1;
	}
	else {
//		log_err("%d >= %d\n",list_info->item_num, list_info->max_item_num);
		return 0;
	}
}


int queue_push2(SINGLY_LINKED_LIST_INFO_ST * plist, char * frame_data_buf, int frame_len)
{	
	int ret = 0;

#if 1
	while( sll_list_enable_add_item(plist) == 0)
	{				
		sll_del_list_tail_item(plist);				
	}		
#endif 

#if 1
	ret = sll_add_list_item(plist, frame_data_buf, frame_len);
	if(ret < 0){
        return -1;
	}
#endif 
	return ret;
}


int queue_pop2(SINGLY_LINKED_LIST_INFO_ST * plist, char * buf, int * len, int id)
{
	int dataLen = 0;
	while(sll_list_item_num(plist) <= 0) 
	{
#ifndef LINUX_HOST
		Sleep(10000);
#else
        usleep(50 * 1000);  
#endif 
    }
	
	dataLen = sll_get_list_item(plist, buf, 512 * 1024, id);
	if (dataLen > 0) {
		return dataLen;
	}
	else {
		if (id > plist->head_id) {
		
		}
		else {

		}
	}
	return dataLen;
}
