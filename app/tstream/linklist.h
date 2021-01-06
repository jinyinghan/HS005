#ifndef  __LINK_LIST_H__
#define __LINK_LIST_H__
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define av_malloc malloc
#define av_free   free

typedef struct {
	int channel;
	unsigned int index;
	uint8_t keyframe;
	struct timeval tv;
	int size;
	uint8_t *data;
	
}YsxAVPacket;


typedef struct LNode {
	YsxAVPacket *pkt;
	int read_mark;
	struct LNode *next;
}LNode, *pLinkList;

//pLinkList m_pList;
//int m_listLength;
int InitList(pLinkList m_pList);
int DestroyList(pLinkList m_pList) ;
int IsEmpty(pLinkList m_pList);
int GetLength(pLinkList m_pList);
int ClearList(pLinkList m_pList);
int SetNodePkt(pLinkList m_pList,int position, YsxAVPacket *newpkt) ;
YsxAVPacket *GetNodePkt(pLinkList m_pList,int position) ;
int InsertNode(pLinkList m_pList,int beforeWhich, YsxAVPacket *pkt) ;
int DeleteNode(pLinkList m_pList,int position);
int GetNode(pLinkList m_pList,int position, LNode **node);


#endif

