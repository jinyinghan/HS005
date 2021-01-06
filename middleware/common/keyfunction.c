#include<stdio.h>
#include <sysutils/su_misc.h>
int main(){
    /*
    SUKeyEvent event;
    int ret, key_code;
    int evfd ; 
    
    evfd = SU_Key_OpenEvent();
    printf("%d\n",evfd);
    if (evfd < 0) {
        printf("Key event open error\n");
        return ;
    }
    printf("while main\n");
    while (1) {
        ret = SU_Key_ReadEvent(evfd, &key_code, &event);
        if (ret != 0) {
            printf("Get Key event error\n");
            return NULL;
        }

        printf("%d %d\n",key_code,event);
    }

    */
    int ret ;
    while(1){
        ret = QCamGetFunctionKeyStatus();
        if(ret >= 0)
            printf("%s\n",ret == 0 ? "key pressed" : "key release"); 
        else
            printf("get keyfunc failed\n");
        usleep(10*1000); 
    }


}
