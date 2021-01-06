#ifndef _YSX_SYS_H_
#define _YSX_SYS_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include <sysutils/su_misc.h>
#define CLEAR(x) memset(x,0,sizeof(x))

#define LED_RED     82
#define LED_GREEN   72  
#define LED_BLUE    73
#define LED_FLIGHT  90

#define LED_OFF     0
#define LED_ON      1   
#define RESET_KEY  7    

#define BUF_SIZE 128


#define PWM_DEVICE "/dev/pwm"
#define PWM_CONFIG	0x001
#define PWM_ENABLE	0x010
#define PWM_DISABLE 0x100
#define ONE_MSEC    1000000


#define FAIL    (-1)
#define SUCCESS (0)

struct pwm_ioctl_t {
	int index;
	int duty;
	int period;
	int polarity;
};


enum {
    KERNEL = 0,
    ROOT   = 1,
    USER   = 2,
};

#define FILE_FW 	"/tmp/fw.bin"
#define FILE_UBOOT  "/tmp/uboot.bin"
#define FILE_KERNEL "/tmp/kernel.bin"
#define FILE_ROOT   "/tmp/root.bin"
#define FILE_USER   "/tmp/user.bin"
#define FILE_MTD   "/tmp/mtd.bin"

#define CAMERA_CONF "/system_rw/device_config"
#define SYMBOL_LIGHTNESS_TOP1 "lightness_top1="
#define SYMBOL_LIGHTNESS_BUTTON1 "lightness_button1="
#define SYMBOL_LIGHTNESS_TOP2 "lightness_top2="
#define SYMBOL_LIGHTNESS_BUTTON2 "lightness_button2="
#define SYMBOL_MAXGOP "maxGop="
#define SYMBOL_MAXQP "maxQp="
#define SYMBOL_MINQP "minQp="
#define SYMBOL_FRMRATENUM "frmRateNum="
#define SYMBOL_MAXBITRATE "maxBitRate="

#define SYMBOL_JPEG_PROFILE "jpeg_profile="
#define SYMBOL_EV_LOG "ev_log="
#define SYMBOL_EV1_VAL "ev1="
#define SYMBOL_EV2_VAL "ev2="
#define SYMBOL_MIC_GAIN "mic_gain="
#define SYMBOL_SPK_GAIN "spk_gain="
#define SYMBOL_AW8733_MODE "aw8733_mode="

/*open gpio before use it*/
int ysx_gpio_open(int pin);

/*use gpio as input , read the current value*/
int ysx_gpio_read(int pin, int *val);

/*use gpio as output , set the output value*/
int ysx_gpio_write(int pin , int value);

/*close gpio if gpio is not be used anymore*/
int ysx_gpio_close(int pin);

//void QCamRegKeyListener(QCam_Key_Status_cb cb);


unsigned char * base64_encode(const unsigned char *src, size_t len,
			      size_t *out_len);
unsigned char * base64_decode(const unsigned char *src, size_t len,
			      size_t *out_len);
#endif  /*_YSX_SYS_H_*/

