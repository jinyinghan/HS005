#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ysx_sys.h"

int ysx_read_config(const char *symbol, int *value)
{
    int ret = 0;
    char line_buf[128];
    char * match;
    FILE * fp;
    fp = fopen(CAMERA_CONF, "r");
    if (NULL == fp) {
        ret = errno > 0 ? errno : -1;
        printf("open file failed with: %s\n", strerror(ret));
        return -1;
    }

    if (NULL == symbol || NULL == value) {
        printf("argument error!\n");
        return -1;
    }
    
    while(fgets(line_buf,128,fp) != NULL)
    {
        if ((match = strstr(line_buf,symbol)) != NULL) {
            match += strlen(symbol);
            *value= atoi(match);
        }       
    }
     
    fclose(fp);
    return 0;    
}

