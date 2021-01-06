#ifndef _QUEUE_BUF_H_
#define _QUEUE_BUG_H_


/*
 7   |----|  - tail
 6   |----|
 5   |----|
 4   |----|
 3   |----|
 2   |----|
 1   |----|  - head 
*/
    
#define QUEUE_ERR_BASE  (-100)
#define QUEUE_ITEM_ENOUGH_ERR (QUEUE_ERR_BASE - 1)
#define QUEUE_ITEM_EMPTY_ERR (QUEUE_ERR_BASE - 2)
#define QUEUE_ITEM_MEM_NOT_ENOUGH_ERR (QUEUE_ERR_BASE - 3)
#define QUEUE_HEAD_IS_NULL_ERR (QUEUE_ERR_BASE - 4)
#define QUEUE_ITEM_MALLOC_ERR (QUEUE_ERR_BASE - 5)
#define QUEUE_ITEM_NOT_FOUND_ERR (QUEUE_ERR_BASE - 6)
#define QUEUE_BUF_NOT_ENOUGH_ERR (QUEUE_ERR_BASE - 7)


#define QUEUE_LOG printf


#define RED            "\033[0;32;31m"
#define LIGHT_RED      "\033[1;31m"
#define NONE           "\033[m"
#define WHITE          "\033[1;37m"


//#define EMERGENCY_DEBUG
#ifdef  EMERGENCY_DEBUG
#define QUEUE_DBG(fmt...)   \
	        do {\
				printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0)  
#else 
#define QUEUE_DBG(fmt...)
#endif 



#define QUEUE_ERR(fmt...)   \
	        do {\
				printf(LIGHT_RED"[%s] [%s] [%d]: "NONE,  \
                        __FILE__, \
				        __FUNCTION__,  \
				        __LINE__);\
				printf(fmt);\
			}while(0)  





typedef struct {
    int item_id;
    char * data;
    int len;
    struct list_item * next;
}list_item;

    
typedef struct {
    int head_id;
    int tail_id;
    int max_item_num;
    int cur_item_num;   
    int max_data_size;
    unsigned int cur_data_size;
    unsigned int seq_no;
    list_item * next;
    pthread_mutex_t mutex;
}queue_info;


int queue_init(queue_info * queue, int max_item_num, int max_item_size);
int queue_uninit(queue_info * queue);

int queue_push(queue_info * queue, char * data, int len);
int queue_del_tail_item(queue_info * queue);
int queue_pop(queue_info * queue, char * dst, int buf_size, int item_id);
int queue_item_num(queue_info * queue);
int queue_tail_item_num(queue_info * queue, unsigned int * item_num);
int queue_is_full(queue_info * queue);


#endif 
