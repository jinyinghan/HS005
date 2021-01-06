#ifndef _PTZ_H_
#define _PTZ_H_
#include <stdbool.h>

//#include "d706_ptz_ctl.h"
//#define Y_DEBUG
//#define APP_API_DBG
#define STEP_RETURN_NOT_DOUBLE			1
#if defined (T10L)
#define MAX_STEPS_X 1024*2
#define MAX_STEPS_Y 768*2
#endif
#if defined (S201A)
#define MAX_STEPS_X 					1024
#define MAX_STEPS_Y 					400//300//660//768
#define ONE_TIME_STEPS 					100
#elif defined (S301)
#define MAX_STEPS_X 					1936
#define MAX_STEPS_Y 					350//300//660//768
#define ONE_TIME_STEPS 					100

#elif defined (S201)
#define MAX_STEPS_X 					992//500//752//1024
#define MAX_STEPS_Y 					660//300//660//768
#define ONE_TIME_STEPS 					500
#elif defined (S202)
#define MAX_STEPS_X 4000
#define MAX_STEPS_Y 2000
#define ONE_TIME_STEPS 					500
#elif defined (S203)
#define MAX_STEPS_X 4000
#define MAX_STEPS_Y 768*2
#define ONE_TIME_STEPS 					500
#else
#define MAX_STEPS_X 4000
#define MAX_STEPS_Y 768*2
#define ONE_TIME_STEPS 					500
#endif
//#define V_WORK
#define MIN_STEPS_X 					0//1024
#define MIN_STEPS_Y 					MIN_STEPS_X//768
#define FRAMEINFO_WIDTH_YSX				640
#define FRAMEINFO_HIGH_YSX				360
#define FRAMEINFO_CEN_WIDTH_YSX			(FRAMEINFO_WIDTH_YSX/2)
#define FRAMEINFO_CEN_HIGH_YSX			(FRAMEINFO_HIGH_YSX/2)
#define FRAMEINFO_WIDTH_PTZ_LIMI_YSX	(FRAMEINFO_CEN_WIDTH_YSX-15)
#if defined (S201A)
#define PTZ_NOT_MOTOR_STEP				(6)
#define PTZ_MOVE_LARGE_DELT_CHG			(6)
#elif defined (S201)
#define PTZ_NOT_MOTOR_STEP				(8)
#define PTZ_MOVE_LARGE_DELT_CHG			(4)
#endif
#define ATRACK_WIDTH_MAX				FRAMEINFO_WIDTH_YSX
#define ATRACK_WIDTH_MIN				(16)
#define ATRACK_AREA_MAX					FRAMEINFO_WIDTH_YSX*FRAMEINFO_HIGH_YSX//(160000)//pixel
#define ATRACK_AREA_MIN					(ATRACK_WIDTH_MIN*ATRACK_WIDTH_MIN)
#define PTZ_DELAY_TIMES					(100)//(200)
//circle, horizontal work
#define H_CIR_PIXEL						(-320)//(-180)//(-320)
#define H_CIR_STEP						(320-H_CIR_PIXEL)
#define H_CIR_RADIUS_SQUARE				(H_CIR_PIXEL*H_CIR_PIXEL+H_CIR_STEP*H_CIR_STEP)
#if defined (S301A)
#define H_CIR_STEP_OFFSET				493/600
#elif defined (S301)
#define H_CIR_STEP_OFFSET				493/600
#endif
#define GOLD_POS_H						(MAX_STEPS_X/2)
#if defined (S301A)
#define GOLD_POS_V						(124)
#elif defined (S301)
#define GOLD_POS_V						(68)//(18)
#endif
#define PTZ_STEPS_MOVE_ERR				(-2)
#ifdef V_WORK//for vertical work
#define HORIZONTAL_VERTICAL_OFFSET		100
#define V_MIN_OFFSET					50
#define V_INIT_OFFSET					50
#define V_ZERO_OFFSET					0
#define V_MAX_OFFSET					100
#define V_CIR_PIXEL						(-320)//(-180)//(-320)
#define V_CIR_STEP						(FRAMEINFO_HIGH_YSX-V_CIR_PIXEL)
#define V_CIR_RADIUS_SQUARE				(V_CIR_PIXEL*V_CIR_PIXEL+V_CIR_STEP*V_CIR_STEP)
#define V_CIR_OFFSET					13/15
#define V_CIR_STEP_OFFSET				3/4
#endif
#define PTZ_POINT_NUM					0x8//0x20
#define PTZ_POINT_NUM_BIT				0xFF//0xFFFFFFFF


int get_motor_status();

int PTZ_Init();
void *LimitSwitch_thread(void *arg);
void *PTZ_thread(void *arg);
void Ptz_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo);
void *motion_tracker_thread(void *arg);
int ipcam_set_ptz_cpoint(SMsgAVIoctrlSetPTZSavePointReq *ptz_point);
void avioctrl_ptz_goto_nextpoint(int ptz_point);

int ptz_set_steps_h(int direction, int steps);
int ptz_set_steps_v(int direction, int steps);
#if defined (S301)
int ptz_get_speed(int which);
#endif
/**
 * PTZ 运动等待时间定义数据
 */
typedef enum {
	ATRACK_DELAY 			= 0x5,
	PTZ_DELAY				= 0xFFF,
	PTRACK_DELAY 			= 0x10,//(16)
	P2ATRACK_DELAY 			= 0x6,
	SDINITPOSITION_DELAY 	= 0x2,
	PTRACK_STEP_DELAY		= 0xa//0x3C,//60"
}ptz_delay_e;
/**
 * PTZ 方向定义数据
 */

typedef enum{
	DOWN_DIR_YSX 	= 1,
	UP_DIR_YSX 		= 0,
	LEF_DIR_YSX 	= 1,
	RIG_DIR_YSX 	= 0,
	VERT_DIR_YSX	= PTZ_MOTOR_UP,
	HORI_DIR_YSX	= PTZ_MOTOR_DOWN,
	HORI_VERT_DIR_YSX,
} ptz_dir_e;


#if 0
#if defined (S301)

typedef enum {
	DOWN_DIR_YSX 	= PTZ_MOTOR_DOWN,   ///1
	UP_DIR_YSX 		= PTZ_MOTOR_UP,     ///0
	LEF_DIR_YSX 	= 1,//UP_DIR_YSX,       ///0
	RIG_DIR_YSX 	= 0,//DOWN_DIR_YSX,    ////1
	VERT_DIR_YSX	= PTZ_MOTOR_UP,
	HORI_DIR_YSX	= PTZ_MOTOR_DOWN,
	HORI_VERT_DIR_YSX,
}ptz_dir_e;
#else
typedef enum{
	DOWN_DIR_YSX 	= PTZ_MOTOR_UP,
	UP_DIR_YSX 		= PTZ_MOTOR_DOWN,
	LEF_DIR_YSX 	= DOWN_DIR_YSX,
	RIG_DIR_YSX 	= UP_DIR_YSX,
	VERT_DIR_YSX	= PTZ_MOTOR_UP,
	HORI_DIR_YSX	= PTZ_MOTOR_DOWN,
	HORI_VERT_DIR_YSX,
} ptz_dir_e;
#endif
#endif
/**
 * 定点巡航有效点个数定义数据
 */
typedef enum {
	PTZ_TRACK_CLR = 0,	//清除定点巡航有效点
	PTZ_TRACK_ONE,
	PTZ_TRACK_MORE,
}ptrack_pointvalid_e;
/**
 * 定点巡定时定义数据
 */
typedef enum {
	PTRACK_OFF = 0,
	PTRACK_ONE,
	PTRACK_ALWAY,
}ptrack_time_e;
/**
 * 摇头运动类型定义数据
 */
typedef enum {
	TYPE_MIN = -1,
	TYPE_AUTO_RUN,
	TYPE_ATRACK,
	TYPE_PTRACK,
	TYPE_PRE_PTRACK,
	TYPE_PMOVE,
	TYPE_PTZ_MOVE,		//5
	TYPE_MAX,
}ptz_motion_type_e;
/**
 * 摇头步数记录类型定义数据
 */
typedef enum {
	RECORD_MIN = -1,
	RECORD_POINT,
	RECORD_ATPOINT,
	RECORD_MAX,
}ptz_precord_type_e;
/**
 * 坐标差值数据
 */
typedef struct{
	int dx;	  /**< 横向的坐标差值 */
	int dy;	  /**< 纵向的坐标差值 */
}pointdelt_t;
/**
 * PTZ 方向属性数据
 */
typedef struct {
	int  hori_vert;
	int  rig_lef;
	int  up_down;
}ptz_direction_t;
/**
 * PTZ 运动使能
 */
typedef struct {
	int  arun:1;
	int  atrack:1;
	int  ptrack:1;
}ptz_motion_switch_s;
/**
 * 坐标数据
 */
typedef struct{
    int x;    /**< 横向的坐标值 */
    int y;    /**< 纵向的坐标值 */
    int ul_x;    /*左上横向的坐标值 */
    int ul_y;    /*左上纵向的坐标值 */
    int br_x;    /*右下横向的坐标值 */
    int br_y;    /*右下纵向的坐标值 */
}ptz_point_t;
/**
 * 重启记录到文件的设备状态数据
 */
typedef struct{
    int hori;		/**< 横向的坐标值 */
    int vert;		/**< 纵向的坐标值 */
	int dev_boot;	/**< 设备重启标记 0: dev reboot, 1: tstreamer reboot*/
}ptz_steps_t;
/**
 * IMP_IVS 移动追踪数据
 */
typedef struct {
	ptz_point_t ptz_point;
	pointdelt_t pointdelt_ivs;
	pointdelt_t ptz_tracker_delt;
	ptz_direction_t ptz_tracker_dir;
	int count;
	bool flag;
	bool first_flag;
}ptz_param_input_t;
/**
 * IMP_IVS 移动追踪坐标优化数据
 */
typedef struct {
	int first_delt;
	int average;
	int cnt;
}ptz_motion_trail_t;

/**
 * @fn int PTZ_Init(void)
 *
 * PTZ 初始化，开启 motor 控制
 *
 * @param[in] 无
 *
 * @retval 0 成功
 * @retval -1 失败
 *
 * @remarks 调用此 API 时 pthread_mutex_init, pthread_cond_init 被调用。
 *
 * @attention 无。
 */
int PTZ_Init(void);
void *LimitSwitch_thread(void *arg);
/**
 * @fn void *PTZ_thread(void *arg)
 *
 * PTZ 线程，自动巡航，定点巡航，移动追踪，定点移动，云台操控
 *
 * @param[in] 无
 *
 * @retval 0 成功
 * @retval -1 失败
 *
 * @remarks 调用此 API 时 PTZ_Init 被调用。
 *
 * @attention 无任务时，ptz_cond, ptz_mutex 等待
 */
void *PTZ_thread(void *arg);
/**
 * @fn void Ptz_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo)
 *
 * app 操作 motor 接口
 *
 * @param[in] SID 函数未处理， session ID ，区分APP
 * @param[in] avIndex APP 下发 IO 控制命令时通过 AV 通道使用的通道号
 * @param[in] buf IO 控制命令内容
 * @param[in] type IO 控制命令类型
 * @param[in] iMediaNo 函数未处理， 查看直播流数
 *
 * @retval 无
 *
 * @remarks 无
 *
 * @attention 发送完命令，通过 avSendIOCtrl 上报结果给调用者
 */
void Ptz_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo);
/**
 * @fn void *motion_tracker_thread(void *arg)
 *
 * 完成 IMP_IVS 取流，获取移动物体坐标信息，坐标有效唤醒 PTZ_thread 线程
 *
 * @param[in] 无
 *
 * @retval 无
 *
 * @remarks 调用此 API 时 ivs_motion_tracker_start 被调用。
 *
 * @attention 一直在任务队列
 */
void *motion_tracker_thread(void *arg);
/**
 * @fn int ipcam_set_ptz_cpoint(SMsgAVIoctrlSetPTZSavePointReq *ptz_point)
 *
 * 完成定点收藏添加与删除
 *
 * @param[in] SMsgAVIoctrlSetPTZSavePointReq
 *
 * @retval >0 成功
 * @retval -1 失败
 *
 * @remarks 调用此 API 时 Get_Point_from_conf 等文件操作函数被调用。
 *
 * @attention 一直在任务队列
 */
int ipcam_set_ptz_cpoint(SMsgAVIoctrlSetPTZSavePointReq *ptz_point);
/**
 * @fn int ptz_set_steps_h(int direction, int steps)
 *
 * 完成 motor 左右转动
 *
 * @param[in] direction LEF_DIR_YSX: 左， RIG_DIR_YSX: 右
 * @param[in] steps 范围：(MIN_STEPS_X, MAX_STEPS_X)
 *
 * @retval >0 成功
 * @retval PTZ_STEPS_MOVE_ERR 失败
 *
 * @remarks 调用此 API 时完成记录步数。
 *
 * @attention 无
 */
int ptz_set_steps_h(int direction, int steps);
/**
 * @fn int ptz_set_steps_v(int direction, int steps)
 *
 * 完成 motor 上下转动
 *
 * @param[in] direction DOWN_DIR_YSX: 下， UP_DIR_YSX: 上
 * @param[in] steps 范围：(MIN_STEPS_Y, MAX_STEPS_Y)
 *
 * @retval >0 成功
 * @retval PTZ_STEPS_MOVE_ERR 失败
 *
 * @remarks 调用此 API 时完成记录步数。
 *
 * @attention 无
 */
int ptz_set_steps_v(int direction, int steps);

#endif

