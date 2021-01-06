#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "queue_buf.h"


int queue_init(queue_info * queue, int max_item_num, int max_data_size)
{
    if (NULL == queue) {
        QUEUE_LOG("argument error!\n");
        return QUEUE_HEAD_IS_NULL_ERR;
    }
    memset(queue, '\0', sizeof(queue_info));
    queue -> max_data_size = max_data_size;
    queue -> max_item_num = max_item_num;
    pthread_mutex_init(&queue -> mutex, NULL);

    QUEUE_DBG("------- QUEUE INFO -------\n");
    QUEUE_DBG(" max_item_num: %d\n", queue -> max_item_num);
    QUEUE_DBG(" max_data_size: %d\n", queue -> max_item_num);         
    return 0;
}

int queue_uninit(queue_info * queue)
{
    unsigned int total_malloc_len = 0;
    list_item * cur_item = NULL, * del_item = NULL;
    if (NULL == queue) {
        QUEUE_LOG("argument error!\n");
        return QUEUE_HEAD_IS_NULL_ERR;
    }

    if (NULL == queue -> next) {
        QUEUE_LOG("queue is null!\n");
        return QUEUE_ITEM_EMPTY_ERR;
    }

    cur_item = queue -> next;
    while(cur_item)
    {
        del_item = cur_item;   
        cur_item = cur_item -> next;
        QUEUE_DBG("find free item id: %d\n", del_item -> item_id);
        total_malloc_len += del_item -> len;
        free(del_item);
    }
    cur_item = NULL;
    del_item = NULL;
    pthread_mutex_destroy(&queue -> mutex);
    QUEUE_DBG("queue left all data size: %d\n", queue -> cur_data_size);
    QUEUE_DBG("queue free all data size: %d\n", total_malloc_len);

    if (queue -> cur_data_size != total_malloc_len) {
        QUEUE_ERR("!!!!!!!!!!!! queu buf memory leak !!!!!!!!!!!!\n");
    }
    return 0;
}

static add_item(queue_info * queue, char * data, int len)
{

    int ret = 0;
    list_item * cur_item = NULL;
    list_item * new_item = NULL;

    pthread_mutex_lock(&queue -> mutex);
    if (NULL == queue) {
        QUEUE_LOG("argument error!\n");
        ret =  QUEUE_HEAD_IS_NULL_ERR;
        goto err;
    }

    if (queue -> cur_item_num + 1 > queue -> max_item_num) {
        ret = QUEUE_ITEM_ENOUGH_ERR;
        QUEUE_LOG("queue item num is not enough!");
        goto err;
    }

    if (queue -> cur_data_size + len > queue -> max_data_size) {
        ret = QUEUE_ITEM_MEM_NOT_ENOUGH_ERR;
        QUEUE_LOG("queue space is not enough!");
        goto err;
    }
    
    new_item = (list_item * )malloc(sizeof(list_item));
    if (NULL == new_item) {    
        QUEUE_LOG("malloc failed with: %s!\n", strerror(errno));
        ret = QUEUE_ITEM_MALLOC_ERR;
        goto err;
    }

    memset(new_item, '\0', sizeof(list_item));
    new_item -> data = (char *)malloc(len);
    if (NULL == new_item -> data) {
        QUEUE_LOG("malloc failed with: %s!\n", strerror(errno));
        ret = QUEUE_ITEM_MALLOC_ERR;
        goto err;
    }

    // 初始化节点
    memset(new_item -> data, '\0', len);
    memcpy(new_item -> data, data, len);
    new_item -> len = len;
    new_item -> item_id = queue -> seq_no;

    // 更新表头  
    queue -> seq_no ++;
    queue -> head_id = new_item -> item_id;
    queue -> cur_item_num ++;   
    queue -> cur_data_size += len;
    
    if (NULL == queue -> next) {
        queue -> next = new_item;
    }
    else {
        cur_item = queue -> next;
        while(NULL != cur_item -> next)
        {
            cur_item = cur_item -> next;
        }   
        cur_item -> next = new_item;    
    }
    pthread_mutex_unlock(&queue -> mutex);  
    QUEUE_DBG("[in] id: %d len: %d\n", new_item -> item_id, new_item -> len);
    
    
    return new_item -> item_id; 
    
err:
    
    QUEUE_DBG("-->\n");
    if (cur_item) {
        if (cur_item -> data) {
            free(cur_item -> data);
        }
    }
    free(cur_item);
    pthread_mutex_unlock(&queue -> mutex);
    return ret;
}

int queue_push(queue_info * queue, char * data, int len)
{
    int ret = 0;
    while(queue_is_full(queue))
    {
        queue_del_tail_item(queue);
    }   

    ret = add_item(queue, data, len);
    if (ret < 0) {
        printf("queue push failed!\n");
    }
    return ret;
}


int queue_del_tail_item(queue_info * queue)
{    
    int ret = 0;
    int data_len = 0;
    list_item * cur_item = NULL;

    if (NULL == queue) {
        QUEUE_LOG("argument error!\n");
        return QUEUE_HEAD_IS_NULL_ERR;
    }   

    if (NULL == queue -> next) {
        QUEUE_LOG("queue is null!\n");
        return QUEUE_ITEM_EMPTY_ERR;
    }

    pthread_mutex_lock(&queue -> mutex);
    cur_item = queue -> next;
    queue -> next = cur_item -> next;
    if (cur_item -> data) {
        data_len = cur_item -> len;
        free(cur_item -> data);
    }
    free(cur_item);
    cur_item = NULL;
    // 更新表头
    queue -> tail_id ++;
    queue -> cur_item_num --;
    queue -> cur_data_size -= data_len;    
    pthread_mutex_unlock(&queue -> mutex);
    return 0;
    
err:
    return ret;
}

int queue_pop(queue_info * queue, char * dst, int buf_size, int item_id)
{
    int ret = 0;

    int find_flag = 0;
    list_item * cur_item = NULL;
    int data_len = 0;

    
    pthread_mutex_lock(&queue -> mutex);
    if (NULL == queue -> next) {
        QUEUE_LOG("queue is empty!\n");
        ret = QUEUE_ITEM_EMPTY_ERR;
        goto err;
    }
    else {
        cur_item = queue -> next;
        if (item_id == cur_item -> item_id) {
            find_flag = 1;
        }
        else {
            while(cur_item)
            {
                if (item_id == cur_item -> item_id) {
                    find_flag = 1;
                    break;
                }                
                cur_item = cur_item -> next;
            }   
        }

        if (find_flag) {
            data_len = cur_item -> len; 
            if (data_len > buf_size) {
                ret = QUEUE_BUF_NOT_ENOUGH_ERR;
                goto err;
            }
            memcpy(dst, cur_item -> data, cur_item -> len);
            QUEUE_DBG("[out] id: %d len: %d\n", cur_item -> item_id, cur_item -> len);
        }
        else {
            ret = QUEUE_ITEM_NOT_FOUND_ERR;
        } 
        ret = data_len;
    }
    pthread_mutex_unlock(&queue -> mutex);

    return ret;

err:
    
    QUEUE_DBG("-->\n");
    pthread_mutex_unlock(&queue -> mutex);
    return ret;
}

int queue_item_num(queue_info * queue)
{
    if (NULL == queue -> next) {
        QUEUE_LOG("queue is empty!\n"); 
        return QUEUE_ITEM_EMPTY_ERR;
    }
    
    return queue -> cur_item_num;
}


int queue_tail_item_num(queue_info * queue, unsigned int * item_num)
{
    if (NULL == queue -> next) {
        QUEUE_LOG("queue is empty!\n");
        return QUEUE_ITEM_EMPTY_ERR;
    }    
    pthread_mutex_lock(&queue -> mutex);
    * item_num = queue -> next -> item_id;
    pthread_mutex_unlock(&queue -> mutex);
    return 0;
}


int queue_is_full(queue_info * queue)
{
    if (NULL == queue) {
        QUEUE_LOG("argument error!\n");
        return QUEUE_HEAD_IS_NULL_ERR;
    }

    if (queue -> cur_item_num >= queue -> max_item_num) {
 //       QUEUE_DBG("current item num: %d\n", queue -> cur_item_num);
        return 1;
    }
    
    return 0;
}

