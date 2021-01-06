#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <openssl/evp.h>

const char base3[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c) (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))
#define isdigit(c) ('0' <= (c) && (c) <= '9')

char find_pos(char ch)
{
    char *ptr = (char*)strrchr(base3, ch);//the last position (the only) in base[]
    return (ptr - base3);
}

char *ysx_base64_encode(const char* data, int data_len,int *len)  
{     
        int prepare = 0;  
        int ret_len;  
        *len=0;
        int temp = 0;  
        char *ret = NULL;  
        char *f = NULL;  
        int tmp = 0;  
        char changed[4];  
        int i = 0;  
        ret_len = data_len / 3;  
        temp = data_len % 3;  
        if (temp > 0)  
        {  
                ret_len += 1;  
        }  
        //最后一位以''结束 
        ret_len = ret_len*4 + 1;  
        ret = (char *)mem_malloc(ret_len);  

        if ( ret == NULL)  
        {  
                printf("No enough memory.n");  
                exit(0);  
        }  
        memset(ret, 0, ret_len);  
        f = ret;
        //tmp记录data中移动位置  
        while (tmp < data_len)  
        {  
                temp = 0;  
                prepare = 0;  
                memset(changed, 0, 4);  
                while (temp < 3)  
                {     
                        if (tmp >= data_len)  
                        {  
                                break;  
                        } 
                        //将data前8*3位移入prepare的低24位 
                        prepare = ((prepare << 8) | (data[tmp] & 0xFF));  
                        tmp++;  
                        temp++;  
                }  
                //将有效数据移到以prepare的第24位起始位置
                prepare = (prepare<<((3-temp)*8));  

                for (i = 0; i < 4 ;i++ )  
                {  
                        //最后一位或两位
                        if (temp < i)  
                        {  
                                changed[i] = 0x40;  
                        }  
                        else
                        {  
                                //24位数据
                                changed[i] = (prepare>>((3-i)*6)) & 0x3F;  
                        }  
                        *f = base3[changed[i]];  

                        f++;
                        (*len)++; 
                }  
        }  
        *f = '\0';  

        return ret;  

}

char *ysx_base64_decode(const char *data, int data_len,int *len)  
{  
    int ret_len = (data_len / 4) * 3+1;  
    int equal_count = 0;  
    char *ret = NULL;  
    char *f = NULL;  
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
       
    ret = (char *)mem_malloc(ret_len);  
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
    /*
    while(*(--f)=='')
    {
      (*len)--;
         
    }
    */
    return ret;  
}

void ysx_aes_cbc_encrypt(unsigned char* in, int inl, unsigned char *out, int* len, char * key)
{
    unsigned char iv[16];
    EVP_CIPHER_CTX ctx;
    int i;
    strncpy((char*)iv, key, 16);

    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, (unsigned char *)key, iv);  

    *len = 0;
    int outl = 0;
    if((inl%16)==0) {
        for (i=0; i<16; ++i)
            in[inl+i] = 16; 
        inl += 16; 
    }   

    EVP_EncryptUpdate(&ctx, out+*len, &outl, in+*len, inl);
    *len+=outl;
    int test = inl>>4;
    if(inl != test<<4){
        EVP_EncryptFinal_ex(&ctx,out+*len,&outl);  
        *len+=outl;
    }   

    EVP_CIPHER_CTX_cleanup(&ctx);
}

void ysx_aes_cbc_decrypt(unsigned char* in, int inl, unsigned char *out, unsigned char *key)
{
        unsigned char iv[10000];
        EVP_CIPHER_CTX ctx;
        EVP_CIPHER_CTX_init(&ctx);

        strncpy((char*)iv, (char*)key, 16);

        EVP_DecryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, (unsigned char *)key, iv); 
        int len = 0;
        int outl = 0;

        EVP_DecryptUpdate(&ctx, out+len, &outl, in+len, inl);
        len += outl;

        EVP_DecryptFinal_ex(&ctx, out+len, &outl);  
        len+=outl;
        out[len]=0;
        EVP_CIPHER_CTX_cleanup(&ctx);
}

