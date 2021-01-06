#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define FACTOR_1 17445
#define FACTOR_2 32463

static const unsigned char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static unsigned char *base64_decode(const unsigned char *data, int data_len,int *len);
static void decrypt_msg(unsigned char *data, unsigned int offset, unsigned int length, int key1, int key2);

static int decrypt_key2mac(unsigned char *key, unsigned char *mac, int key_len, unsigned int num)
{
    int i = 0, j = 0, len = 0;
    unsigned char buf[36];
    unsigned char tmp_mac[6];
    unsigned char *tmp = NULL;

    memset(buf, 0, sizeof(buf));
    tmp = base64_decode(key, key_len, &len);

    if(tmp && sizeof(buf) == len) {
        memcpy(buf, tmp, len);
        free(tmp);
        
        decrypt_msg(buf, 0, sizeof(buf), num%FACTOR_1+FACTOR_1, FACTOR_2);
        for(i=5, j=0; i<=sizeof(buf); i+=6, j++)
        {
            tmp_mac[j] = buf[i];
        }
    
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                tmp_mac[0], tmp_mac[1], tmp_mac[2], tmp_mac[3], tmp_mac[4], tmp_mac[5]);
        return 0;
    }

    return -1;
}

static void decrypt_msg(unsigned char *data, unsigned int offset, unsigned int length, int key1, int key2) 
{
    int K1, K2,i, C1, C2;

    K1 = key1;
    K2 = key2;
    C1 = (K2 >> 10) % 1024 + 371;
    C2 = (K2 % 1024) + 829;

    for(i = offset; i < length; i++) {
        char temp =  data[i] ^ K1;
        K1 = ((data[i] + K1) * C1 + C2) % 4096 + 111;
        data[i] = temp;
    }
    return ;
}

static char find_pos(unsigned char ch)
{
    unsigned char *ptr = (unsigned char*)strrchr(base, ch);//the last position (the only) in base[]
    return (ptr - base);
}

static unsigned char *base64_decode(const unsigned char *data, int data_len,int *len)  
{  
    int ret_len = (data_len / 4) * 3+1;  
    int equal_count = 0;  
    unsigned char *ret = NULL;  
    unsigned char *f = NULL;  
    *len=0;
    int tmp = 0;  
    int temp = 0;  
    char need[3];  
    int prepare = 0;  
    int i = 0;  
    if (*(data + data_len - 1) == '=')  
    {  
        equal_count += 1;  
    }  
    if (*(data + data_len - 2) == '=')  
    {  
        equal_count += 1;  
    }  

    ret = (unsigned char *)malloc(ret_len);  
    if (ret == NULL)  
    {  
        printf("No enough memory.n");  
        exit(0);  
    }  
    memset(ret, 0, ret_len);  
    f = ret;  
    while (tmp < (data_len - equal_count))  
    {  
        temp = 0;  
        prepare = 0;  
        memset(need, 0, 4);  
        while (temp < 4)  
        {  
            if (tmp >= (data_len - equal_count))  
            {  
                break;  
            }  
            prepare = (prepare << 6) | (find_pos(data[tmp]));  
            temp++;  
            tmp++;  
        }  
        prepare = prepare << ((4-temp) * 6);  
        for (i=0; i<3 ;i++ )  
        {  
            if (i == temp)  
            {  
                break;  
            }  
            *f = (char)((prepare>>((2-i)*8)) & 0xFF);  
            f++;
            (*len)++;  
        }  
    }
    *f = '\0';
    if(data[data_len-1]=='=')
    {
        (*len)--;
    }
    
    return ret;  
}

static int nvram_get_str(const char *word, char *buf, int len)
{
    FILE *read_fp = NULL;
    char cmd[128];
    
    memset(cmd, '\0', sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "nvram_get %s", word);
    
    read_fp = popen(cmd, "r");
    if (read_fp != NULL) {
        fread(buf, 1, len, read_fp);
        pclose(read_fp);
        read_fp = NULL;
    }

    if(strlen(buf) == 0) {
        return -1;
    }
        
    return 0;
    
}

static void convert_mac_to_uper(char mac[], int len)
{
    int i = 0, j = 0, k = 0;
    char ptr[len];

    memset(ptr, 0, len);

    for(i=0; mac[i]!='\0'; i++)
    {
        if(mac[i] >= 'a' && mac[i] <= 'z') {
            mac[i] -= 32;
        }
    }
    
#if 1
    if(NULL == strstr(mac, ":")) {
        for(i=j=0,k=2;mac[j]!='\0';i++) {
            if(i == k) {
                ptr[i] = ':';
                k += 3;
            }else {
                ptr[i] = mac[j];
                j++;
            }
        }
        ptr[i+1] = '\0';
        memcpy(mac, ptr, strlen(ptr));
    }
#endif
}

int ysx_verify(void)
{
    unsigned char mac_cmp[18];
    unsigned char mac_dec[18];
    unsigned char mac_hex[6];
    unsigned char key_get[52];
    unsigned char key[49];
    int ret = 0;
    unsigned int num = 0;

    // 通过nvram_get获取MAC和KEY
    memset((mac_cmp), '\0', sizeof((mac_cmp)));
    memset(key_get, '\0', sizeof(key_get));
    memset(key, '\0', sizeof(key));
    
    if((ret=nvram_get_str("MAC", (mac_cmp), sizeof((mac_cmp)))) < 0) return -1;
    if((ret=nvram_get_str("KEY", key_get, sizeof(key_get))) < 0) return -1;

#if 0
    if(NULL == strstr(mac_cmp, ":")) {
        printf("MAC addr format error!\n");
        return -1;
    }
#endif

    // 去掉KEY的前缀
    if(NULL == strstr(key_get, "YSX")) return -1;
    memcpy(key, key_get+3, sizeof(key));

    // 转换大小写
    convert_mac_to_uper(mac_cmp, sizeof(mac_cmp));

    // 转十六进制
    sscanf(mac_cmp, "%02x:%02x:%02x:%02x:%02x:%02x",
            &mac_hex[0], &mac_hex[1], &mac_hex[2], &mac_hex[3], &mac_hex[4], &mac_hex[5]);
    
    num = ((num | mac_hex[5]) << 16) + ((num | mac_hex[4]) << 8) + (num | mac_hex[3]);

    memset(mac_dec, '\0', sizeof(mac_dec));
    ret = decrypt_key2mac(key, mac_dec, sizeof(key)-1, num);
    if(0 == ret) {
        ret = memcmp(mac_dec, (mac_cmp), strlen(mac_dec));
    }

    return ret;
}

#if 0
int main(int argc, char *argv[])
{
    int ret = ysx_verify();
    if(ret) {
        printf("verify failed!\n");
    }else {
        printf("verify success!\n");
    }
    return 0;
}
#endif
