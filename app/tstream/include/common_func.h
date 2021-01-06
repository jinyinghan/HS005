#ifndef COMMON_FUNC
#define COMMON_FUNC

typedef struct
{
     unsigned short year;    // The number of year.
     signed char month;  // The number of months since January, in the range 1 to 12.
     signed char day;        // The day of the month, in the range 1 to 31.
     signed char wday;       // The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
     signed char hour;     // The number of hours past midnight, in the range 0 to 23.
     signed char minute;   // The number of minutes after the hour, in the range 0 to 59.
     signed char second;   // The number of seconds after the minute, in the range 0 to 59.
}STimeDay;

#define CLEAR(x) memset(x,0,sizeof(x))


int check_proc(const char *procname);

///conf.c
int InitCameraCommonConfig();

int GetShareePass(char *passwd);
int SetSharePass(char *pass);
int SetPass(char *pass);
void set_ir_mode(int mode);
int InitDeviceSetting();
int InitDeviceCommonConfig();
int GetYSXCtl(int iType);
void SetYSXCtl(int iType, int iValue);
void SetMotionTime(unsigned int tStart, unsigned int tEnd);
void GetMotionTime(unsigned int *tStart, unsigned int *tEnd);
#ifdef IVS_SENSE
void set_sensor_sense(int sense);
#endif
//conf.c
/**
 * @fn int Set_Point_to_conf(int iType, int iindex, int iValue)
 *
 * 完成对文件的定点收藏添加与删除
 *
 * @param[in] iType 定点类型
 * @param[in] iindex 定点序号
 * @param[in] iValue 定点坐标
 *
 * @retval 0 成功
 * @retval -1 失败
 *
 * @remarks 调用此 API 时 iniparser_load 等文件操作函数被调用。
 *
 * @attention 无。
 */
int Set_Point_to_conf(int iType, int iindex, int iValue);
/**
 * @fn int Get_Point_from_conf(int iType, int iindex)
 *
 * 完成对文件的定点收藏添加与删除
 *
 * @param[in] iType 定点类型
 * @param[in] iindex 定点序号
 *
 * @retval 0  位置记录坐标默认值
 * @retval -1 位置收藏坐标默认值
 * @retval -2 失败
 *
 * @remarks 调用此 API 时 iniparser_load 等文件操作函数被调用。
 *
 * @attention 返回值为文件读取数值，调用函数不同，默认返回值不同。
 * @attention 收藏的 key--[ysx]-- 必须存在。
 */
int Get_Point_from_conf(int iType, int iindex);
/**
 * @fn int Get_cfg_from_conf(int iType, int iindex)
 *
 * 完成对移动配置信息的获取
 *
 * @param[in] iType 定点类型
 * @param[in] iindex 定点序号
 *
 * @retval 0  配置使能关闭
 * @retval 1  配置使能打开
 * @retval -1 失败
 *
 * @remarks 调用此 API 时 iniparser_load 等文件操作函数被调用。
 *
 * @attention 无
 */
int Get_cfg_from_conf(int iType);
/**
 * @fn int InitDevDtc(int c_en)
 *
 * 完成对移动侦测配置初始化
 *
 * @param[in] c_en 开关配置
 *
 * @retval 0 成功
 * @retval -1 失败
 *
 * @remarks 无。
 *
 * @attention 无
 */
int InitDevDtc(int c_en);
/**
 * @fn int InitDevIR(int c_en)
 *
 * 完成对红外灯配置初始化
 *
 * @param[in] c_en 开关配置
 *
 * @retval 0 成功
 * @retval -1 失败
 *
 * @remarks 无。
 *
 * @attention 无
 */
int InitDevIR(int c_en);

//main.c
char find_pos(char ch);
void start_post_mdrecord();

///guesthandle.c
void Handle_IOCTRL_Cmd(int SID, int avIndex, char *buf, int len,int iMediaNo);
void Gen_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo);
void Play_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo);
void Camera_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo);
void UTCToLocal(STimeDay *sT);
void LocalToUTC(STimeDay *sT);
void wpa_handle(unsigned char *name, unsigned char *pass,unsigned char key);
unsigned char WifiChange(const char *ssid,char *wifi_sw);
int change_wifi(const char * ssid, const char * pwd);
//void ysx_bt_init(void*arg);
//int get_sd_path(char *path, int path_len);
int get_sd_strorage(char *mount_point,uint64_t* totalsize,uint64_t* freesize);




//media.c
int init_media();
void Media_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo);
void Media_Close();
void start_md_record();
void Md2Snapshot(char *file_name);

//utils.c

int get_fill_light_status();

int set_fill_light_status(int status);

int LedEnable(int flag);
int SetLed(int onoff,int status);
int GetLedStatus();

FILE* ysx_popen(const char *pCommand, const char *pMode, pid_t *pid);
int  ysx_pclose(FILE *fp, pid_t pid);
void ysx_setTimer(int seconds, int mseconds);

//ez-setup.c
int WaitEzSetupOK();
int GetEthStatus();
int check_proc(const char *procname);
unsigned char GetDeviceMode();
int wired_mode_bind_initialization(void *arg);
void wird_mode_bind_uninitialization(void);
int ap_mode_bind_initialization(void *arg);
void ap_mode_bind_uninitialization(void);

int ap_mode_band_initialization(void*arg);
void ap_mode_band_uninitialization(void);


#ifdef CONFIG_TRACK_DRAWRECT
//motion_track_drawrect.c
void *motion_drawRect_thread(void *arg);
#endif
void * perdetect_thread(void *arg);

/*aes.c declare*/
char *ysx_base64_encode(const char* data, int data_len,int *len);
char *ysx_base64_decode(const char *data, int data_len,int *len);
void ysx_aes_cbc_encrypt(unsigned char* in, int inl, unsigned char *out, int* len, char * key);
void ysx_aes_cbc_decrypt(unsigned char* in, int inl, unsigned char *out, unsigned char *key);

#define     ERR_DEBUG_SWITCH        0

#endif
