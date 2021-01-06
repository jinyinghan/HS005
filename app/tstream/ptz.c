#include <stdbool.h>
#include "include/common_env.h"
#include "include/common_func.h"


#include "ptz.h"

//#include <ivs_common.h>
//#include <ivs_interface.h>
//#include <ivs_inf_motion_tracker.h>
//#include <imp/imp_ivs.h>


static pthread_cond_t  ptz_cond;
static pthread_mutex_t ptz_mutex;
static int ptz_flag = true, ptz_cmd = -1;;
unsigned char *ptz_autoturn;

int PTZ_Init()
{
	int ret;

	ptz_autoturn = &(g_enviro_struct.ptz_autoturn_flag);

	ret = open_ptz();
	if(ret < 0)
	{
		LOG("### open_motor error\n");
		return -1;
	}

//	ret = ptz_init();
	if(ret < 0)
	{
		LOG("open_motor error\n");
		return -1;
	}

//	ptz_set_steps_y(1,50);

	pthread_mutex_init(&ptz_mutex, NULL);
  	pthread_cond_init(&ptz_cond, NULL);

	return 0;
}
#if 0
int ivs_motion_tracker_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	motion_tracker_param_input_t param;

	memset(&param, 0, sizeof(motion_tracker_param_input_t));
	param.frameInfo.width = 640;
	param.frameInfo.height = 360;
	param.stable_time_out = 13;
	param.move_sense = 2;
	*interface = Motion_TrackerInterfaceInit(&param);
	if (*interface == NULL) {
		printf( "Motion_TrackerInterfaceInit failed\n");
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		printf("IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		printf("IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		printf("IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return (void *)-1;
	}

	return 0;
}
#endif
int data_limit(int value, int max, int min)
{
	int ret;
	if(value > max)
		ret = max;
	if(value < min)
		ret = min;
	else
		ret = value;
	return ret;
}

int ivs_motion_tracker_process(int chn_num)
{
	int ret = 0 ,i ;
	int position_x = 0;

	int cnt ;


	return position_x;
}
int motion_tracker = 0;
int is_moving = 0;
int x_pos = 0;
float delt = 16.0;

#if 0

void *motion_tracker_thread(void *arg)
{
	IMPIVSInterface *inteface = NULL;
	motion_tracker_param_output_t *result = NULL;

	int ret;
	int chn_num = 1;
	ivs_motion_tracker_start(0, 1, &inteface);
	printf("#### motion tracker start ...\n");

	prctl(PR_SET_NAME,"motion_tracker_thread");

	while(1)
	{
		ret = IMP_IVS_PollingResult(chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			printf("IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			continue;
		}


		ret = IMP_IVS_GetResult(chn_num, (void **)&result);
		if (ret < 0) {
			printf( "IMP_IVS_GetResult(%d) failed\n", 0);
			continue;
		}
		// 非转动中
		if(is_moving == 0)
		{
			if( result->count ) {
				pthread_mutex_lock(&ptz_mutex);
				ptz_flag = false;
				x_pos = (result->rect[0].ul.x + result->rect[0].br.x)/2;//result->dx;
				if(x_pos < 0)
					x_pos = 0;
				if(x_pos > 640)
					x_pos = 640;
				delt = abs(result->dx)/5;
				printf("x_pos = %d  dx = %d\n",x_pos,result->dx);
				pthread_cond_signal(&ptz_cond);
				pthread_mutex_unlock(&ptz_mutex);
			}
		}

		ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
		if (ret < 0) {
			printf("IMP_IVS_ReleaseResult(%d) failed\n", 0);
			continue;
		}

	}

}
#endif
#define ONE_STEP 45

void *PTZ_thread(void *arg)
{
	if( PTZ_Init() != 0)
		return (void *)-1;

	prctl(PR_SET_NAME,"PTZ_thread");
	LOG("#### PTZ_thread start ...[%d %d]\n",MAX_STEPS_X,MAX_STEPS_Y);

	// update move time;
	SetYSXCtl(YSX_CAMERA_MOVE, 0);

	int steps = 0;
	int direction = 0;
	int result = 0;

	while(1)
	{
#if 0
		if((*ptz_autoturn) == 1)	/*自动巡航*/
		{
			SetYSXCtl(YSX_CAMERA_MOVE, 0);
			steps = ptz_set_steps_x(direction,ONE_STEP);
			if(steps < ONE_STEP && (*ptz_autoturn) == 1 )
			{
				printf("## Change turn direction @@\n");
				direction = direction > 0 ? 0: 1;
			}
			usleep(1500*1000);
			continue;
		}
#endif


		pthread_mutex_lock(&ptz_mutex);
		while (ptz_flag)
		{
			pthread_cond_wait(&ptz_cond, &ptz_mutex);
		}
		ptz_flag = true;
		pthread_mutex_unlock(&ptz_mutex);
		SetYSXCtl(YSX_CAMERA_MOVE, 0);
#if 0
		if(motion_tracker)
		{
			is_moving = 1;
			result = sqrt(delt*abs(x_pos-320));
			printf("move step %d\n",result);
			if(x_pos < 320 && x_pos > 15){ 		// (100,320)
				printf("turn left now!\n");
				ptz_set_steps_x(1, result);

			}

			if( x_pos > 320 && x_pos < 625)	//(-320,-100)
			{
				printf("turn right now!\n");
				ptz_set_steps_x(0, result);

			}
			x_pos = 0;
//			sleep(1);	//转动后画面稳定
//
			printf("@@@@@ start check done !\n");

//			usleep(100*1000);	//转动后画面稳定
			is_moving = 0;

			continue;

		}
#endif
		switch(ptz_cmd)
		{
			case AVIOCTRL_PTZ_UP:
			{
				steps = ptz_set_steps_y(UP_DIR_YSX,MAX_STEPS_Y);
			}break;
			case AVIOCTRL_PTZ_DOWN:
			{
				steps = ptz_set_steps_y(DOWN_DIR_YSX,MAX_STEPS_Y);
			}break;
			case AVIOCTRL_PTZ_LEFT:
			{
				steps = ptz_set_steps_x(!LEF_DIR_YSX,MAX_STEPS_X);
			}break;
			case AVIOCTRL_PTZ_RIGHT:
			{
				steps = ptz_set_steps_x(!RIG_DIR_YSX,MAX_STEPS_X);
			}break;

			default:
				LOG("not support for ptz cmd %d\n",ptz_cmd);
				break;
		}

	}

	LOG("PTZ_thread thread exit\n");
}

static unsigned int lst_cmd = 0;
void Ptz_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	unsigned int cur_cmd = (tv.tv_sec*1000 + tv.tv_usec/1000);
	printf("ptz command in =================================================\n");
	if (type == IOTYPE_USER_IPCAM_PTZ_COMMAND)
	{
		SMsgAVIoctrlPtzCmd *p = (SMsgAVIoctrlPtzCmd*)buf;
		int step = 0;

		pthread_mutex_lock(&ptz_mutex);
		ptz_flag = false;
		ptz_cmd  = p->control;

		switch(p->control)
		{
			case AVIOCTRL_PTZ_STOP:
			{
				LOG("AVIOCTRL_PTZ_STOP %ld , %ld\n",cur_cmd,lst_cmd);
				int diff = abs(cur_cmd - lst_cmd);
				if(diff <= 500)
					usleep(500*1000);
				LOG("AVIOCTRL_PTZ_STOP now %d\n",diff);
				if((*ptz_autoturn) == 1)
				{
					(*ptz_autoturn) = 0;
					SetYSXCtl(YSX_AUTOTURN, 0);
				}

				ptz_stop_turn(0);
			}break;
			case AVIOCTRL_PTZ_UP:
			{
				if(GetYSXCtl(YSX_INVERSION)) {
					ptz_cmd = AVIOCTRL_PTZ_DOWN;
				}
				LOG("AVIOCTRL_PTZ_UP\n");
			}break;
			case AVIOCTRL_PTZ_DOWN:
			{
				if(GetYSXCtl(YSX_INVERSION)) {
					ptz_cmd = AVIOCTRL_PTZ_UP;
				}
				LOG("AVIOCTRL_PTZ_DOWN\n");
			}break;
			case AVIOCTRL_PTZ_LEFT:
			{
				if(GetYSXCtl(YSX_INVERSION)) {
					ptz_cmd = AVIOCTRL_PTZ_RIGHT;
				}
				LOG("AVIOCTRL_PTZ_LEFT\n");
			}break;
			case AVIOCTRL_PTZ_LEFT_UP:
			{
				LOG("AVIOCTRL_PTZ_LEFT_UP\n");
			}break;
			case AVIOCTRL_PTZ_LEFT_DOWN:
			{
				LOG("AVIOCTRL_PTZ_LEFT_DOWN\n");
			}break;
			case AVIOCTRL_PTZ_RIGHT:
			{
				if(GetYSXCtl(YSX_INVERSION)) {
					ptz_cmd = AVIOCTRL_PTZ_LEFT;
				}
				LOG("AVIOCTRL_PTZ_RIGHT\n");
			}break;
			case AVIOCTRL_PTZ_RIGHT_UP:
			{
				LOG("AVIOCTRL_PTZ_RIGHT_UP\n");
			}break;
			case AVIOCTRL_PTZ_RIGHT_DOWN:
			{
				LOG("AVIOCTRL_PTZ_RIGHT_DOWN\n");
			}break;
			case AVIOCTRL_PTZ_AUTO:
			{
//				return;
			}break;
			case AVIOCTRL_PTZ_SET_POINT:
			{
				LOG("AVIOCTRL_PTZ_SET_POINT\n");
			}break;
			case AVIOCTRL_PTZ_CLEAR_POINT:
			{
				LOG("AVIOCTRL_PTZ_CLEAR_POINT\n");
			}break;
			case AVIOCTRL_PTZ_GOTO_POINT:
			{
				LOG("AVIOCTRL_PTZ_GOTO_POINT\n");
			}break;
			default:
				LOG("not support for ptz cmd %d\n",p->control);
				break;
		}
		pthread_cond_signal(&ptz_cond);
		pthread_mutex_unlock(&ptz_mutex);
		lst_cmd = cur_cmd;
	}
}

