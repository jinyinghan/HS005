/*
 * linklist.c
 *
 *  Created on: 2013-8-12
 *  Author: wanglei
 */
#include "linklist.h"

//初始化，分配一个头节点。
int InitList(pLinkList m_pList) {
	if (!(m_pList = (pLinkList)av_malloc(sizeof(LNode)))) {
	  return FALSE;
	}
	m_pList->next = NULL;
	
	return TRUE;
}

//销毁链表。
int DestroyList(pLinkList m_pList) {
	if (!ClearList(m_pList)) {
	  return FALSE;
	}
	
	av_free(m_pList);
	
	return TRUE;
}

//判断链表是否为空。若为空，返回true，否则返回false。
int IsEmpty(pLinkList m_pList) {
	if (m_pList->next == NULL) {
	  return TRUE;
	}
	return FALSE;
}

//返回链表的中当前节点数。
int GetLength(pLinkList m_pList) {
	int m_listLength = 0;
	LNode *pTemp = m_pList;

	if(pTemp == NULL ){
		printf("List is NULL \n");
		return -1;
	}
	while (pTemp->next != NULL) {
	  pTemp = pTemp->next;
	  m_listLength++;
	}
	return m_listLength;
}

//将链表清空，释放当前所有节点。
int ClearList(pLinkList m_pList) {
	if (m_pList == NULL) {
	  return FALSE;
	}

	LNode *pTemp = NULL;
	while (m_pList->next != NULL) {
	  pTemp = m_pList->next;
	  m_pList->next = pTemp->next;
	  free(pTemp->pkt);
	  av_free(pTemp);
	}
	//m_listLength = 0;
	
	return TRUE;
}

//将position指定的节点内的数据设置为newPkt
//第一个有效节点的position为1。
int SetNodePkt(pLinkList m_pList,int position, YsxAVPacket *newpkt) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	  return FALSE;
	}
	
	pTemp->pkt = newpkt;
	
	return TRUE;
}

//得到指定位置节点的数据。
//节点索引从1到listLength。
YsxAVPacket *GetNodePkt(pLinkList m_pList,int position) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	   printf("cannot find node position\n");
	    return NULL;
	}
	
	return pTemp->pkt;
	
}

int GetNodeMark(pLinkList m_pList,int position) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	  	 printf("cannot find node position\n");
	    return -1;
	}
	
	return pTemp->read_mark;
	
}

YsxAVPacket *ReadNodePkt(pLinkList m_pList,int position) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	   printf("cannot find node position\n");
	    return NULL;
	}
	//pTemp->read_mark = 1;
	return pTemp->pkt;
	
}

int  SetPktReadable(pLinkList m_pList,int position) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	   printf("cannot find node position\n");
	    return -1;
	}
	pTemp->read_mark = 0;
	return 0;
	
}


int  SetPktReadover(pLinkList m_pList,int position) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	    printf("cannot find node position\n");
	    return -1;
	}
	pTemp->read_mark = 1;
	return 0;
	
}

//在链表中插入一个节点。
//插入的位置由beforeWhich指定，新节点插入在beforeWhich之前。
//beforeWhich的取值在1到ListLength+1之间。
int InsertNode(pLinkList m_pList,int beforeWhich, YsxAVPacket *pkt) {
	LNode *pTemp = NULL;
	
	if (beforeWhich < 1){// || beforeWhich > (GetLength(m_pList) + 1)) {
	  return FALSE;
	}
	
	if (!(GetNode(m_pList, beforeWhich - 1, &pTemp))) {
	  return FALSE;
	}
	
	LNode *newNode = (pLinkList)av_malloc( sizeof(LNode));
	if(!newNode){
		printf( "Malloc new node error\n");
		return FALSE;
	}
	newNode->read_mark = 0;
	newNode->pkt = pkt;
	newNode->next = pTemp->next;
	pTemp->next = newNode;
	
	//m_listLength++;
	
	return TRUE;
}

//删除一个指定的节点。
//节点位置由position指定。
//positon的值从1到listLength。
//若链表为空或指定的节点不存在则返回false。
int DeleteNode(pLinkList m_pList,int position) {
	if (position < 1){// || position > GetLength(m_pList)) {
	  return FALSE;
	}
	
	LNode *pTemp = NULL;
	if (!(GetNode(m_pList,position - 1, &pTemp))) {
	  return FALSE;
	}
	
	LNode *pDel = NULL;
	pDel = pTemp->next;
	pTemp->next = pDel->next;
	av_free(pDel);
	
	//m_listLength--;
	
	return TRUE;
}

//得到指定位置节点的指针。
int GetNode(pLinkList m_pList,int position, LNode **node) {
	LNode *pTemp = NULL;
	int curPos = -1;
	
	pTemp = m_pList;
	while (pTemp != NULL) {
	  curPos++;
	  if (curPos == position) 
	   break;
	  pTemp = pTemp->next;
	} 
	if (curPos != position) {
	  return FALSE;
	}
	
	*node = pTemp;
	
	return TRUE;
}

