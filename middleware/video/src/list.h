#ifndef _LIST_H_
#define _LIST_H_




#define LINUX_HOST


#define SLL_ERR_BASE  (-100)
#define SLL_ITEM_ENOUGH_ERR (SLL_ERR_BASE-1)
#define SLL_ITEM_EMPTY_ERR (SLL_ERR_BASE-2)
#define SLL_ITEM_BUF_NOT_ENOUGH_ERR (SLL_ERR_BASE-3)

#define DPRINTK printf

typedef struct _SINGLY_LINKED_LIST_ST_{
	unsigned int id;
	char * data;
	int len;
	struct _SINGLY_LINKED_LIST_ST_ * next;
}SINGLY_LINKED_LIST_ST;

typedef struct _SINGLY_LINKED_LIST_INFO_ST_{
	unsigned int max_item_num;
	unsigned int item_num;
	unsigned int head_id;
	unsigned int tail_id;
	unsigned int seq_no;
	unsigned int max_item_size;
	unsigned int all_data_size;
#if defined (LINUX_HOST)
	pthread_mutex_t lock;
#elif defined (WIN32)
	CMutex mutex;
	CCriticalSection criticalSection;
#endif 
	SINGLY_LINKED_LIST_ST * list;
}SINGLY_LINKED_LIST_INFO_ST;

int sll_init_list(SINGLY_LINKED_LIST_INFO_ST  * list_info,int max_item_num,int data_max_size);
int sll_destroy_list(SINGLY_LINKED_LIST_INFO_ST  * list_info);
int sll_add_list_item(SINGLY_LINKED_LIST_INFO_ST  * list_info,char * data,int len);
int sll_del_list_tail_item(SINGLY_LINKED_LIST_INFO_ST  * list_info);
int sll_get_list_item(SINGLY_LINKED_LIST_INFO_ST  * list_info,char * dest_buf,int dest_max_len,int item_id);
//inline int sll_get_list_item(SINGLY_LINKED_LIST_INFO_ST  * list_info, 
//			char * dest_buf, int * len, int item_id);
int sll_list_item_num(SINGLY_LINKED_LIST_INFO_ST  * list_info);
int sll_list_enable_add_item(SINGLY_LINKED_LIST_INFO_ST  * list_info);

int queue_push2(SINGLY_LINKED_LIST_INFO_ST * plist, char * frame_data_buf, int frame_len);
int queue_pop2(SINGLY_LINKED_LIST_INFO_ST * plist, char * buf, int * len, int id);

int set_h264_array_point(void * ptrH264);
void clear_h264_array_point(int id);
#endif
