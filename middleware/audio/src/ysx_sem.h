#ifndef _YSX_SEM_H_
#define _YSX_SEM_H_
#include "ysx_audio.h"
#include <sys/sem.h>

union semun  
{  
    int val;  
    struct semid_ds *buf;  
    unsigned short *arry;  
};  

 int set_semvalue();  
 void del_semvalue();  
 int semaphore_p();  
 int semaphore_v(); 


extern int sem_id;


#endif  /*_YSX_SEM_H_*/
