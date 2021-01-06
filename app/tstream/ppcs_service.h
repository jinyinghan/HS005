#ifndef __PPCS_SERVICE_H_
#define __PPCS_SERVICE_H_

int ppcs_service_write(const int SessionID, const int Channel, const void *data, const int len);
void ppcs_service_set_param(char *didCode,char *initCode,char *crcKey);
void init_ppcs_service(void);
void uninit_ppcs_service(void);

#endif

