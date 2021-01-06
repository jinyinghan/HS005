#include "circular_buf.h"

cbuf_handle_t circular_buf_init(char* buffer, size_t size) //初始化
{

    cbuf_handle_t cbuf = (cbuf_handle_t)mem_malloc(sizeof(circular_buf));
    if(buffer == NULL)
	{
		LOG("buffer is NULL\n");
		return NULL;	
	}

    cbuf->buffer = buffer;
    cbuf->max = size;
    circular_buf_reset(cbuf);

    if(!circular_buf_empty(cbuf))
	{
		LOG("buffer is not empty \n");
		return NULL;
	}

    return cbuf;
}

void circular_buf_reset(cbuf_handle_t cbuf)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
	return;
}

void circular_buf_free(cbuf_handle_t cbuf)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}
    free(cbuf);
}

bool circular_buf_full(cbuf_handle_t cbuf)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}
	if(cbuf->head >cbuf->max || cbuf->tail > cbuf->max)
	{
		cbuf->full = true;
	}
	
	if((circular_buf_size(cbuf)) >= (cbuf->max))
	{
		cbuf->full = true;
	}
	else
	{
		cbuf->full = false;
	}

    return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}

    return ((cbuf->full != true) && (cbuf->head == cbuf->tail));
}

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}

    return cbuf->max;
}

size_t circular_buf_size(cbuf_handle_t cbuf)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}

    size_t size = cbuf->max;

    if(!cbuf->full)
    {
        if(cbuf->head >= cbuf->tail)
        {
            size = (cbuf->head - cbuf->tail);
        }
        else
        {
            size = (cbuf->max + cbuf->head - cbuf->tail);
        }
    }

    return size;
}

static void advance_pointer(cbuf_handle_t cbuf, int size)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}

    if(cbuf->full == true)
    {
        cbuf->tail = (cbuf->tail + size) % cbuf->max;
    }

    cbuf->head = (cbuf->head + size) % cbuf->max;
	
	if(cbuf->head == cbuf->tail)
	{
		cbuf->full = true;
	}
	else
	{
		cbuf->full = false;
	}
}

void retreat_pointer(cbuf_handle_t cbuf, int size)
{
	if(cbuf == NULL)
	{
		LOG("return error\n");
		return ;
	}

    cbuf->full = false;
//	memset((cbuf->buffer + cbuf->tail), 0, size);
    cbuf->tail = (cbuf->tail + size) % cbuf->max;
}
/*
void circular_buf_put(cbuf_handle_t cbuf, char* data)//, int size)
{
    if(cbuf  == NULL || cbuf->buffer == NULL)
	{
		LOG("return error\n");
		return ;
	}

//	memcpy(cbuf->buffer[cbuf->head],data,size);		
    cbuf->buffer[cbuf->head] = data;
    advance_pointer(cbuf);
}
*/

int circular_buf_put(cbuf_handle_t cbuf, char* data, int size)
{
	 if( cbuf == NULL || cbuf->buffer == NULL)
	{
		LOG("cbuf->buffer == NULL return error\n");
		return -1;
	}

	if(size > cbuf->max)
	{
		LOG("write %d,should be %d !\n",size,cbuf->max);
		return -2;
	}

	if(circular_buf_full(cbuf))
    {
    	LOG("circular_buf_full error!\n");
		return -3;
	}
	
	//将要存入的数据copy到buffer起始处
	if(cbuf->head + size > cbuf->max)//需要分成两段
	{	
		int len1 = cbuf->max - cbuf->head;
		int len2 = size - len1;
		memcpy((cbuf->buffer + cbuf->head), data, len1);
		memcpy( cbuf->buffer, (data + len1), len2);
		cbuf->head = len2;			//新的写入位置
	}
	else
	{
		memcpy((cbuf->buffer + cbuf->head), data, size);		
        advance_pointer(cbuf, size);
	}

	return 0;
}

/*

int circular_buf_get(cbuf_handle_t cbuf, char* data, int size)
{
    if(cbuf  == NULL || cbuf->buffer == NULL || data == NULL)
	{
		LOG("return error\n");
		return -1;
	}

    if(!circular_buf_empty(cbuf))
    {
    	memcpy(data, (cbuf->buffer + cbuf->tail), size);
//        *data = cbuf->buffer[cbuf->tail];
        retreat_pointer(cbuf, size);
		return 0;
    }
	return -1;
}


*/
