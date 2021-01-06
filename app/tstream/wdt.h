/*************************************************************************
	> File Name: wdt.h
	> Author: unixcc
	> Mail: 2276626887@qq.com 
	> Created Time: Sat 24 Aug 2019 08:18:54 PM CST
 ************************************************************************/

#ifndef _WDT_H_
#define _WDT_H_

int wdt_keep_alive(void);
int wdt_enable(void);
int wdt_disable(void);
int wdt_set_timeout(int to);
int wdt_get_timeout(void);








#endif 
