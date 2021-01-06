#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/sha.h>
#include<openssl/crypto.h>
#include<openssl/err.h>
#include<openssl/hmac.h>
//#include"fwdownload_main.h"

#define DEV_UID "/etc/uid"
#define DEV_VER "/etc/SNIP39/version.conf"


const char base2[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c) (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))
#define isdigit(c) ('0' <= (c) && (c) <= '9')

static char find_pos(char ch)
{
        char *ptr = (char*)strrchr(base2, ch);//the last position (the only) in base[]
        return (ptr - base2);
}

char *base64_decode(const char *data, int data_len,int *len);
char *base64_encode(const char* data, int data_len,int *len);

int get_device_uid(char *uid)
{
        FILE *uid_fd = NULL;

        uid_fd = fopen(DEV_UID,"rb");
        if (uid_fd == NULL) {
                fprintf(stderr, "open %s error\n", DEV_UID);
                return -1;
        }    
        else {
                fgets(uid, 280, uid_fd);    
        }    
        fclose(uid_fd);

        if (!strlen(uid)) {
                fprintf(stderr, "get uid fail\n");
                return -1;
        }    

        printf("UID:%s\n", uid);

        return 0;
}

int get_device_version(char *version)
{
        FILE *ver_fd = NULL;

        ver_fd = fopen(DEV_VER,"rb");
        if (ver_fd == NULL) {
                fprintf(stderr, "open %s error\n", DEV_VER);
                return -1;
        }    
        else {
                fgets(version, 280, ver_fd);    
        }    
        fclose(ver_fd);

        if (!strlen(version)) {
                fprintf(stderr, "get uid fail\n");
                return -1;
        }    

        printf("VERSION:%s\n", version);

        return 0;
}

char *ysx_rsa_pub_encrypt(char *instr, char *path_key, int inlen)
{
    char *p_hex;
    RSA *p_rsa;
    FILE *file;
    int rsa_len=0 , flen=0 , base64_len = 0 , encrypt_size = 0;
	char *base64_data = NULL , *encrypt_data = NULL , *p = NULL; 

    if((file=fopen(path_key,"r"))==NULL){
	    printf("Key read error!\n");
	    return NULL;
    }

    if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL){
        printf("PEM read err!\n");
		fclose(file);		
        return NULL;
    }
    else    
    {
		fclose(file);
    	printf("PEM read success!\n"); 
    }

    flen=inlen;
    rsa_len=RSA_size(p_rsa);

    printf("flen:%d\n",flen); 
    printf("rsa_len:%d\n",rsa_len);
	
	encrypt_data = (char *)malloc(1024);
	if(!encrypt_data){
		perror("malloc for decrypt data error\n");
		RSA_free(p_rsa);
		return NULL;
	}
	memset(encrypt_data,0,1024);

    if(flen > (rsa_len-11))
    {
        int time = flen/(rsa_len-11);
        int remain = flen%(rsa_len-11);
        int i;  

        for(i = 0; i < time; i++)
        {       
            p = instr+i*(rsa_len-11);

            encrypt_size = RSA_public_encrypt(rsa_len-11,(unsigned char *)p,(unsigned char*)(encrypt_data+i*rsa_len),p_rsa,RSA_PKCS1_PADDING);
			if(encrypt_size < 0)
            {
                printf("RSA_public_encrypt error!\n");
				goto finally;
            }

        }
        if(remain > 0)
        {
            p = instr+time*(rsa_len-11);
			
            encrypt_size = RSA_public_encrypt(remain,(unsigned char *)p,(unsigned char*)(encrypt_data+time*rsa_len),p_rsa,RSA_PKCS1_PADDING);
            if(encrypt_size < 0)
            {
				printf("RSA_public_encrypt error!\n");
				goto finally;
            }
        }

        base64_data =  base64_encode(encrypt_data, time*rsa_len+encrypt_size, &base64_len);        	
    }
    else
    {
        if(RSA_public_encrypt(rsa_len-11,(unsigned char *)instr,(unsigned char*)encrypt_data,p_rsa,RSA_PKCS1_PADDING)<0)
        {
			printf("RSA_public_encrypt error!\n");
			goto finally;
        }
        base64_data =  base64_encode(encrypt_data, rsa_len, &base64_len);        	
    }
	printf("[ %s ]-> RSA encrypt success!\n",__FUNCTION__);
		
finally:
	if(p_rsa)
		RSA_free(p_rsa);
	if(encrypt_data)
		free(encrypt_data);

	return base64_data;		
}


char *ysx_rsa_pri_decrypt(char *instr, char *path_key)
{
    RSA *p_rsa;
    FILE *file;
    int rsa_len = 0 ,flen = 0 , ret = -1;

    char *decrypt_data = NULL , *p = NULL;

    if((file=fopen(path_key,"r"))==NULL){
	    perror("open key file error");
	    return NULL;
    }
	
    if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL){
	    printf("PEM read err!\n"); 
		fclose(file);
	    return NULL;
    }
    else
    {
	    fclose(file);		
        printf("PEM read success!\n"); 
    }

    rsa_len=RSA_size(p_rsa);
    printf("flen:%d\n",flen); 
    printf("rsa_len:%d\n",rsa_len);
	
	decrypt_data = (char *)malloc(1024);
	if(!decrypt_data){
		perror("malloc for decrypt data error\n");
		RSA_free(p_rsa);
		return NULL;
	}
	memset(decrypt_data,0,1024);
	
    char * base64_data = base64_decode(instr, strlen(instr),&flen);	

    if(flen > (rsa_len))
    {
        int time = flen/(rsa_len);
        int i;  

        for(i = 0; i < time; i++)
        {
            p = base64_data+i*rsa_len;

            if(RSA_private_decrypt(rsa_len,(unsigned char *)p,(unsigned char*)(decrypt_data+i*(rsa_len-11)),p_rsa,RSA_PKCS1_PADDING)<0)
            {       
                printf("RSA_private_decrypt error!\n");
				ret = -1;
				goto finally;
            }       
        }

    }
    else{
        if(RSA_private_decrypt(rsa_len,(unsigned char *)base64_data,(unsigned char*)decrypt_data,p_rsa,RSA_PKCS1_PADDING)<0)
        {
	        printf("RSA_private_decrypt error!\n");
			ret = -1;			
			goto finally;
        }
    }
	ret = 1;
	printf("[ %s ]-> RSA decrypt success!\n",__FUNCTION__);
		
finally:
	if(p_rsa)
		RSA_free(p_rsa);
	if(base64_data)
		free(base64_data);

	if(!ret)
	{
		free(decrypt_data);
		decrypt_data = NULL;
	}

	return decrypt_data;	
}

char *ysx_rsa_pub_decrypt(char *instr, char *path_key)
{
    RSA *p_rsa = NULL;
    FILE *file = NULL;
    int rsa_len = 0, flen = 0 , ret = 0;

    char *decrypt_data = NULL , *p = NULL;

    if((file=fopen(path_key,"r"))==NULL){
        perror("open key file error");
		return NULL;
    }
    if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL){
        printf("PEM read err!\n"); 
    	fclose(file);
		return NULL;
	}
    else
    {
		fclose(file);		
        printf("PEM read success!\n"); 
    }

    rsa_len=RSA_size(p_rsa);
    printf("flen:%d\n",flen); 
    printf("rsa_len:%d\n",rsa_len);
	
	decrypt_data = (char *)malloc(1024);
	if(!decrypt_data){
		perror("malloc for decrypt data error\n");
		RSA_free(p_rsa);
		return NULL;
	}
	memset(decrypt_data,0,1024);
	
    char * base64_data = base64_decode(instr, strlen(instr),&flen);	

    if(flen > (rsa_len))
    {
        int time = flen/(rsa_len);
        int i;  

        for(i = 0; i < time; i++)
        {
            p = base64_data+i*rsa_len;

            if(RSA_public_decrypt(rsa_len,(unsigned char *)p,(unsigned char*)(decrypt_data+i*(rsa_len-11)),p_rsa,RSA_PKCS1_PADDING)<0)
            {       
                printf("RSA_public_decrypt error!\n");
				ret = -1;
				goto end;
            }       
        }
    }
    else{

        if(RSA_public_decrypt(rsa_len,(unsigned char *)base64_data,(unsigned char*)decrypt_data,p_rsa,RSA_PKCS1_PADDING)<0)
        {
			printf("RSA_public_decrypt error!\n");
			ret = -1;
			goto end;
        }
    }	
	ret = 1;
	printf("[ %s ]-> RSA decrypt success!\n",__FUNCTION__);
	
end:
	if(p_rsa)
		RSA_free(p_rsa);
	if(base64_data)
		free(base64_data);
	
	if(!ret)
	{
		free(decrypt_data);
		decrypt_data = NULL;
	}

	return decrypt_data;	
}


char *ysx_rsa_pri_encrypt(char *instr,  int inlen , char *path_key)
{
	char *p_hex;
	RSA *p_rsa;
	FILE *file = NULL;
	int rsa_len , base64_len = 0 , encrypt_size = 0;
	int flen;
	char *encrypt_data;
	char *p, *base64_data = NULL;
	
    if((file=fopen(path_key,"r"))==NULL){
        printf("Key read error!\n");
        return NULL;
    }

    if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL){
        printf("PEM read err!\n");
		fclose(file);		
        return NULL;
    }
    else    
    {
		fclose(file);		
        printf("PEM read success!\n"); 
    }

	flen=inlen;
    rsa_len=RSA_size(p_rsa);

    printf("flen:%d\n",flen); 
    printf("rsa_len:%d\n",rsa_len);

	encrypt_data = (char *)malloc(1024);
	memset(encrypt_data,0,1024);

    if(flen > (rsa_len-11))
    {
	    int time = flen/(rsa_len-11);
	    int remain = flen%(rsa_len-11);
		int i;  

        for(i = 0; i < time; i++)
        {  
            p = instr+i*(rsa_len-11);

            encrypt_size = RSA_private_encrypt(rsa_len-11,(unsigned char *)p,(unsigned char*)(encrypt_data+i*rsa_len),p_rsa,RSA_PKCS1_PADDING);
			if(encrypt_size < 0)
            {
                printf("RSA_private_encrypt error!\n");
				goto finally;
            }
			printf("encrypt size = %d\n",encrypt_size);
        }

        if(remain > 0)
        {
            p = instr+time*(rsa_len-11);

            encrypt_size = RSA_private_encrypt(remain,(unsigned char *)p,(unsigned char*)(encrypt_data+time*rsa_len),p_rsa,RSA_PKCS1_PADDING);
            if(encrypt_size<0)
            {
                printf("RSA_private_encrypt error!\n");
				goto finally;
			}
        }
		printf("before base64 %d\n",rsa_len*time);
        encrypt_data[rsa_len*time+encrypt_size] = '\0';
        base64_data = base64_encode(encrypt_data, time*rsa_len+encrypt_size, &base64_len);	
	}
	else    
    {
    	rsa_len=RSA_size(p_rsa);
  
        encrypt_size = RSA_private_encrypt(rsa_len-11,(unsigned char *)instr,(unsigned char*)encrypt_data,p_rsa,RSA_PKCS1_PADDING);
		if(encrypt_size < 0)
        {
			printf("RSA_private_encrypt error!\n");		
       		goto finally;
		}

        base64_data = base64_encode(encrypt_data, rsa_len, &base64_len);
	}
	
	printf("[ %s ]-> RSA encrypt success!\n",__FUNCTION__);
	
finally:	
	if(p_rsa)
		RSA_free(p_rsa);
	if(encrypt_data)	
		free(encrypt_data);	
	
	return base64_data;	
}


char *base64_encode(const char* data, int data_len,int *len)  
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
        ret = (char *)malloc(ret_len);  

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
                        *f = base2[changed[i]];  

                        f++;
                        (*len)++; 
                }  
        }  
        *f = '\0';  

        return ret;  

}

char *base64_decode(const char *data, int data_len,int *len)  
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

        ret = (char *)malloc(ret_len);  
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



void ysx_aes_cbc_encrypt2(unsigned char* in, int inl, unsigned char *out, int* len, char * key)
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

void ysx_aes_cbc_decrypt2(unsigned char* in, int inl, unsigned char *out, unsigned char *key)
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

