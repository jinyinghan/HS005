#include<stdio.h>
int main(){
    int val;
    while(1){
        val = QCamGetKeyStatus();
        printf("%d\n",val);
        sleep(1); 
    
    
    }



}
