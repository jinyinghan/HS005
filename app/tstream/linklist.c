/*
 * linklist.c
 *
 *  Created on: 2013-8-12
 *  Author: wanglei
 */
#include "linklist.h"

//��ʼ��������һ��ͷ�ڵ㡣
int InitList(pLinkList m_pList) {
	if (!(m_pList = (pLinkList)av_malloc(sizeof(LNode)))) {
	  return FALSE;
	}
	m_pList->next = NULL;
	
	return TRUE;
}

//��������
int DestroyList(pLinkList m_pList) {
	if (!ClearList(m_pList)) {
	  return FALSE;
	}
	
	av_free(m_pList);
	
	return TRUE;
}

//�ж������Ƿ�Ϊ�ա���Ϊ�գ�����true�����򷵻�false��
int IsEmpty(pLinkList m_pList) {
	if (m_pList->next == NULL) {
	  return TRUE;
	}
	return FALSE;
}

//����������е�ǰ�ڵ�����
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

//��������գ��ͷŵ�ǰ���нڵ㡣
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

//��positionָ���Ľڵ��ڵ���������ΪnewPkt
//��һ����Ч�ڵ��positionΪ1��
int SetNodePkt(pLinkList m_pList,int position, YsxAVPacket *newpkt) {
	LNode *pTemp = NULL;
	
	if (!(GetNode(m_pList, position, &pTemp))) {
	  return FALSE;
	}
	
	pTemp->pkt = newpkt;
	
	return TRUE;
}

//�õ�ָ��λ�ýڵ�����ݡ�
//�ڵ�������1��listLength��
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

//�������в���һ���ڵ㡣
//�����λ����beforeWhichָ�����½ڵ������beforeWhich֮ǰ��
//beforeWhich��ȡֵ��1��ListLength+1֮�䡣
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

//ɾ��һ��ָ���Ľڵ㡣
//�ڵ�λ����positionָ����
//positon��ֵ��1��listLength��
//������Ϊ�ջ�ָ���Ľڵ㲻�����򷵻�false��
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

//�õ�ָ��λ�ýڵ��ָ�롣
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

