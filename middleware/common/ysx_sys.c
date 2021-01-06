#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "zlib.h"
#include <errno.h>
#include "ysx_sys.h"
#include "qcam_sys.h"
#include <sysutils/su_misc.h>


static char buf[BUF_SIZE];
#define KEY_EVENT  1

#ifdef KEY_EVENT

static int evfd;
static QCam_Key_Status_cb key_event_cb;

#endif 

typedef struct image_header {  
        uint32_t ih_magic; /* Image Header Magic Number */  //镜像头号  
        uint32_t ih_hcrc; /* Image Header CRC Checksum */   //镜像头部检验和  
        uint32_t ih_time; /* Image Creation Timestamp */    //镜像创建时间  
        uint32_t ih_size; /* Image Data Size  */            //大小  (单位是kbyte)
        uint32_t ih_load; /* Data  Load  Address  */        //数据加载地址  
        uint32_t ih_ep;  /* Entry Point Address  */         //入口地址  
        uint32_t ih_dcrc; /* Image Data CRC Checksum */     //镜像数据校验和  
        uint8_t  ih_os;  /* Operating System  */            //操作系统类型  
        uint8_t  ih_arch; /* CPU architecture  */           //cpu架构  
        uint8_t  ih_type; /* Image Type   */                //镜像类型  
        uint8_t  ih_comp; /* Compression Type  */           //压缩类型  
        uint8_t  ih_name[32]; /* Image Name  */       //镜像名  
} image_header_t; 

#define IH_MAGIC    0x27051956  /* Image Magic Number       */
#define IH_ARCH_MIPS        5   /* MIPS     */
#define IH_TYPE_INVALID     0   /* Invalid Image        */
#define IH_TYPE_STANDALONE  1   /* Standalone Program       */
#define IH_TYPE_KERNEL      2   /* OS Kernel Image      */
#define IH_TYPE_RAMDISK     3   /* RAMDisk Image        */
#define IH_TYPE_MULTI       4   /* Multi-File Image     */
#define IH_TYPE_FIRMWARE    5   /* Firmware Image       */
#define IH_TYPE_SCRIPT      6   /* Script file          */
#define IH_TYPE_FILESYSTEM  7   /* Filesystem Image (any type)  */
#define IH_TYPE_FLATDT      8   /* Binary Flat Device Tree Blob */
#define IH_TYPE_KWBIMAGE    9   /* Kirkwood Boot Image      */
#define IH_TYPE_IMXIMAGE    10  /* Freescale IMXBoot Image  */
#define IH_TYPE_UBLIMAGE    11  /* Davinci UBL Image        */
#define IH_TYPE_OMAPIMAGE   12  /* TI OMAP Config Header Image  */
#define IH_TYPE_AISIMAGE    13  /* TI Davinci AIS Image     */
#define IH_TYPE_KERNEL_NOLOAD   14  /* OS Kernel Image, can run from any load address */
#define IH_TYPE_PBLIMAGE    15  /* Freescale PBL Boot Image */

#define _CMD_LEN    (256)

static void _close_all_fds (void)
{
    int i;
    for (i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
      if (i != STDIN_FILENO && i != STDOUT_FILENO && i != STDERR_FILENO)
        close(i);
    }
}
extern int __libc_fork (void);
static int _system(char *command)
{
    int pid = 0;
    int status = 0;
    char *argv[4];
    extern char **environ;

    if (NULL == command) {
        return -1;
    }

    pid = __libc_fork();        /* vfork() also works */
    if (pid < 0) {
        return -1;
    }
    if (0 == pid) {             /* child process */
        _close_all_fds();       /* 杩欐槸鎴戣嚜宸卞啓鐨勪竴涓嚱鏁帮紝鐢ㄦ潵鍏抽棴鎵€鏈夌户鎵跨殑鏂囦欢鎻忚堪绗︺€傚彲涓嶇敤 */
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = NULL;

        execve ("/bin/sh", argv, environ);    /* execve() also an implementation of exec() */
        _exit (127);
    }

 
    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR) {
            status = -1; /* error other than EINTR from waitpid() */
            break;
        }

    return (status);
}

static int AMCSystemCmd (const char *format, ...)
{
    char cmdBuff[_CMD_LEN];
    va_list vaList;
    int i_ret = 0;
    int i_dbg = 0;
    va_start (vaList, format);
    vsnprintf ((char *)cmdBuff, sizeof(cmdBuff), format, vaList);
    va_end (vaList);
    
    i_ret = _system ((char *)cmdBuff);
    if ((i_ret) && (i_dbg == 1)){
        pr_error("err %s, cmd: %s\n",strerror(errno), cmdBuff);
        LOG("err %s, cmd: %s\n",strerror(errno), cmdBuff);
        }
    
    return i_ret;
}

 __inline__ int ysx_gpio_exist(int pin)
{
        CLEAR(buf);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d",pin);
        if(access(buf,F_OK) == 0)  
                return 1;
        else
                return 0;
}

__inline__ int ysx_gpio_open(int pin)
{
        if(pin < 0 || pin > 96){
                printf("illegal pin number!\n");
                return -1;
        }
        char buf_num[64] = {0};

        if(ysx_gpio_exist(pin))  //already exit 
                return 0;

        CLEAR(buf);
        //snprintf(buf,BUF_SIZE,"echo %d > /sys/class/gpio/export",pin);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/export");
        int len = snprintf(buf_num,64,"%d",pin);
        //printf("%s\n",buf_num);

       int  fd = open(buf,O_WRONLY);
        if(fd < 0)
        {
                printf("open gpio %d error !\n",pin);
                return -1;        
        }
        write(fd,buf_num,len);
        close(fd);
         
        //int ret = AMCSystemCmd(buf);
        //if(ret < 0)
         //       printf("%s\n",strerror(errno));
        return 0;
}

__inline__ int ysx_gpio_read(int pin, int *val)
{
        int fd;
        char value_str[5];

        CLEAR(buf);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/direction",pin);
        fd = open(buf,O_WRONLY);
        if(fd < 0)
        {
                printf("set gpio %d direction error !\n",pin);
                return -1;        
        }
        write(fd,"in",sizeof("in"));
        close(fd);

        CLEAR(buf);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/value",pin);
        fd = open(buf,O_RDONLY);
        if(fd < 0)
        {
                printf("set gpio %d direction error !\n",pin);
                return -1;        
        }
        CLEAR(value_str);
        read(fd, value_str, 5);   
        close(fd);
        if(NULL != val)
                *val = atoi(value_str);

        return 0;
}

__inline__ int ysx_gpio_write(int pin , int value)
{
        int fd;

        CLEAR(buf);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/direction",pin);
        fd = open(buf,O_RDWR);
        if(fd < 0)
        {
                printf("set gpio %d direction error [%d] %s!\n",pin,__LINE__,strerror(errno));
                return -1;        
        }
        CLEAR(buf);
        read(fd, buf, 5); 
        if(!strstr(buf,"out"))
        {	
                write(fd,"out",sizeof("out"));
        }	
        close(fd);

        CLEAR(buf);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/value",pin);
        fd = open(buf,O_WRONLY);
        if(fd < 0)
        {
                printf("set gpio %d direction error [%d]!\n",pin,__LINE__);
                return -1;        
        }
        if(value == 0)
                write(fd,"0",sizeof("0"));
        else
                write(fd,"1",sizeof("1"));

        close(fd);


        return 0;
}

__inline__ int ysx_gpio_close(int pin)
{
        if(pin < 0 || pin > 96){
                printf("illegal pin number!\n");
                return -1;
        }
        char buf_num[2] = {0};
        if(!ysx_gpio_exist(pin))  // not exit 
                return 0;

        CLEAR(buf);
        //snprintf(buf,BUF_SIZE,"echo %d > /sys/class/gpio/unexport",pin);
        snprintf(buf,BUF_SIZE,"/sys/class/gpio/unexport");
        int len = snprintf(buf_num,64,"%d",pin);

        int fd = open(buf,O_WRONLY);
        if(fd < 0)
        {
                printf("open gpio %d error !\n",pin);
                return -1;        
        }
        write(fd,buf_num,len);
        close(fd);

        //AMCSystemCmd(buf);

        return 0;
}

static void pwm_usage()
{   
        printf("please input: pwm_id duty time periodt time\n");  
        printf("pwm_id: can be 0 , 1 ,2 ,3\n");
        printf("duty :duty keep time(millisecond ) must > 0\n");
        printf("period :period time (millisecond ) must > 0\n");
}


static __inline__ int ysx_pwm_set(int id, int duty,int period)
{
        int ret,fd;
        struct pwm_ioctl_t pwm_param;

        memset(&pwm_param, 0 , sizeof(struct pwm_ioctl_t));


        if( id < 0 || id > 3)
        {  
                pwm_usage();
                return FAIL;
        }


        pwm_param.duty= duty * ONE_MSEC;
        pwm_param.period= period * ONE_MSEC; 
        pwm_param.index = id;
        pwm_param.polarity = 1;


        if((pwm_param.period < 200) || (pwm_param.period > 1000000000)) {
                printf("period error !\n");
                return -1;
        }

        if((pwm_param.duty < 0) || (pwm_param.duty > pwm_param.period)) {
                printf("duty error !\n");
                return -1;
        }

        //    printf("id=%d,duty=%d,period=%d\n",pwm_param.index,pwm_param.duty,pwm_param.period);
        fd =  open(PWM_DEVICE, O_RDONLY);
        if(fd < 0)
        {
                printf("open the file %s failed\n",PWM_DEVICE);
                return FAIL;
        }

        ret = ioctl(fd, PWM_CONFIG, &pwm_param);
        if(ret != 0) {
                printf("ioctl : %d error !\n", __LINE__);
                close(fd); 
                return FAIL;
        }
        #if 0
        if(pwm_param.duty == 0){
                printf("led:%d LED_OFF:%d\n",LED_GREEN,LED_OFF);
                ret = ioctl(fd, PWM_DISABLE, id);
                ysx_gpio_open(LED_GREEN);
                ysx_gpio_write(LED_GREEN,LED_OFF);	
                ysx_gpio_open(LED_BLUE);
                ysx_gpio_write(LED_BLUE,LED_OFF);		
        }
        else 
        #endif       
                ret = ioctl(fd, PWM_ENABLE, id);
        if(ret != 0) {
                printf("ioctl : %d error !\n", __LINE__);
                close(fd); 
                return FAIL;
        }

        close(fd);
        return SUCCESS;
}

static int set_direction(int pin)
{
         int fd;
 
         CLEAR(buf);
         snprintf(buf,BUF_SIZE,"/sys/class/gpio/gpio%d/direction",pin);
         fd = open(buf,O_RDWR);
         if(fd < 0)
         {
                 printf("set gpio %d direction error [%d] %s!\n",pin,__LINE__,strerror(errno));
                 return -1;
         }
         CLEAR(buf);
         read(fd, buf, 5);
         if(!strstr(buf,"out"))
         {
                 write(fd,"out",sizeof("out"));
         }
         close(fd);
        return 0;

}
#if 0
int QCamLedSet(QCAM_LED_MODE mode, int blink)
{
        if(mode == LED_MODE_FLIGHT)
        {
            int mode = blink;
            ysx_gpio_open(LED_FLIGHT);
            ysx_gpio_write(LED_FLIGHT,mode);
            
            return 0;



        }
        int ret;
        int duty;

        ret = ysx_pwm_set(2,0,1000);
        if( ret < 0){
                printf("open failed gpio%d!\n",LED_BLUE);
                return -1;
        }
        ret = ysx_pwm_set(3,0,1000);
        if( ret < 0){
                printf("open failed gpio%d!\n",LED_GREEN);
                return -1;
        }

        #if 0
       //ysx_gpio_open(LED_RED);	
        ysx_gpio_open(LED_BLUE);
        ysx_gpio_open(LED_GREEN); 
        //AMCSystemCmd("echo out > /sys/class/gpio/gpio72/direction");
        set_direction(72);
        set_direction(73);
        //AMCSystemCmd("echo out > /sys/class/gpio/gpio73/direction");  		
        //ysx_gpio_write(LED_GREEN,LED_OFF);
        ysx_gpio_write(LED_GREEN,LED_OFF);
        ysx_gpio_write(LED_BLUE,LED_OFF);
        //ysx_gpio_write(LED_RED,0);	
        #endif
        
        if(mode == LED_MODE_OFF)
                return 0;
        #if 0
        if(blink == 0)
        {
                if(mode == LED_MODE_GREEN)
                        ysx_gpio_write(LED_GREEN,LED_ON);
                        //ysx_gpio_write(LED_GREEN,0);
                if(mode == LED_MODE_BLUE)
                        ysx_gpio_write(LED_BLUE,LED_ON);        
                        //ysx_gpio_write(LED_GREEN,1);
                if(mode == LED_MODE_RED)
                        ysx_gpio_write(LED_RED,LED_ON);        
        }else
        {
                duty = 500;
                if(mode == LED_MODE_GREEN)
                {  
                        ysx_pwm_set(2,duty,1000);printf("green blink\n");
                }
                else if(mode ==  LED_MODE_BLUE)
                {
                        ysx_pwm_set(3,duty,1000);printf("blue blink\n");
                }
                else if(mode ==  LED_MODE_RED)
                {      
                        printf("not support red blink\n");
                }

        }
        #endif
        duty = 500;        
        if(blink == 0)
           duty = duty*2;

                if(mode == LED_MODE_GREEN)
                {  
                        ysx_pwm_set(2,duty,1000);printf("green blink\n");
                }
                else if(mode ==  LED_MODE_BLUE)
                {
                        ysx_pwm_set(3,duty,1000);printf("blue blink\n");
                }
                else if(mode ==  LED_MODE_RED)
                {      
                        printf("not support red blink\n");
                }

        return 0;
}
#else
int QCamLedSet(QCAM_LED_MODE mode, int blink)
{
        int fd = 0;
        int ret = 0;
        int type = 0;
        int cmd = 0;
        fd = open("/dev/led", O_WRONLY);
        if (fd < 0) {
           perror("open error");
           return -1;
        }
        switch(mode){
            case LED_MODE_OFF: 
                type = 0;
                cmd =  1;//guanbi
                ret = ioctl(fd, cmd, (void*)(&type));
                if (ret < 0) {
                    perror("ioctl error");
                    break;
                }
                type = 1;
                cmd =  1;
                ret = ioctl(fd, cmd, (void*)(&type));
                if (ret < 0) {
                    perror("ioctl error");
                    break;
                }
            break;
            case LED_MODE_GREEN: 
                type = 1;
                if(blink == 0)
                    cmd =  0;//guanbi
                else if(blink == 1)
                    cmd =  2;//shansuo
                ret = ioctl(fd, cmd, (void*)(&type));
                if (ret < 0) {
                    perror("ioctl error");
                    break;
                }
            break;
            case LED_MODE_BLUE: 
                type = 0;
                if(blink == 0)
                    cmd =  0;//guanbi
                else if(blink == 1)
                    cmd =  2;//shansuo
                ret = ioctl(fd, cmd, (void*)(&type));
                if (ret < 0) {
                    perror("ioctl error");
                    break;
                }
            break;
            default:
                printf("unsupported led mode\n");
            break;
        } 
        close(fd);
        return 0;
        
}


#endif






int QCamGetKeyStatus()
{
        int ret,val;

        ret = ysx_gpio_open(RESET_KEY);
        if(ret < 0)
        {   
                printf("open gpio %d failed!\n",RESET_KEY);
                return QCAM_KEY_INVALID;
        }

        ret = ysx_gpio_read(RESET_KEY,&val);
        if(ret < 0)
        {   
                printf("read gpio %d failed!\n",RESET_KEY);
                return QCAM_KEY_INVALID;
        }   

        return (!val);
}

#ifdef KEY_EVENT
static void* key_listener(void *p)
{
        SUKeyEvent event;
        int ret, key_code;

        printf("Start read key event.\n");

        while (1) {
                ret = SU_Key_ReadEvent(evfd, &key_code, &event);
                if (ret != 0) {
                        printf("Get Key event error\n");
                        return NULL;
                }

                key_event_cb(event);
                printf("%d %d\n",key_code,event);
        }

        return NULL;

}


void QCamRegKeyListener(QCam_Key_Status_cb cb)
{

        int ret;
        pthread_t key_tid;

        key_event_cb = cb;
        ysx_gpio_close(52);
        evfd = SU_Key_OpenEvent();
        if (evfd < 0) {
                printf("Key event open error\n");
                return ;
        }

        ret = pthread_create(&key_tid, NULL, key_listener, NULL);
        if (ret < 0) {
                perror("key_event_listener thread create error\n");
                return ;
        }

        pthread_detach(key_tid);    //·?à?
}
int QCamGetFunctionKeyStatus(){
          SUKeyEvent event;
          int ret, key_code;
          int evfd ;
      
          evfd = SU_Key_OpenEvent();
          ///printf("%d\n",evfd);
          if (evfd < 0) {
              printf("Key event open error\n");
              return -1;
          }
     //    printf("while main\n");
               ret = SU_Key_ReadEvent(evfd, &key_code, &event);
           if (ret != 0) {
                printf("Get Key event error\n");
                return -1;
             }
      
             printf("%d %d\n",key_code,event);
            SU_Key_CloseEvent(evfd);
      
            return !event;    

}

#endif

int check_cmd(const char *cmd)
{
        char buf1[128],buf2[256],tmp[256];
        int ret = -1;
        char file[128];

        CLEAR(buf1);
        CLEAR(buf2);
        strcpy(buf2,getenv("PATH"));
        do{
                CLEAR(tmp); 
                sscanf(buf2,"%[^:]:%s",buf1,tmp);
                CLEAR(buf2);       
                strcpy(buf2,tmp);
                sprintf(file,"%s/%s",buf1,cmd);
                if(access(file,F_OK) == 0)
                {
                        ret = 0;
                        break;
                }
        }while(strlen(buf2));

        return ret;
}


static int check_header_valid(const char *file)
{
        FILE *fp ;
        image_header_t hdr;
        unsigned long checksum;
        int len = 0;
        int retval = 0;
        unsigned long crc_dat = 0;

        fp = fopen(file,"rb");
        if(NULL == fp)
        {
                printf("open file %s error !\n",file);
                return -1;
        }
        memset(&hdr,0,sizeof(hdr));
        fread(&hdr,1,sizeof(hdr),fp);

        /* check the easy ones first */
#if 0
#define CHECK_VALID_DEBUG
#else
#undef CHECK_VALID_DEBUG
#endif

#ifdef CHECK_VALID_DEBUG
        printf("\nmagic %x %x\n", ntohl(hdr.ih_magic), IH_MAGIC);
        printf("arch %#x %x\n", hdr.ih_arch, IH_ARCH_MIPS);
        printf("size %x %x\n", ntohl(hdr.ih_size), 64);
        printf("type %x %x\n", hdr.ih_type, IH_TYPE_FIRMWARE);
#endif

        if (ntohl(hdr.ih_magic) != IH_MAGIC || hdr.ih_arch != IH_ARCH_MIPS) {
                printf("Image bad MAGIC or ARCH\n");
                return -1;
        }

        /* check the type - could do this all in one gigantic if() */
        if (hdr.ih_type != IH_TYPE_FIRMWARE) {
                printf("Image wrong type\n");
                return -1;
        }

        /* check the hdr CRC */
        checksum = ntohl(hdr.ih_hcrc);
        hdr.ih_hcrc = 0;

        if ((crc_dat = crc32(0, (unsigned char const *)&hdr, 64)) != checksum) {
                printf("Image bad header checksum %x != %x \n",checksum,crc_dat);
                //		return -1;
        }
		printf("=====check_header_valid===========crc_dat = %x,checksum = %x\n",crc_dat, checksum);

        /* check the data CRC */
        checksum = ntohl(hdr.ih_dcrc);
        fseek(fp,0,SEEK_END);
        len = ftell(fp)-sizeof(image_header_t);		//ignore image header 64 bytes
        unsigned char *data = (unsigned char *)malloc(len);
        if(NULL == data)
        {
                printf("malloc buffer %d failed !\n",len);
                return -1;
        }
        fseek(fp,sizeof(image_header_t),SEEK_SET);
        retval = fread(data,1,len,fp);
        if(retval != len )	
        {
                printf("read data len = %d is error , it should be %d!\n",retval,len);
                return -1;
        }
        if ((crc_dat = crc32(0, (unsigned char const *)data, retval)) != checksum) {
                printf("Image bad data checksum %x != %x \n",checksum,crc_dat);
                return -1;
        }

        free(data);
        fclose(fp);

        return 0;
}

static unsigned long check_header_valid_forOTA(const char *file)
{
        FILE *fp ;
        image_header_t hdr;
        unsigned long checksum;
        int len = 0;
        int retval = 0;
        unsigned long crc_dat = 0;

        fp = fopen(file,"rb");
        if(NULL == fp)
        {
                printf("open file %s error !\n",file);
                return 0;
        }
        memset(&hdr,0,sizeof(hdr));
        fread(&hdr,1,sizeof(hdr),fp);

        /* check the easy ones first */
#if 0
#define CHECK_VALID_DEBUG
#else
#undef CHECK_VALID_DEBUG
#endif

#ifdef CHECK_VALID_DEBUG
        printf("\nmagic %x %x\n", ntohl(hdr.ih_magic), IH_MAGIC);
        printf("arch %#x %x\n", hdr.ih_arch, IH_ARCH_MIPS);
        printf("size %x %x\n", ntohl(hdr.ih_size), 64);
        printf("type %x %x\n", hdr.ih_type, IH_TYPE_FIRMWARE);
#endif

        if (ntohl(hdr.ih_magic) != IH_MAGIC || hdr.ih_arch != IH_ARCH_MIPS) {
                printf("Image bad MAGIC or ARCH\n");
                return 0;
        }

        /* check the type - could do this all in one gigantic if() */
        if (hdr.ih_type != IH_TYPE_FIRMWARE) {
                printf("Image wrong type\n");
                return 0;
        }

        /* check the hdr CRC */
        checksum = ntohl(hdr.ih_hcrc);
        hdr.ih_hcrc = 0;

        if ((crc_dat = crc32(0, (unsigned char const *)&hdr, 64)) != checksum) {
                printf("Image bad header checksum %x != %x \n",checksum,crc_dat);
                //		return -1;
        }
		printf("=====check_header_valid===========crc_dat = %x,checksum = %x\n",crc_dat, checksum);

        /* check the data CRC */
        checksum = ntohl(hdr.ih_dcrc);

		fclose(fp);
		return checksum;

}


#define MAX_PARTS 7
typedef struct 
{
        uint32_t id;
        uint32_t len;
        uint32_t flash_off;
        uint32_t reserve1;
}zone_info;

struct flash_part{
        const char *file;
        const char *device;
        uint32_t size;
};

static zone_info part_info[MAX_PARTS] = {0};
static int part_num=0;
static int f_flag = 0;
static int update_num = 0;

int get_zone_info(const char *file_path)
{
        zone_info info;
        FILE *fp ;
        int i;
        fp = fopen(file_path,"rb");
        if(NULL == fp)
        {
                printf("open file %s error !\n",file_path);
                return -1;
        }
        fseek(fp,64,SEEK_SET);
        fread(&part_num,4,1,fp);
        if(part_num == 3 )
                f_flag = 0;
        else if(part_num == 8)
                f_flag = 1;
        else{
                printf("part_num[%d] must be 3 or 8!\n",part_num);
                return -1;
        }		

        for(i=0;i<part_num;i++)
        {
            fread(&info,sizeof(zone_info),1,fp);
        	printf("id=%d,len=%d,flash_off=0x%x,rev1=%d\n",info.id,info.len,info.flash_off,info.reserve1);
            memcpy(&part_info[i],&info,sizeof(zone_info));
        }

        return 0;
}

#define FLAG "WORK"

int wr_flag(char *file_path , off_t offset)
{
        int fd;
        fd = open(file_path, O_RDWR);
        if (fd < 0) {
                perror("Input file open error");
                _exit(1);
        }

        off_t cur_off;
        cur_off = lseek(fd, offset - 4, SEEK_SET);
        if (cur_off != (offset - 4)) {
                printf("seek to %08x error cur_off=0x%08x\n", (uint32_t)offset, (uint32_t)cur_off);
                _exit(1);
        }

        ssize_t ret = write(fd, FLAG, 4);
        if (ret != 4) {
                printf("write flag error\n");
                _exit(1);
        }
        /*
        cur_off = lseek(fd, offset - 8, SEEK_SET);
        if (cur_off != (offset - 8)) {
                printf("seek to %08x error cur_off=0x%08x\n", (uint32_t)offset, (uint32_t)cur_off);
                _exit(1);
        }
        char buf[4] ;
        memset(buf,0,sizeof(buf));
        ret = read(fd,0,buf);
        int vers = 0;
        vers |= buf[3]<<24;
        vers |= buf[2]<<16;
        vers |= buf[1]<<8;
        vers |= buf[0]<<0;
        printf("ver %d\n",vers);

        ret = write(fd, FLAG, 4);
        if (ret != 4) {
                printf("write flag error\n");
                _exit(1);
        */
        close(fd);
}

int wr_rescure_flag(char *file_path , off_t offset,char*flags)
{
        int fd;
        fd = open(file_path, O_RDWR);
        if (fd < 0) {
                perror("Input file open error");
                _exit(1);
        }

        off_t cur_off;
        cur_off = lseek(fd, offset - 4, SEEK_SET);
        if (cur_off != (offset - 4)) {
                printf("seek to %08x error cur_off=0x%08x\n", (uint32_t)offset, (uint32_t)cur_off);
                _exit(1);
        }

        ssize_t ret = write(fd, flags, 4);
        if (ret != 4) {
                printf("write flag error\n");
                _exit(1);
        }
        close(fd);
}
void copy_env_to_ram()
{
        AMCSystemCmd("mkdir -p /tmp/lib");
        AMCSystemCmd("mkdir -p /tmp/bin");

        AMCSystemCmd("cp -a /bin/busybox /tmp/bin/");
        AMCSystemCmd("cp -a /usr/sbin/flashcp /tmp/bin/");

        AMCSystemCmd("cp -a /lib/libuClibc-0.9.33.2.so /tmp/lib/");
        AMCSystemCmd("cp -a /lib/libc.so.0 /tmp/lib/"); 
        AMCSystemCmd("cp -a /lib/libm-0.9.33.2.so /tmp/lib/");
        AMCSystemCmd("cp -a /lib/libm.so.0 /tmp/lib/");
        AMCSystemCmd("cp -a /lib/libpthread.so.0 /tmp/lib/");
        AMCSystemCmd("cp -a /lib/libpthread-0.9.33.2.so /tmp/lib/");
        AMCSystemCmd("cp -a /lib/ld-uClibc-0.9.33.2.so /tmp/lib/");
        AMCSystemCmd("cp -a /lib/ld-uClibc.so.0 /tmp/lib/");
        AMCSystemCmd("cp -a /lib/libdl.so.0 /tmp/lib/");
        AMCSystemCmd("cp -a /lib/libdl-0.9.33.2.so /tmp/lib/");	
        //	AMCSystemCmd("cp -a /usr/lib/libz.so* /tmp/lib/");

        setenv("LD_LIBRARY_PATH","/tmp/lib",1);
        sync();

//        return 0;
}

int QCamFlashBurn_(const char *firmwarePath)
{
        char sys_cmd[256];
        uint32_t count=0,offset=1,block = 64, block_1k = 1024/block;
        struct stat st;
        int workflag=1;

        if(stat(firmwarePath,&st) == -1)
        {
                fprintf(stderr,"file %s not exit !\n",firmwarePath);
                return -1;
        }
        //	printf("file size = %ld\n",st.st_size/1024);
        if(check_header_valid(firmwarePath) != 0)
        {
                fprintf(stderr,"check_header_valid error!\n");
                return -1;
        }

        if(get_zone_info(firmwarePath) != 0){
                fprintf(stderr,"get_zone_info error!\n");
                return -1;
        }


        if(check_cmd("flash_erase") != 0)
        {
                fprintf(stderr,"cmd flash_erase not find !\n");
                return -1;
        }
        if(check_cmd("flashcp") != 0)
        {
                fprintf(stderr,"cmd flashcp not find !\n");
                return -1;
        }   

        struct flash_part pi[MAX_PARTS] = {
                #if 0
                {FILE_UBOOT, "/dev/mtd0", 256},	
                {FILE_ROOT,  "/dev/mtd1",1024},	
                {FILE_KERNEL,"/dev/mtd2",1792}, 
                {FILE_USER,  "/dev/mtd3",5120},	
                {FILE_KERNEL,"/dev/mtd4",1792},	
                {FILE_USER,  "/dev/mtd5",5120},	
                {FILE_MTD,   "/dev/mtd6",1024}
                #endif
            
                {FILE_UBOOT, "/dev/mtd0", 256},	
                {FILE_KERNEL,"/dev/mtd1",1792}, 
                {FILE_ROOT,  "/dev/mtd2",1792},	
                {FILE_USER,  "/dev/mtd3",4992},	
                {FILE_ROOT,  "/dev/mtd4",1792},	
                {FILE_USER,  "/dev/mtd5",4992},	
                {FILE_MTD,   "/dev/mtd6", 512}
        };

        copy_env_to_ram();

        /*step 1 . erase work flag */
        //AMCSystemCmd("flash_erase /dev/mtd1 0x1B8000 1");	    // erase 'WORK' flag
        //AMCSystemCmd("flash_erase /dev/mtd2 0x1B8000 1");     // erase 'WORK' flag
        //AMCSystemCmd("flash_erase /dev/mtd3 0x4F8000 1");     // erase 'WORK' flag
         
        system("flash_erase /dev/mtd2 0x1B8000 1");     // erase 'WORK' flag
        system("flash_erase /dev/mtd3 0x4D8000 1"); 
        printf("new backup root user for fwupdate\n");
        int i,id;
        if(f_flag == 1)
            id = 0;
        else
        {
            id = 2;
                /*		
                        FILE *flag_fp = fopen("/var/flag.bin","rb");
                        if(flag_fp)
                        {
                        char bootflag[7] = {0};
                        fread(bootflag,1,sizeof(bootflag),flag_fp);
                        fclose(flag_fp);

                // update main 1 
                if(strncmp(bootflag,"WORK_2",6) == 0)
                {
                id = 2;
                workflag = 2;
                printf("Update factory kernel & user\n");
                }
                else	// update main 2
                {
                workflag = 1;
                printf("Update factory kernel2 & user2\n");
                }
                }
                */
        }	
        offset  = 1+1024/64;		// header 64bytes  + part_info 1024 bytes

        for(i=0;i<part_num;i++,id++){

                /*step 2 . extract fw into kernel rootfs user*/
                CLEAR(sys_cmd);
                snprintf(sys_cmd,256,"LD_LIBRARY_PATH=\"/tmp/lib\" /tmp/bin/busybox dd if=%s of=%s bs=%d count=%d skip=%d",firmwarePath,pi[id].file,block,part_info[i].len*block_1k,offset);
                AMCSystemCmd(sys_cmd);

                snprintf(sys_cmd,256,"LD_LIBRARY_PATH=\"/tmp/lib\" /tmp/bin/busybox dd if=/dev/zero of=%s bs=1k seek=%d count=%d",pi[id].file,part_info[i].len,pi[id].size-part_info[i].len);
                //		printf(":%s\n",sys_cmd);		
                AMCSystemCmd(sys_cmd);

                wr_flag(pi[id].file,pi[id].size*1024);
                /*step 3 . update flash */		
                snprintf(sys_cmd,256,"LD_LIBRARY_PATH=\"/tmp/lib\" /tmp/bin/flashcp %s %s",pi[id].file,pi[id].device);  // update uboot 
                AMCSystemCmd(sys_cmd);
                //		printf(":%s:\n",sys_cmd);
                remove(pi[id].file);
                offset += part_info[i].len*block_1k;

        }
    

        return 0;
}
#define BLOCK_NAME_LEN 9
int which_zone_to_write(){
  FILE *fp = fopen("/proc/cmdline", "rb");
  if(fp == NULL)
  { 
        printf("open cmdline error\n");

        return -1;
  }
     char buf[256];
     memset(buf,0,sizeof(buf));
     char block_name[16];
     memset(block_name,0,sizeof(block_name));

     fread(buf,1,sizeof(buf),fp);
     fclose(fp);
     printf("%s\n",buf);
     char* p = strrchr(buf, '/');
     if(p == NULL){
        printf("parse cmdline failed\n");
        return -1;
     }
     strncpy(block_name,p+1,BLOCK_NAME_LEN);
     printf("%s\n",block_name);
     if(strcmp(block_name,"mtdblock2") == 0){
        printf("now block main  to update rescure\n");
        return 2;
     }else{
        printf("now block rescure  to update main\n");
        return 1;
    }
}
/*
int compare_end_ver(char*main_path,char*rescure_path){
        int fd;
        fd = open(main_path, O_RDWR);
        if (fd < 0) {
                perror("Input file open error");
                _exit(1);
        }

        off_t cur_off;
        cur_off = lseek(fd, offset - 8, SEEK_SET);
        if (cur_off != (offset - 4)) {
                printf("seek to %08x error cur_off=0x%08x\n", (uint32_t)offset, (uint32_t)cur_off);
                _exit(1);
        }

        char buf[4] ;
        memset(buf,0,sizeof(buf));
        ret = read(fd,0,buf);
        close(fd);
        int main_ver = 0;
        main_ver |= buf[3]<<24;
        main_ver |= buf[2]<<16;
        main_ver |= buf[1]<<8;
        main_ver |= buf[0]<<0;
        printf("ver %d\n",vers);


        fd = open(rescure_path, O_RDWR);
        if (fd < 0) {
                perror("Input file open error");
                _exit(1);
        }

        off_t cur_off;
        cur_off = lseek(fd, offset - 8, SEEK_SET);
        if (cur_off != (offset - 4)) {
                printf("seek to %08x error cur_off=0x%08x\n", (uint32_t)offset, (uint32_t)cur_off);
                _exit(1);
        }

        memset(buf,0,sizeof(buf));
        ret = read(fd,0,buf);
        close(fd);
        int rescure_ver = 0;
        rescure_ver |= buf[3]<<24;
        rescure_ver |= buf[2]<<16;
        rescure_ver |= buf[1]<<8;
        rescure_ver |= buf[0]<<0;
        printf("ver %d\n",vers);

}
*/
int QCamFlashBurn(const char *firmwarePath)
{
        char sys_cmd[256];
        uint32_t count=0,offset=1,block = 64, block_1k = 1024/block;
        struct stat st;
        int workflag=1;

        if(stat(firmwarePath,&st) == -1)
        {
                fprintf(stderr,"file %s not exit !\n",firmwarePath);
                return -1;
        }
        //	printf("file size = %ld\n",st.st_size/1024);
        if(check_header_valid(firmwarePath) != 0)
        {
                fprintf(stderr,"check_header_valid error!\n");
                return -1;
        }

        if(get_zone_info(firmwarePath) != 0){
                fprintf(stderr,"get_zone_info error!\n");
                return -1;
        }


        if(check_cmd("flash_erase") != 0)
        {
                fprintf(stderr,"cmd flash_erase not find !\n");
                return -1;
        }
        if(check_cmd("flashcp") != 0)
        {
                fprintf(stderr,"cmd flashcp not find !\n");
                return -1;
        }   
        int update_num = which_zone_to_write();
        if(update_num < 0)
        {
                fprintf(stderr,"cmdline read error !\n");
                return -1;
        }   


        struct flash_part pi[MAX_PARTS] = {
            
                {FILE_UBOOT, "/dev/mtd0", 256},	
                {FILE_KERNEL,"/dev/mtd1",1792}, 
                {FILE_ROOT,  "/dev/mtd2",1792},	
                {FILE_USER,  "/dev/mtd3",4288},	
                {FILE_KERNEL,"/dev/mtd4",1792}, 
                {FILE_ROOT,  "/dev/mtd5",1792},	
                {FILE_USER,  "/dev/mtd6",4288},	
                {FILE_MTD,   "/dev/mtd7", 256}
        };

        copy_env_to_ram();

        /*step 1 . erase work flag */
        //AMCSystemCmd("flash_erase /dev/mtd1 0x1B8000 1");	    // erase 'WORK' flag
        //AMCSystemCmd("flash_erase /dev/mtd2 0x1B8000 1");     // erase 'WORK' flag
        //AMCSystemCmd("flash_erase /dev/mtd3 0x4F8000 1");     // erase 'WORK' flag
        if(update_num == 1){
            system("flash_erase /dev/mtd1 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd2 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd3 0x428000 1"); 
            printf("erase main zone work flag\n");
        }else{
            system("flash_erase /dev/mtd4 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd5 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd6 0x428000 1"); 
            printf("erase rescure zone work flag\n");
        
        }
        int i,id;
        if(f_flag == 1)
            id = 0;
        else if(update_num == 1)
        {
            id = 1;
        }else
            id = 4;
        offset  = 1+1024/64;		// header 64bytes  + part_info 1024 bytes

        printf("update %s\n",(id == 4 ? "rescure":"main"));
        for(i=0;i<part_num ;i++,id++){

                /*step 2 . extract fw into kernel rootfs user*/
                printf("%d %d\n",i,id);
                CLEAR(sys_cmd);
                snprintf(sys_cmd,256,"LD_LIBRARY_PATH=\"/tmp/lib\" /tmp/bin/busybox dd if=%s of=%s bs=%d count=%d skip=%d",firmwarePath,pi[id].file,block,part_info[i].len*block_1k,offset);
                AMCSystemCmd(sys_cmd);

                snprintf(sys_cmd,256,"LD_LIBRARY_PATH=\"/tmp/lib\" /tmp/bin/busybox dd if=/dev/zero of=%s bs=1k seek=%d count=%d",pi[id].file,part_info[i].len,pi[id].size-part_info[i].len);
                //		printf(":%s\n",sys_cmd);		
                AMCSystemCmd(sys_cmd);

                wr_flag(pi[id].file,pi[id].size*1024);
                /*step 3 . update flash */		
                snprintf(sys_cmd,256,"LD_LIBRARY_PATH=\"/tmp/lib\" /tmp/bin/flashcp %s %s",pi[id].file,pi[id].device);  // update uboot 
                AMCSystemCmd(sys_cmd);
                //		printf(":%s:\n",sys_cmd);
                remove(pi[id].file);
                offset += part_info[i].len*block_1k;

        }
        system("flash_erase /dev/mtd8 0x18000 1");
        if(update_num == 2){
            wr_rescure_flag("/dev/mtd8",128*1024,"BACK_UP");      
        }else
            wr_rescure_flag("/dev/mtd8",128*1024,"MAIN_UP");
        

        return 0;
}


int FwHeadConfig(const char *firmwarePath,zone_info* config_info, int* part_numb, unsigned int* checksum)
{
		char sys_cmd[256];
        struct stat st;
		int i;
		unsigned int crc_data = 0;
		
        if(stat(firmwarePath,&st) == -1)
        {
                fprintf(stderr,"file %s not exit !\n",firmwarePath);
                return -1;
        }
        printf("file size = %ld\n",st.st_size);
		crc_data = check_header_valid_forOTA(firmwarePath);
        if(crc_data == 0)
        {
                fprintf(stderr,"check_header_valid error!\n");
                return -1;
        }

        if(get_zone_info(firmwarePath) != 0){
                fprintf(stderr,"get_zone_info error!\n");
                return -1;
        }

        if(check_cmd("flash_erase") != 0)
        {
                fprintf(stderr,"cmd flash_erase not find !\n");
                return -1;
        }
        if(check_cmd("flashcp") != 0)
        {
                fprintf(stderr,"cmd flashcp not find !\n");
                return -1;
        }   
        update_num = which_zone_to_write();
        if(update_num < 0)
        {
                fprintf(stderr,"cmdline read error !\n");
                return -1;
        }   

        copy_env_to_ram();

        /*step 1 . erase work flag */

        if(update_num == 1){
            system("flash_erase /dev/mtd1 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd2 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd3 0x428000 1"); 
            printf("erase main zone work flag\n");
        }else{
            system("flash_erase /dev/mtd4 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd5 0x1B8000 1");     // erase 'WORK' flag
            system("flash_erase /dev/mtd6 0x428000 1"); 
            printf("erase rescure zone work flag\n");
        
        }
		for(i=0;i<part_num;i++)
        {
                memcpy(&config_info[i],&part_info[i],sizeof(zone_info));				
				printf("+++++FwHeadConfig : id=%d,len=%d,flash_off=0x%x,rev1=%d\n",config_info[i].id,config_info[i].len,config_info[i].flash_off,config_info[i].reserve1);
        }
		*part_numb = part_num;
		*checksum = crc_data;
		return update_num;
}

