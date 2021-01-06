#include "ysx_data_buf.h"

/*
OPERATE_RET tuya_ipc_ring_buffer_init(CHANNEL_E channel, UINT_T bitrate, UINT_T fps,
					UINT_T max_buffer_seconds, FUNC_REQUEST_I_FRAME requestIframeCB);
*/

av_buffer *init_av_buffer(int buf_size,int min_node_num, int max_node_num)
{
	av_buffer *buffer = (av_buffer *)mem_malloc(sizeof(av_buffer));
	if(buffer == NULL){
		printf("cannot get memory for buffer in init_av_buffer\n");
		return NULL;	
	}
	memset(buffer, 0, sizeof(av_buffer));
	
	buffer->max_node_num = max_node_num;
	buffer->min_node_num = min_node_num;
	buffer->buffer_start = (char *)mem_malloc(buf_size);
	if(buffer->buffer_start == NULL){
		printf("cannot get memory for buffer_start in init_av_buffer\n");
		mem_free(buffer);
		return NULL;	
	}
	
	memset(buffer->buffer_start, 0, buf_size);
	buffer->buffer_sum_size = buf_size;
	buffer->read_pos = -1;
	buffer->running = 1;
	buffer->wait_Iframe = 1; 

	int i;
	for(i = 0; i < buffer->max_node_num; i++)
		buffer->node[i].read_mark = 1;

	return buffer;
}

void deinit_av_buffer(av_buffer *buf)
{	
	if(buf){
		if(buf->buffer_start)
			mem_free(buf->buffer_start);
		mem_free(buf);
	}
	return;
}

void start_buffer_data(av_buffer *buffer)
{
	av_buffer *buf = buffer;
	
	buf->wait_Iframe = 1; 
	buf->running = 1;

	return;
}

/*
	以下几个地方需要用到此接口,一下三个接口需要配套使用
	stop_buffer_data(video_live_buffer);
	clean_buffer_data(video_live_buffer);
	start_buffer_data(video_live_buffer);
	1 app下发出图命令
	2 app下发切换清晰度命令
*/
void stop_buffer_data(av_buffer *buffer)
{
	av_buffer *buf = buffer;

	buf->running = 0;

	return;
}

void clean_buffer_data(av_buffer *buffer)
{
	av_buffer *buf = buffer;

	int i;
//	buf->polling_once == 0;
	buf->polling_once = 0;
	buf->read_pos = -1;
	for(i = 0; i < buffer->max_node_num; i++ )
		buf->node[i].read_mark = 1;

	return;
}

// 查询一个GOP最后一个P帧是否被读取
static int check_buffer_data(av_buffer *buffer)
{
	av_buffer *buf = buffer;
	int ret = 0;

	if (buf->node[buf->Pframe_pos].read_mark)
		ret = 1;

	return ret;
}
/*
需求:
	1 可根据设定的时间来缓存固定的时间大小
	2 

*/

/*
OPERATE_RET tuya_ipc_ring_buffer_append_data(CHANNEL_E channel, UCHAR_T *addr, UINT_T size, 
		MEDIA_FRAME_TYPE_E type, UINT64_T pts);

OPERATE_RET tuya_ipc_ring_buffer_append_extra_data(CHANNEL_E channel, UCHAR_T *dataAddr, UINT_T dataSize);
*/

int mpool_append_data(av_buffer *buffer, char *data, int size, int keyframe, struct timeval tv)
{
	av_buffer *buf = buffer;

	if (buf -> running) {
		if (buf) {
			if (buf -> polling_once == 0) {
				// 队列未被插入过节点
				buf->write_pos = 0;
				buf->polling_once = 1;
			}

			if (buf -> write_pos == buf -> max_node_num)	{
				// 已经达到最大节点数
				buf->write_pos = 0;
			}

			if (buf->wait_Iframe) {
				if (keyframe) {
					if (check_buffer_data(buf)) {
						// GOP序列最后一个P帧已经被读取,可以从头开始插入I帧
						buf->write_pos = 0;
						buf->wait_Iframe = 0;
					}
				}
				else{
					// GOP序列最后一个P帧未被读取,丢弃当前帧
					return AV_BUFFER_FULL;
				}
			}
					
 			if (buf -> write_pos) {
 				if (buf -> buffer_sum_size - buf->buffer_offset >= size) {	  // 对列空间够大
 					memcpy(buf->buffer_start + buf->buffer_offset, data, size);
 					buf->node[buf->write_pos].data = buf->buffer_start + buf->buffer_offset;
 					buf->buffer_offset += size;
 				}
 				else {					
 					buf -> write_pos ++; // 队列空间不够大
 					if (buf -> write_pos == buf -> max_node_num) {
 						buf -> write_pos = 0;
 					}
 
 					buf->wait_Iframe = 1;
 					return AV_BUFFER_FULL;
 				}	
 			}
			else{
				if (buf->buffer_sum_size >= size) {
					memcpy(buf->buffer_start, data, size);
					buf->node[buf->write_pos].data = buf->buffer_start;
					buf->buffer_offset = size;
				}
				else {
					buf->wait_Iframe = 1;
					return AV_BUFFER_FULL;
				}
			}
			
			buf->node[buf->write_pos].size = size;
			buf->node[buf->write_pos].keyframe = keyframe;
			memcpy(&buf->node[buf->write_pos].tv, &tv, sizeof(struct timeval));
			buf->node[buf->write_pos].read_mark = 0;

			if (!keyframe)
				buf->Pframe_pos = buf->write_pos;
			
			buf->write_pos ++; 
			if(buf->write_pos == buf->max_node_num)	{
				buf->write_pos = 0;
			}
			// 成功插入节点
			return AV_BUFFER_NULL;	

		}
		else {
			// 节点为空
			return AV_BUFFER_ERR; 
		}
	}
	else {		
		// 暂停缓存队列
		return AV_BUFFER_ERR;	
	}
}




int write_buffer_data(av_buffer *buffer, char *data, int size, int keyframe, struct timeval tv, int index)
{
	av_buffer *buf = buffer;

	if (buf -> running) {
		if (buf) {
			if (buf -> polling_once == 0) {
				// 队列未被插入过节点
				buf->write_pos = 0;
				buf->polling_once = 1;
			}

			if (buf -> write_pos == buf -> max_node_num)	{
				// 已经达到最大节点数
				buf->write_pos = 0;
			}

			if (buf->wait_Iframe) {
				if (keyframe) {
					if (check_buffer_data(buf)) {
						// GOP序列最后一个P帧已经被读取,可以从头开始插入I帧
						buf->write_pos = 0;
						buf->wait_Iframe = 0;
					}
				}
				else{
					// GOP序列最后一个P帧未被读取,丢弃当前帧
					return AV_BUFFER_FULL;
				}
			}
					
 			if (buf -> write_pos) {
 				if (buf -> buffer_sum_size - buf->buffer_offset >= size) {	  // 对列空间够大
 					memcpy(buf->buffer_start + buf->buffer_offset, data, size);
 					buf->node[buf->write_pos].data = buf->buffer_start + buf->buffer_offset;
 					buf->buffer_offset += size;
 				}
 				else {					
 					buf -> write_pos ++; // 队列空间不够大
 					if (buf -> write_pos == buf -> max_node_num) {
 						buf -> write_pos = 0;
 					}
 
 					buf->wait_Iframe = 1;
 					return AV_BUFFER_FULL;
 				}	
 			}
			else{
				if (buf->buffer_sum_size >= size) {
					memcpy(buf->buffer_start, data, size);
					buf->node[buf->write_pos].data = buf->buffer_start;
					buf->buffer_offset = size;
				}else{
					buf->wait_Iframe = 1;
					return AV_BUFFER_FULL;
				}
			}
			
			buf->node[buf->write_pos].size = size;
			buf->node[buf->write_pos].keyframe = keyframe;
			memcpy(&buf->node[buf->write_pos].tv, &tv, sizeof(struct timeval));
			buf->node[buf->write_pos].read_mark = 0;
			buf -> node[buf -> write_pos].index = index;

			if (!keyframe)
				buf->Pframe_pos = buf->write_pos;
			
			buf->write_pos ++; 
			if(buf->write_pos == buf->max_node_num)	{
				buf->write_pos = 0;
			}
			// 成功插入节点
			return AV_BUFFER_NULL;	

		}
		else {
			// 节点为空
			return AV_BUFFER_ERR; 
		}
	}
	else {		
		// 暂停缓存队列
		return AV_BUFFER_ERR;	
	}
}

int read_buffer_data(av_buffer *buffer, char **data, int *size, int *keyframe, struct timeval *tv, int * index)
{
	av_buffer *buf = buffer;

	if(buffer){
		if(buf->polling_once == 0)
			buf ->read_pos = 0;
		else{
			if(buf->read_pos == -1){
				if((buf->write_pos - buf->min_node_num) >= 0){
					buf ->read_pos = buf->write_pos - buf->min_node_num;
					
				}else{
					buf->read_pos = buf->max_node_num - (buf->min_node_num - buf->write_pos);
				}
			}
		}
		while(1){

			if(buf->node[buf->read_pos].read_mark == 0){
				buf->node[buf->read_pos].read_mark = 1;
				
				*data = buf->node[buf->read_pos].data;
				*size = buf->node[buf->read_pos].size;
				*keyframe = buf->node[buf->read_pos].keyframe;
				*index = buf -> node[buf -> read_pos].index;
				memcpy(tv, &buf->node[buf->read_pos].tv, sizeof(struct timeval));

				buf->read_pos++;
				if(buf->read_pos == buf->max_node_num){
					buf->read_pos = 0;
				}
				return AV_BUFFER_NULL;
				
			}else{
				if(buf->read_pos == buf->write_pos){
					*size = 0;
					*data = NULL;
					*keyframe = 0;
					memset(tv, 0, sizeof(struct timeval));
					return AV_BUFFER_FULL;
				}
				buf->read_pos++;
				if(buf->read_pos == buf->max_node_num){
					buf->read_pos = 0;
				}
			}
			
		}
		return AV_BUFFER_NULL;
	}else
		return AV_BUFFER_ERR;
	
}

#if 0
void read_write_pos_sync(av_buffer *buffer)
{
	int i;
	buffer->read_pos = -1;
	for(i = 0; i < buffer->max_node_num; i++ )
		buffer->node[i].read_mark = 0;
	return;
}
#endif

