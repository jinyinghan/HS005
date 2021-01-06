#include<stdio.h>
#include <unistd.h>
static int g_wdt_enalbe = 0;
int main(){
    unsigned int ticket = 0;
    g_wdt_enalbe = 1;
    QCamWatchdogEnable(1);
    QCamWatchdogSetTimeout(20);
    while(1){
        if (ticket % 30 == 0) {
            if (g_wdt_enalbe) {
                QCamWatchdogKeepAlive();
            }
        }
        if (0 == access("/tmp/stop_wdt", F_OK)) {
            if (g_wdt_enalbe) {
                QCamWatchdogEnable(0);
                g_wdt_enalbe = 0;
                printf("XXXXXXXXXXXXXXXXXXXXXXX stop wdt\n");
                remove("/tmp/stop_wdt");
            }
        }
        sleep(1);
        ticket ++;
    }




}
