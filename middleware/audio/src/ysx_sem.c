#include "ysx_audio.h"
#include <sys/sem.h>

union semun  
{  
    int val;  
    struct semid_ds *buf;  
    unsigned short *arry;  
};  

static int set_semvalue(int sem_id)  
{  
    //���ڳ�ʼ���ź�������ʹ���ź���ǰ����������  
    union semun sem_union;  
  
    sem_union.val = 1;  
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)  
        return 0;  
    return 1;  
}  
  
static void del_semvalue(int sem_id)  
{  
    //ɾ���ź���  
    union semun sem_union;  
  
    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)  
        fprintf(stderr, "Failed to delete semaphore\n");  
}  
  
static int semaphore_p(int sem_id)  
{  
    //���ź�������1���������ȴ�P��sv��  
    struct sembuf sem_b;  
    sem_b.sem_num = 0;  
    sem_b.sem_op = -1;//P()  
    sem_b.sem_flg = SEM_UNDO;  
    if(semop(sem_id, &sem_b, 1) == -1)  
    {  
        fprintf(stderr, "semaphore_p failed\n");  
        return 0;  
    }  
    return 1;  
}  
  
static int semaphore_v(int sem_id)  
{  
    //����һ���ͷŲ�������ʹ�ź�����Ϊ���ã��������ź�V��sv��  
    struct sembuf sem_b;  
    sem_b.sem_num = 0;  
    sem_b.sem_op = 1;//V()  
    sem_b.sem_flg = SEM_UNDO;  
    if(semop(sem_id, &sem_b, 1) == -1)  
    {  
        fprintf(stderr, "semaphore_v failed\n");  
        return 0;  
    }  
    return 1;  
}  

