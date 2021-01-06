#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h> 
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


#include "qcam.h"
#include "zbar.h"
#define LOG printf



#define YUV_SIZE_W    640
#define YUV_SIZE_H    360


#define QR_TEMP_FILE             "/tmp/QR.temp"            // QR 扫描临时文件

volatile int g_detect_flag = 1;
char g_result[4096] = {};
const char *g_result_tag = NULL;

static __inline pthread_t util_pthread_create( void *( *start)(void *), void *arg, int stack_size)
{
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if(stack_size > 0)
    {
        pthread_attr_setstacksize(&attr, stack_size);
    }

    pthread_t tid = 0;
    pthread_create(&tid, &attr, start, arg);
    pthread_attr_destroy(&attr);
    return tid;
}

static void sig_handler(int arg)
{
    LOG("Receive signal to terminate \n");
    g_detect_flag = 0;
}

void * zbar_proc_thr(void * arg)
{
    int size;
    int lp;
    FILE *fp;
    const char *data;

    char buf[YUV_SIZE_W*YUV_SIZE_H*3/2];
    //int ret = 0;
    zbar_image_scanner_t *scanner = NULL;
    zbar_image_t *image = NULL;

    size = sizeof(buf);
    /* create a reader */
    scanner = zbar_image_scanner_create();
    /* configure the reader */
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 0);
    zbar_image_scanner_set_config(scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    
    /* wrap image data */
    image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, YUV_SIZE_W, YUV_SIZE_H);
    while (g_detect_flag)
    {
        for(lp=0;lp<5 && g_detect_flag;lp++)
        {
            usleep(100*1000);
        }

        if(g_detect_flag == 0)
            break;

        if(QCAM_OK != QCamVideoInput_CatchYUV(YUV_SIZE_W, YUV_SIZE_H, buf, size))
        {
            continue;
        }
#if 0
        FILE *sv_fp = fopen("/tmp/raw.nv12","wb");
        int len = fwrite(buf,1,size,sv_fp);
        printf("len = %d\n",len);
        fclose(sv_fp);
        exit(0);
#endif        
        if(g_detect_flag == 0)
            break;

        const zbar_symbol_t *symbol;
        
        //无需释放内存
        //zbar_image_set_data(image, buf,size, zbar_image_free_data);
        zbar_image_set_data(image, buf,size, NULL);

        /* scan the image for barcodes */
        zbar_scan_image(scanner, image);//cost much time if the picture's size is very big
        /* extract results */
        symbol = zbar_image_first_symbol(image);
        for (; symbol; symbol = zbar_symbol_next(symbol)) {
            /* do something useful with results */
            zbar_symbol_type_t type = zbar_symbol_get_type(symbol);
            data = zbar_symbol_get_data(symbol);
            LOG("zbar symbol type=[%s] str=[%s]\n", zbar_get_symbol_name(type), data);
            fp = fopen("/tmp/qrcode.txt","w");            
            fwrite(data,strlen(data),1,fp);
            fclose(fp);
            g_detect_flag = 0;
            goto end;
            // 有些场景例如电视、海报有其他的二维码干扰，
            // 导致声波也扫不出来，所以要判断是不是我们的二维码
            
        }
    }
end:    
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);
    return NULL;
}
void qr_scan_init(void)
{
    QCamVideoInput_Init();

    // ensure stack for buf[YUV_SIZE_W*YUV_SIZE_H]
    util_pthread_create(zbar_proc_thr, NULL, 1.5*1024*1024);
}

int main (int argc, char *argv[])
{
    signal(SIGINT, sig_handler); 
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);



    LOG("init...\n");

    memset(g_result, 0, sizeof(g_result));

    qr_scan_init();
    LOG("init video finish.\n");

    while(g_detect_flag)
    {
        usleep(100*1000);
    }

    LOG("uninit video...\n");
    QCamVideoInput_Uninit();

    LOG("exit.\n");

    return 0;
}
