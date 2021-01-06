#include <stdbool.h>
#include "include/common_env.h"
#include "include/common_func.h"

#include "ptz.h"
#ifdef USE_MOVE_TO_TRACK
//motion_track_lib crash device with oom, use move instead
#include <ivs/ivs_inf_move.h>
#endif
#ifdef PATRACK
#include <ivs_common.h>
#include <ivs_interface.h>
#include <ivs_inf_motion_tracker.h>
#include <imp/imp_ivs.h>
#endif

static pthread_cond_t  ptz_cond;
static pthread_mutex_t ptz_mutex;
static struct timeval now;
static struct timespec outtime;
static int ptz_flag = true, patrack_flag = true;
static int ptz_cmd = -1;
unsigned char *ptz_autoturn;
#ifdef PATRACK
static int g_piont_preload = 0;
int motion_tracker = 0, ptz_move = 0;//Young, 接受app指令,Ptz_Cmd()//1
int is_moving = 0;
int x_pos = 0;
#define MAX_FPS_YSX 15
int y_pos[MAX_FPS_YSX] = {0};
ptz_param_input_t ptz_param_input = {
	.ptz_point = {0},
	.pointdelt_ivs = {0},
	.flag = false,
	};
ptz_steps_t ptz_steps = {0, 0, 0};
float delt = 16.0;//Young, default 16.0
SMsgAVIoctrlSetPTZPTrackReq ptz_pointtrack = {
				.point_no_bit = 0,
				.point_en = 0,
				.point_track_en = 0,
				};
static void record_position(ptz_steps_t pos);
static int ptrack_time_ok(void);
void position_reset(ptz_steps_t *ptz_pos);
void Get_Motion_Conf(void);

static int get_steps_ysx(int dir)
{
	if (HORI_DIR_YSX == dir)
		return ptz_steps.hori;
	else if (VERT_DIR_YSX == dir)
		return ptz_steps.vert;
	else
		return -1;
}
static void set_ptz_operate_flag(ptz_motion_type_e ptz_type, int val){
	if ((val > 1) || (val < 0)){
		pr_error("in \n");
		return;
	}
	switch(ptz_type){
		case TYPE_AUTO_RUN:{
			*ptz_autoturn = val;
		}break;
		case TYPE_ATRACK:{
			motion_tracker = val;
		}break;
		case TYPE_PTRACK:{
			ptz_pointtrack.point_track_en = val;
		}break;
		case TYPE_PRE_PTRACK:{
			g_piont_preload = val;
		}break;
		case TYPE_PTZ_MOVE:{
			ptz_move = val;
		}break;
		default:
			pr_error("in \n");
		break;
	}
//	pr_dbg("out, val:(%d,%d)\n", ptz_type, val);
}

static bool get_ptz_operate_flag(ptz_motion_type_e ptz_type){
	int i_ret = 0;
	switch(ptz_type){
		case TYPE_AUTO_RUN:{
			i_ret = *ptz_autoturn;
		}break;
		case TYPE_ATRACK:{
			i_ret = motion_tracker;
		}break;
		case TYPE_PTRACK:{
			i_ret = ptz_pointtrack.point_track_en;
		}break;
		case TYPE_PRE_PTRACK:{
			i_ret = g_piont_preload;
		}break;
		case TYPE_PTZ_MOVE:{
			i_ret = ptz_move;
		}break;
		default:
			pr_error("in \n");
		break;
	}
//	pr_dbg("type: %d, ret %d\n", ptz_type, i_ret);
	return (i_ret>0)?true:false;
}

static void off_ptz_switch(ptz_motion_type_e ptz_type)
{
	if (get_ptz_operate_flag(ptz_type)){
		pr_info("ptz_type %d set off\n", ptz_type);
		set_ptz_operate_flag(ptz_type, 0);
		SetYSXCtl(ptz_type, get_ptz_operate_flag(ptz_type));
	}
}

static void set_ptz_ptrack(int no_bit, int p_en, int pt_en)
{
	if (p_en == 0)
	{
		ptz_pointtrack.point_no_bit = no_bit;
		set_ptz_operate_flag(TYPE_PTRACK, pt_en);
	}
	ptz_pointtrack.point_en = p_en;
}
static int get_ptz_point_flag(void)
{
	return ptz_pointtrack.point_en;
}
static void set_ptz_point_flag(int p_en)
{
	ptz_pointtrack.point_en = p_en;
}

#ifdef IOTYPE_USER_IPCAM_PTZ_AUTORUN_RESP
static int set_ptz_arun_flag(SMsgAVIoctrlSetPTZAutoRunReq *arun)
{
	if (NULL == arun)
	{
		pr_error("in,\n");
		return -1;
	}
	set_ptz_operate_flag(TYPE_AUTO_RUN, arun->autorun_en);
	if (arun->autorun_en != 1)
		record_position(ptz_steps);
	return 0;
}
#endif
/*
人为手动操作云台>自动追踪>定点巡航。
同时开启自动追踪和定点巡航，会执行有物体移动时去追踪，
没有物体移动或物体追踪结束时，按先前的定点巡航路径继续接着巡航。

*/
static int get_ptz_operate_mutex(ptz_motion_type_e motion_type){
	int i_ret = 0, i_c_en = -1;
	switch(motion_type){
		case TYPE_PTZ_MOVE:{
			if (get_ptz_operate_flag(TYPE_PTZ_MOVE) == true){
				i_ret = 0;
				i_c_en = 0;
			}
		}break;
		case TYPE_ATRACK:{
			if (get_ptz_operate_flag(TYPE_ATRACK) == true){
				i_c_en = 0;
			i_ret += get_ptz_operate_flag(TYPE_PTZ_MOVE);
			}
		}break;
		case TYPE_PTRACK:{
#ifdef IOTYPE_USER_IPCAM_PTRACK_TIME_RESP
			if((get_ptz_operate_flag(TYPE_PTRACK) == true) && (ptrack_time_ok() > 0)) {
#else
			if (get_ptz_operate_flag(TYPE_PTRACK) == true){
#endif
				i_c_en = 0;
			i_ret += get_ptz_operate_flag(TYPE_PTZ_MOVE);
//			i_ret += get_ptz_operate_flag(TYPE_ATRACK);
			}
//			pr_dbg("out, i_ret:(%d,%d,%d)\n", motion_type, i_c_en, i_ret);
		}break;
		case TYPE_AUTO_RUN:
			if ((get_ptz_operate_flag(TYPE_AUTO_RUN) == true)){
				i_c_en = 0;
			i_ret += get_ptz_operate_flag(TYPE_PTZ_MOVE);
			i_ret += get_ptz_operate_flag(TYPE_ATRACK);
			i_ret += get_ptz_operate_flag(TYPE_PTRACK);
			}
		case TYPE_PMOVE:{
			if (get_ptz_point_flag() > 0){
				i_c_en = 0;
			i_ret += get_ptz_operate_flag(TYPE_PTZ_MOVE);
			i_ret += get_ptz_operate_flag(TYPE_ATRACK);
			i_ret += get_ptz_operate_flag(TYPE_PTRACK);
			}
		}break;
		default:
			pr_error("in,\n");
			break;
	}
	if (GetYSXCtl(YSX_C_EN) == 0)
		i_ret += 1;
//	pr_dbg("out, i_ret:(%d,%d,%d)\n", motion_type, i_c_en, i_ret);
	return ((!i_c_en)&&(!i_ret))?true:false;
}

static int get_ptrack_nobit_from_conf(void)
{
	int i_cnt,i_ret = 0x0;
	for (i_cnt=1; i_cnt<=PTZ_POINT_NUM;i_cnt++)
	{
		if ((Get_Point_from_conf(YSX_POINT_TRACK_H, i_cnt) != -1) &&
			(Get_Point_from_conf(YSX_POINT_TRACK_H, i_cnt) != -1))
			i_ret |= (0x1 << (i_cnt - 1));
	}
	return i_ret;
}

static int get_ptz_ptrack(SMsgAVIoctrlPTZPointTrackResp *p_track)
{
	if (NULL == p_track)
	{
		pr_error("in,\n");
		return -1;
	}
	if (ptz_pointtrack.point_track_en)
		p_track->point_no_bit = ptz_pointtrack.point_no_bit;
	else
		p_track->point_no_bit = 0;
	p_track->result = 0;
	return get_ptz_operate_flag(TYPE_PTRACK);
}

static void position_init(void)
{
	ptz_set_steps_h(LEF_DIR_YSX, MAX_STEPS_X);
	ptz_set_steps_v(DOWN_DIR_YSX, MAX_STEPS_Y);
}

static void update_position(ptz_steps_t *pos)
{
//	pr_dbg("in\n");
	pos->hori = Get_Point_from_conf(YSX_POINT_RECORD_H, (int)RECORD_POINT);
	pos->vert = Get_Point_from_conf(YSX_POINT_RECORD_V, (int)RECORD_POINT);
}

static void record_position(ptz_steps_t pos)
{
//	pr_dbg(" in, step:(%d,%d), flag(%d,%d)\n", pos.hori, pos.vert,
//											 pos.dev_boot, pos.dev_boot);
//	if ((get_ptz_operate_flag(TYPE_AUTO_RUN) == true) ||
//		(get_ptz_operate_flag(TYPE_ATRACK) == true) ||
//		(get_ptz_operate_flag(TYPE_PTRACK) == true))
	if (access(SD_INIT_FLAG,F_OK) == 0)
		return;
	if (Get_Point_from_conf(YSX_POINT_RECORD_H, (int)RECORD_POINT) != pos.hori)
		 Set_Point_to_conf(YSX_POINT_RECORD_H, (int)RECORD_POINT, pos.hori);
	if (Get_Point_from_conf(YSX_POINT_RECORD_V, (int)RECORD_POINT) != pos.vert)
		 Set_Point_to_conf(YSX_POINT_RECORD_V, (int)RECORD_POINT, pos.vert);
//	 Set_Point_to_conf(YSX_DEV_INIT, 0, pos.dev_init);
//	 Set_Point_to_conf(YSX_DEV_BOOT, 0, pos.dev_boot);
}

static bool get_initboot_flag(void)
{
	return g_flag_struct.ptz_reset_flag;
}

static void set_initboot_flag(bool en)
{
	g_flag_struct.ptz_reset_flag = en;
}

static void swap_step(int *src_h, int *dest_h)
{
	int tmp_h;
	tmp_h = *src_h;
	*src_h = *dest_h;
	*dest_h = tmp_h;
}
static int get_cpoint_nobit_from_conf(void)
{
	int i_cnt,i_ret = 0x0;
	for (i_cnt=1; i_cnt<=PTZ_POINT_NUM;i_cnt++)
	{
		if ((Get_Point_from_conf(YSX_POINT_CPOINT_H, i_cnt) != -1) &&
			(Get_Point_from_conf(YSX_POINT_CPOINT_V, i_cnt) != -1))
			i_ret |= (0x1 << (i_cnt - 1));
	}
	return i_ret;
}

int ipcam_set_ptz_cpoint(SMsgAVIoctrlSetPTZSavePointReq *p_point)
{
	if (NULL == p_point)
	{
		pr_error("in,\n");
		return -1;
	}
	static int no_bit = 0x0, first = 0;
	int h_step = 0, v_step = 0;
	int i_cnt;
	SMsgAVIoctrlSetPTZSavePointReq ptz_point;
	ptz_point.idel = p_point->idel;
	ptz_point.save_no = p_point->save_no;
	if (first == 0)
	{
		no_bit = get_cpoint_nobit_from_conf();
		first = 1;
	}
//	pr_dbg("idel,idel(%d)\n", ptz_point.idel);
	if (ptz_point.idel)
	{
//		pr_dbg("idel,save_no(0x%x)\n", ptz_point.save_no);
		no_bit &= ~(ptz_point.save_no);
		for (i_cnt=1;i_cnt<=PTZ_POINT_NUM;i_cnt++)
		{
			if (ptz_point.save_no & (0x1 << (i_cnt - 1)))
			{
				if (Get_Point_from_conf(YSX_POINT_CPOINT_H, i_cnt) != -1)
					Set_Point_to_conf(YSX_POINT_CPOINT_H, i_cnt, -1);
				if (Get_Point_from_conf(YSX_POINT_CPOINT_V, i_cnt) != -1)
					Set_Point_to_conf(YSX_POINT_CPOINT_V, i_cnt, -1);
			}
		}
	}
	else
	{
//		pr_dbg("idel,save_no(%d)\n", ptz_point.save_no);
		if (ptz_point.save_no > PTZ_POINT_NUM)
			return -1;
		no_bit |= 0x1 << ptz_point.save_no;
		h_step = get_steps_ysx(HORI_DIR_YSX);
		v_step = get_steps_ysx(VERT_DIR_YSX);
		if (Get_Point_from_conf(YSX_POINT_CPOINT_H, ptz_point.save_no) != h_step)
			Set_Point_to_conf(YSX_POINT_CPOINT_H, ptz_point.save_no, h_step);
		if (Get_Point_from_conf(YSX_POINT_CPOINT_V, ptz_point.save_no) != v_step)
			Set_Point_to_conf(YSX_POINT_CPOINT_V, ptz_point.save_no, v_step);
	}
//	pr_dbg("get no:%d step(%d,%d)\n", ptz_point.save_no, h_step, v_step);
	return no_bit;
}

int ipcam_get_ptz_cpoint(SMsgAVIoctrlPTZSavePointResp *p_point)
{
	if (NULL == p_point)
	{
		pr_error("in,\n");
		return -1;
	}
	p_point->save_no_bit = get_cpoint_nobit_from_conf();
//	pr_dbg("save_no_bit(0x%x)\n", p_point->save_no_bit);
	return 0;
}

int ipcam_set_ptz_atrack(SMsgAVIoctrlSetPTZAutoTrackReq *p_track)
{
	if (NULL == p_track)
	{
		pr_error("in,\n");
		return -1;
	}
//	pr_dbg("in, %d\n",p_track->auto_track_en);
	set_ptz_operate_flag(TYPE_ATRACK, p_track->auto_track_en);
	if (AVIOCTRL_PTZ_STOP == p_track->auto_track_en)
	{
		record_position(ptz_steps);
	}
	return get_ptz_operate_flag(TYPE_ATRACK);
}
int ipcam_get_ptz_atrack(SMsgAVIoctrlPTZAutoTrackResp *p_track)
{
	if (NULL == p_track)
	{
		pr_error("in,\n");
		return -1;
	}
//	pr_dbg("in, %d\n",motion_tracker);
	p_track->auto_track_en = motion_tracker;
	p_track->result = 0;
	return motion_tracker;
}

void ptz_goto_nextpoint(  int h_step, int v_step)
{
//	pr_dbg(" in, step(%d, %d)\n", h_step, v_step);
	if ((h_step < 0) || (v_step < 0))
		return;
	int i_hstep = 0, i_vstep = 0;
	i_hstep = get_steps_ysx(HORI_DIR_YSX);
	i_vstep = get_steps_ysx(VERT_DIR_YSX);
//	pr_dbg(" in, step_cur(%d, %d)\n", i_hstep, i_vstep);
#if defined (S301)
	//ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_LOW_SPEED);
#endif
	ptz_set_steps_h(((h_step - i_hstep) > 0) ? RIG_DIR_YSX : LEF_DIR_YSX, abs(h_step - i_hstep));
	ptz_set_steps_v(((v_step - i_vstep) > 0) ? UP_DIR_YSX : DOWN_DIR_YSX, abs(v_step - i_vstep));
#if defined (S301)
//	pr_dbg("in ptz cmd %d, speed (%d, %d)\n",ptz_cmd, ptz_get_speed(HORI_DIR_YSX), ptz_get_speed(VERT_DIR_YSX));
#endif
}

static void set_track_dir_postion(int *track_h_step, int *track_v_step, int *i_dir,
										int *i_postion, int i_circle)
{
//	pr_dbg(" in, var(%d, %d, %d)\n", *i_dir, *i_postion, i_circle);
	int i_cnt = 0, h_step_cur = 0;
	h_step_cur = get_steps_ysx(HORI_DIR_YSX);
	for (i_cnt=0;i_cnt<i_circle;)
		if (h_step_cur>track_h_step[i_cnt])
			i_cnt++;
		else
			break;
	if (i_cnt == 0)
	{
		*i_dir = LEF_DIR_YSX;
		*i_postion = i_cnt;
	}
	else if (i_cnt == i_circle)
	{
		*i_dir = RIG_DIR_YSX;
		*i_postion = i_cnt - 1;
	}
	else
	{
		if (abs(h_step_cur-track_h_step[i_cnt]) < abs(h_step_cur-track_h_step[i_cnt-1]))
		{
			*i_dir = RIG_DIR_YSX;
			*i_postion = i_cnt;
		}
		else
		{
			*i_dir = LEF_DIR_YSX;
			*i_postion = i_cnt - 1;
		}
	}
//	pr_dbg(" in, var(%d, %d, %d)\n", *i_dir, *i_postion, i_circle);
}
#if 0
static bool Motion_Large_Deltchg_YSX(int i_delt)
{
	static ptz_motion_trail_t ptz_motion_trail = {0};
	if (ptz_motion_trail.cnt%PTZ_MOVE_LARGE_DELT_CHG == 0)
		ptz_motion_trail.first_delt = ptz_motion_trail.average = i_delt;
	ptz_motion_trail.average = (ptz_motion_trail.average + i_delt)/2;
//	pr_dbg("in, delt(%d, %d, %d), cnt: %d\n", ptz_motion_trail.first_delt,
//						ptz_motion_trail.average, i_delt, ptz_motion_trail.cnt);
	if (ptz_motion_trail.first_delt == ptz_motion_trail.average)
	{
		ptz_motion_trail.cnt ++;
		if (ptz_motion_trail.cnt == PTZ_MOVE_LARGE_DELT_CHG)
		{
			ptz_motion_trail.cnt = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		ptz_motion_trail.cnt = 0;
		return true;
	}
}
#endif
#ifndef USE_MOVE_TO_TRACK
static ptz_point_t Getpointinfo_fromivs(motion_tracker_param_output_t *result)
{
	ptz_point_t ptz_point = {0};
	int i_area = 0, i_max_area = 0, i_max_count = 0, i_cnt;
	int i_width = 0;
	for(i_cnt=0; i_cnt<result->count; i_cnt++){
		i_width = (result->rect[i_cnt].br.x - result->rect[i_cnt].ul.x);
		i_area = i_width * (result->rect[i_cnt].br.y - result->rect[i_cnt].ul.y);
		if ((i_area > ATRACK_AREA_MAX) || (i_area < ATRACK_AREA_MIN) ||
			(i_width > ATRACK_WIDTH_MAX) || (i_width < ATRACK_WIDTH_MIN)){
			continue;
		}
		if(i_area>i_max_area)
		{
			i_max_count = i_cnt;
			i_max_area = i_area;
		}
	}
	ptz_point.ul_x = result->rect[i_max_count].ul.x;
	ptz_point.ul_y = result->rect[i_max_count].ul.y;
	ptz_point.br_x = result->rect[i_max_count].br.x;
	ptz_point.br_y = result->rect[i_max_count].br.y;
	ptz_point.x = (result->rect[i_max_count].ul.x + result->rect[i_max_count].br.x)/2;
	ptz_point.y = (result->rect[i_max_count].ul.y + result->rect[i_max_count].br.y)/2;
	return ptz_point;
}

static int Getpositioninfo_fromivs(motion_tracker_param_output_t *result , ptz_param_input_t *ptz)
{
	ptz_param_input_t ptz_inf = {
		.ptz_point = {0},
		.pointdelt_ivs = {0},
		};
	unsigned int ui_delt_dx = 0;
	int delt_dx = 0;
	int i_offset = 1;
#ifdef V_WORK//for vertical work, may lost the focus
	unsigned int ui_delt_dy = 0;
	int delt_dy = 0;
#endif
	ptz_inf.flag = ptz_param_input.flag;
	ptz_inf.ptz_point = Getpointinfo_fromivs(result);
	ptz_inf.count = result->count;
	ptz_inf.pointdelt_ivs.dx = result->dx;
	delt_dx = ptz_inf.ptz_point.x -FRAMEINFO_CEN_WIDTH_YSX;
	ui_delt_dx = abs(delt_dx);
    int rect_width = abs(ptz_inf.ptz_point.br_x-ptz_inf.ptz_point.ul_x);
    if(ptz_inf.ptz_point.br_x < 0 || ptz_inf.ptz_point.ul_x < 0 ||
		ptz_inf.ptz_point.x < 10 || ptz_inf.ptz_point.x > 650 ||
		rect_width < 20 || rect_width > 480)
    {
//		pr_dbg("rect_width: %d,x(%d, %d, %d)\n",rect_width, ptz_inf.ptz_point.x,
//												ptz_inf.ptz_point.ul_x, ptz_inf.ptz_point.br_x);
        return -1;
    }else if((ptz_inf.ptz_point.x < 100 || ptz_inf.ptz_point.x > 550) &&
		(rect_width > 50))
		i_offset = 2;
	pr_dbg("rect_width: %d,x(%d, %d, %d)\n",rect_width, ptz_inf.ptz_point.x,
											ptz_inf.ptz_point.ul_x, ptz_inf.ptz_point.br_x);
//	if ((ptz_inf.pointdelt_ivs.dx == 0)||
//		((!Motion_Large_Deltchg_YSX(ui_delt_dx))&&(ui_delt_dx < PTZ_NOT_MOTOR_STEP)))
//	{
//		ui_delt_dx = 0;
//	}
	ptz_inf.ptz_tracker_delt.dx = (H_CIR_STEP-sqrt(H_CIR_RADIUS_SQUARE-\
								(ui_delt_dx-H_CIR_PIXEL)*(ui_delt_dx-H_CIR_PIXEL)))*\
								H_CIR_STEP_OFFSET;
	pr_dbg("dx: %d\n",ptz_inf.ptz_tracker_delt.dx);
	ptz_inf.ptz_tracker_delt.dx /=i_offset;
	pr_dbg("dx: %d\n",ptz_inf.ptz_tracker_delt.dx);
	if(GetYSXCtl(YSX_INVERSION) > 0)
	ptz_inf.ptz_tracker_dir.rig_lef = (delt_dx < 0) ? LEF_DIR_YSX : RIG_DIR_YSX;
	else
	ptz_inf.ptz_tracker_dir.rig_lef = (delt_dx < 0) ? RIG_DIR_YSX : LEF_DIR_YSX;
				
	//ptz_inf.ptz_tracker_dir.rig_lef = (delt_dx < 0) ? RIG_DIR_YSX : LEF_DIR_YSX;
#ifdef V_WORK//for vertical work, may lost the focus
	static int v_step_record = V_INIT_OFFSET;
	ptz_inf.pointdelt_ivs.dy = result->dy;
	delt_dy = ptz_inf.ptz_point.y -FRAMEINFO_CEN_HIGH_YSX;
	ui_delt_dy = abs(delt_dy);
	if (((ptz_inf.pointdelt_ivs.dy == 0) && (!Motion_Large_Deltchg_YSX(ui_delt_dy)))||
		(ui_delt_dy < PTZ_NOT_MOTOR_STEP))
	{
		ui_delt_dy = 0;
	}
	if (ui_delt_dy > (V_MAX_OFFSET - V_MIN_OFFSET))
		v_step_record = (delt_dy > 0) ? V_MIN_OFFSET : V_MAX_OFFSET;
	else
		v_step_record +=delt_dy;
	if (v_step_record < V_MIN_OFFSET)
	{
		v_step_record = V_MIN_OFFSET;
		ptz_inf.ptz_tracker_dir.hori_vert = HORI_DIR_YSX;
	}
	else if(v_step_record <= V_MAX_OFFSET)
	{
		if (ptz_inf.ptz_tracker_delt.dx > 0)
			ptz_inf.ptz_tracker_dir.hori_vert = HORI_VERT_DIR_YSX;
		else
			ptz_inf.ptz_tracker_dir.hori_vert = VERT_DIR_YSX;
		ptz_inf.ptz_tracker_delt.dy = (V_CIR_STEP-sqrt(H_CIR_RADIUS_SQUARE-\
									(ui_delt_dy-V_CIR_PIXEL)*(ui_delt_dy-V_CIR_PIXEL)))*\
									V_CIR_STEP_OFFSET;//circle/2
		ptz_inf.ptz_tracker_dir.up_down = (delt_dy > 0) ? UP_DIR_YSX : DOWN_DIR_YSX;
	}
	else
	{
		v_step_record = V_MAX_OFFSET;
		ptz_inf.ptz_tracker_dir.hori_vert = HORI_DIR_YSX;
	}
#endif
    memcpy(ptz,&ptz_inf, sizeof(ptz_param_input_t));
	return 0;
}
#else
static ptz_point_t Getpointinfo_fromivs(move_param_output_t *result)
{
	ptz_point_t ptz_point = {0};
	int i_area = 0, i_max_area = 0, i_max_count = 0, i_cnt;
	for(i_cnt=0; i_cnt<result->count; i_cnt++){
		i_area = (result->rects[i_cnt].br.x - result->rects[i_cnt].ul.x) * (result->rects[i_cnt].br.y - result->rects[i_cnt].ul.y);
		if(i_area>i_max_area)
		{
			i_max_count = i_cnt;
			i_max_area = i_area;
		}
	}
	ptz_point.ul_x = result->rects[i_max_count].ul.x;
	ptz_point.ul_y = result->rects[i_max_count].ul.y;
	ptz_point.br_x = result->rects[i_max_count].br.x;
	ptz_point.br_y = result->rects[i_max_count].br.y;
	ptz_point.x = (result->rects[i_max_count].ul.x + result->rects[i_max_count].br.x)/2;
	ptz_point.y = (result->rects[i_max_count].ul.y + result->rects[i_max_count].br.y)/2;
	return ptz_point;
}

static int Getpositioninfo_fromivs(move_param_output_t *result , ptz_param_input_t *ptz)
{
    int ret = -1;
	ptz_param_input_t ptz_inf = {
		.ptz_point = {0},
		.pointdelt_ivs = {0},
		};

    static ptz_param_input_t last_ptz_inf = {0};
    static uint64_t last_msc = 0;
	unsigned int ui_delt_dx = 0;
	int delt_dx = 0;
	ptz_inf.flag = ptz_param_input.flag;
	ptz_inf.first_flag = ptz_param_input.first_flag;
	ptz_inf.ptz_point = Getpointinfo_fromivs(result);
	ptz_inf.count = result->count;
	delt_dx = ptz_inf.ptz_point.x -FRAMEINFO_CEN_WIDTH_YSX;
	ui_delt_dx = abs(delt_dx);

    if(ptz_inf.ptz_point.x < 20 || ptz_inf.ptz_point.x > 620)
        return -1;

    int rect_width = abs(ptz_inf.ptz_point.br_x-ptz_inf.ptz_point.ul_x);
    if(rect_width < 10 || rect_width > 480)
    {
        memset(&last_ptz_inf,0,sizeof(ptz_param_input_t));
        return -1;
    }


    struct timeval tv;
    gettimeofday(&tv,NULL);
    uint64_t msc = tv.tv_sec * 1000 + tv.tv_usec / 1000 ;
    if(abs(msc - last_msc) < 80 ) //开始转动
    {
        float action = 1.0;
        int diff = last_ptz_inf.ptz_point.x - ptz_inf.ptz_point.x;
        if(diff > 0) // out
        {
            action = 1.0;
        }
        if(diff < 0) // in
        {
            action = 0.8;
        }

        float KP = 1.0  ;
        if(ui_delt_dx < 50)
            KP = 0.5;
        else if(ui_delt_dx < 100)
            KP = 0.55;
        else if(ui_delt_dx < 150)
            KP = 0.6;
        else if(ui_delt_dx < 200)
            KP = 0.65;
        else if(ui_delt_dx < 250)
            KP = 0.7;
        float steps=(1.0*KP*ui_delt_dx);
        steps *= action;
        if(steps > 125 &&  rect_width < 70)
            steps = 125;
        if(steps > 125 &&  rect_width < 300)
            steps = 160;

        if(steps < 0)
            steps = 0;
//        printf("KP = %f , diff = %d steps = %f ",KP,diff,steps);

    	ptz_inf.ptz_tracker_delt.dx = (int)steps;
    	ptz_inf.ptz_tracker_dir.rig_lef = (delt_dx < 0) ? RIG_DIR_YSX : LEF_DIR_YSX;
//        printf("[%d,%d] delt = %d, steps = %d\n",ptz_inf.ptz_point.x,ptz_inf.ptz_point.y,ui_delt_dx,ptz_inf.ptz_tracker_delt.dx);
        ret = 0;
        memcpy(&last_ptz_inf,&ptz_inf, sizeof(ptz_param_input_t));
    }

    //save current record
    last_msc = msc;
    memcpy(ptz,&ptz_inf, sizeof(ptz_param_input_t));

	return ret;
}
#endif
#ifdef IOTYPE_USER_IPCAM_PTRACK_TIME_RESP
static int ptrack_time_ok(void)
{
	int ptime_s, ptime_e;
	ptime_s = GetYSXCtl(YSX_PTRACK_TS);
	ptime_e = GetYSXCtl(YSX_PTRACK_TE);
	if (GetYSXCtl(YSX_PTRACK_TT) == PTRACK_OFF)
		return 0;
	time_t timep;
	struct tm *sync_start_time;
	int invaild_time  = 0;
//	if (access("/tmp/sync_time",F_OK) != 0)
//        return 0;
	timep = time(NULL);
	sync_start_time = localtime(&timep);
	if(ptime_s > ptime_e )	/*跨天*/
	{
		if((sync_start_time->tm_hour >= ptime_e) &&
			(sync_start_time->tm_hour < ptime_s))
			invaild_time = 1;
			pr_info(" time: %d\n",sync_start_time->tm_hour);
	}else if(ptime_s < ptime_e ) /*0-24*/
	{
		if ((sync_start_time->tm_hour < ptime_s) ||
			(sync_start_time->tm_hour >= ptime_e))
			invaild_time = 1;
			pr_info(" time: %d\n",sync_start_time->tm_hour);
	}else{
		;/*start == end */
	}
//	pr_dbg("time:(%d, %d, %d),flag:%d\n", sync_start_time, ptime_s, ptime_e, invaild_time);
	if ((invaild_time == 1) &&
		(GetYSXCtl(YSX_PTRACK_TT) == PTRACK_ONE) &&
		get_ptz_operate_flag(TYPE_PRE_PTRACK)){
		SetYSXCtl(YSX_PTRACK_TT, PTRACK_OFF);
		set_ptz_operate_flag(TYPE_PRE_PTRACK, 0);
		set_ptz_operate_flag(TYPE_PTRACK, 0);
		}
	return !invaild_time;//
}

#endif
int ipcam_ptz_ptrack(int *track_h_step, int *track_v_step, int *i_dir,
							int *i_postion, int *i_cir)
{
//	pr_dbg(" in\n");
	int i_cnt = 0;
	if (*i_cir == 0)
		for (i_cnt=0;i_cnt<PTZ_POINT_NUM;i_cnt++)
		{
//			pr_dbg(" in\n");
			track_h_step[i_cnt] = Get_Point_from_conf(YSX_POINT_TRACK_H, i_cnt);
			track_v_step[i_cnt] = Get_Point_from_conf(YSX_POINT_TRACK_V, i_cnt);
			if ((track_h_step[i_cnt] == -1) ||
				(track_v_step[i_cnt] == -1))
			{
//				pr_dbg(" %d, h_v_step(%d, %d)\n", i_cnt,
//						track_h_step[i_cnt], track_v_step[i_cnt]);
				break;
			}
		}
	else
		i_cnt = *i_cir + 1;
	if (i_cnt > 1){
		*i_cir = i_cnt;
		set_track_dir_postion(track_h_step, track_v_step, i_dir, i_postion, *i_cir);
		(*i_cir)--;
		if (get_ptz_operate_flag(TYPE_PRE_PTRACK))
			pr_error("in\n");
		set_ptz_operate_flag(TYPE_PRE_PTRACK, 1);
	}
	else{
		set_ptz_operate_flag(TYPE_PTRACK, 0);
	}
	return 0;
}

int ipcam_set_ptz_ptrack(SMsgAVIoctrlSetPTZPointTrackReq *p_track)
{
	if (NULL == p_track)
	{
		pr_error("in,\n");
		return -1;
	}
	SMsgAVIoctrlSetPTZPointTrackReq ptz_track;
	ptz_track.point_no_bit = p_track->point_no_bit;
//	pr_dbg("save_no_bit(0x%x)\n", ptz_track.point_no_bit);
	int i_cnt,j_cnt,i_sum,i_one_p = 0, p_en = 0, pt_en = 0;
	int track_h_step[PTZ_POINT_NUM] = {0};
	int track_v_step[PTZ_POINT_NUM] = {0};
	for (i_cnt=1, j_cnt=PTZ_TRACK_CLR; ptz_track.point_no_bit!=0 && i_cnt<=PTZ_POINT_NUM;i_cnt++)
		if (ptz_track.point_no_bit & (0x1 << (i_cnt - 1)))
			if (++j_cnt == PTZ_TRACK_ONE)
				i_one_p = i_cnt;
//	pr_dbg("in (%d,%d)\n", j_cnt,i_one_p);
	set_ptz_operate_flag(TYPE_PRE_PTRACK, 0);
	switch (j_cnt)
	{
		case PTZ_TRACK_CLR:
		{
			set_ptz_operate_flag(TYPE_PTRACK, 0);
			for (i_cnt=0; (Get_Point_from_conf(YSX_POINT_TRACK_H, i_cnt) != -1) &&
						(Get_Point_from_conf(YSX_POINT_TRACK_V, i_cnt) != -1) &&
						i_cnt<=PTZ_POINT_NUM;i_cnt++)
			{
				Set_Point_to_conf(YSX_POINT_TRACK_H, i_cnt, -1);
				Set_Point_to_conf(YSX_POINT_TRACK_V, i_cnt, -1);
			}
			if (i_cnt != 0)
				record_position(ptz_steps);
			else
				return 0;
		}break;
		case PTZ_TRACK_ONE:
		{
			p_en = i_one_p;
		}break;
		case PTZ_TRACK_MORE:
		default:
		{
//			if ((Get_Point_from_conf(YSX_POINT_TRACK_H, 1) == -1) ||
//				(Get_Point_from_conf(YSX_POINT_TRACK_V, 1) == -1))
			{
//				pr_dbg("in \n");
				for (i_cnt=1, j_cnt=0;i_cnt<=PTZ_POINT_NUM;i_cnt++)
				{
					if (ptz_track.point_no_bit & (0x1 << (i_cnt - 1)))
					{
						track_h_step[j_cnt] = Get_Point_from_conf(YSX_POINT_CPOINT_H, i_cnt);
						track_v_step[j_cnt] = Get_Point_from_conf(YSX_POINT_CPOINT_V, i_cnt);
						if ((track_h_step[j_cnt] == -1) ||
							(track_v_step[j_cnt] == -1))
						{
							pr_error("in %d, h_v_step(%d, %d)\n", i_cnt,
									track_h_step[j_cnt], track_v_step[j_cnt]);
							return ptz_track.point_no_bit;
						}
						j_cnt ++;
					}
				}
				i_sum = j_cnt;
				for (j_cnt=0; j_cnt<i_sum;j_cnt++)
				{
					for (i_cnt=j_cnt+1;i_cnt<i_sum;i_cnt++)
						if (track_h_step[j_cnt] > track_h_step[i_cnt])
						{
							swap_step(&track_h_step[j_cnt], &track_h_step[i_cnt]);
							swap_step(&track_v_step[j_cnt], &track_v_step[i_cnt]);
						}
					if (Get_Point_from_conf(YSX_POINT_TRACK_H, j_cnt) != track_h_step[j_cnt])
						Set_Point_to_conf(YSX_POINT_TRACK_H, j_cnt, track_h_step[j_cnt]);
					if (Get_Point_from_conf(YSX_POINT_TRACK_V, j_cnt) != track_v_step[j_cnt])
						Set_Point_to_conf(YSX_POINT_TRACK_V, j_cnt, track_v_step[j_cnt]);
				}
			}
			pt_en = 1;
		}break;
	}
	set_ptz_ptrack(ptz_track.point_no_bit, p_en, pt_en);
	return 0;
}
int ipcam_get_ptz_ptrack(SMsgAVIoctrlPTZPointTrackResp *p_track)
{
	if (NULL == p_track)
	{
		pr_error("in,\n");
		return -1;
	}
	return get_ptz_ptrack(p_track);
}

int PTZ_Init(void)
{
	int ret;

	ptz_autoturn = (unsigned char *)&(g_enviro_struct.ptz_autoturn_flag);

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
#ifndef USE_MOVE_TO_TRACK
int ivs_motion_tracker_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	pr_dbg("in\n");
	int ret = 0;
	motion_tracker_param_input_t param;

	printf("ivs_motion_tracker_start  start\n");
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
		return -1;
	}
	printf("ivs_motion_tracker_start  start end\n");
	return 0;
}
#endif
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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int get_motor_status()
{
     char buf[128];
 	 int p_fd=-1,sz=0;
     memset(buf, '\0', sizeof(buf));
    // get(PATH_PTZ_MOTOR_STATUS, buf);
     p_fd=open(PATH_PTZ_MOTOR_STATUS, (O_RDWR | O_CREAT), 0);
	 if(p_fd==-1)
	 	return 0;
 	 sz = read(p_fd,buf,128);
	 buf[sz]='\0';
	 //LOG("motor:%s+++++++++++++++++++\n",buf);
	 close(p_fd);
     if(strstr(buf,"1"))
	 	return 1;
	 else
	 	return 0;
	 	
}



#ifdef PATRACK
void *motion_tracker_thread(void *arg)
{
	IMPIVSInterface *inteface = NULL;
#ifndef USE_MOVE_TO_TRACK
	motion_tracker_param_output_t *result = NULL;
#else
	move_param_output_t *result = NULL;
#endif
	ptz_param_input_t ptz_param_info = {
		.ptz_point = {0},
		.pointdelt_ivs = {0},
		};

	int ret;
	int chn_num = 2;
////	static int patrack_cnt = 0;
#ifndef USE_MOVE_TO_TRACK
	ivs_motion_tracker_start(0, chn_num, &inteface);
#else
	ivs_move_start(0, 1, &inteface, 3);
#endif
	motion_tracker_param_input_t param;
	LOG("#### motion tracker start ...\n");
	
	prctl(PR_SET_NAME,"motion_tracker_thread");
	
	while(1)
	{
		if(!motion_tracker){
			usleep(500*1000);
			continue;
			}
		ret = IMP_IVS_PollingResult(chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			LOG("IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			continue;
		}


		ret = IMP_IVS_GetResult(chn_num, (void **)&result);
		if (ret < 0) {
			LOG( "IMP_IVS_GetResult(%d) failed\n", 0);
			continue;
		}
		// 非转动中
		//if(result->count>0)
			//printf("======================= track get it ==================\n");
		if ((result->count>0) &&
			 (!ptz_param_input.flag) &&
			 (get_ptz_operate_mutex(TYPE_ATRACK)))
		{
//		    pr_dbg("motion detection !\n");

            if( 0 == Getpositioninfo_fromivs(result,&ptz_param_info))
            {

    			pthread_mutex_lock(&ptz_mutex);
    			ptz_flag = false;
    			patrack_flag = false;
    			memcpy(&ptz_param_input , &ptz_param_info , sizeof(ptz_param_input_t));
    			ptz_param_input.flag = true;
    			pthread_cond_signal(&ptz_cond);
    			pthread_mutex_unlock(&ptz_mutex);
             }
		}
		#if 0
	    ret = IMP_IVS_GetParam(chn_num, &param);
        if (ret < 0){
           //IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", 0);
        return -1;
        }
		if(get_motor_status())
        param.is_motor_stop = 0;
		else
		param.is_motor_stop = 1;
		//printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~param.is_motor_stop:%d\n",param.is_motor_stop);
        param.is_feedback_motor_status = 1;


		ret = IMP_IVS_SetParam(chn_num, &param);
         if (ret < 0){
          // IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", 0);
           return -1;
         }
		#endif
         ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
         if (ret < 0) {
             LOG("IMP_IVS_ReleaseResult(%d) failed\n", 2);
             continue;
		}

	}

}

#endif
#define ONE_STEP 50

#ifdef PATRACK
int auto_run(void)
{//arun
	int steps = 0;
	static int direction;
	SetYSXCtl(YSX_CAMERA_MOVE, true);
#if defined (S301)
	//ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_LOW_SPEED);
#endif
	steps = ptz_set_steps_h(direction,ONE_STEP);
	if(steps < ONE_STEP && (*ptz_autoturn) == 1 )
	{
		LOG("## Change turn direction @@\n");
		direction = direction > 0 ? 0: 1;
	}
#if defined (S301)
//	pr_dbg("in ptz cmd %d, speed %d\n",ptz_cmd, ptz_get_speed(HORI_DIR_YSX));
#endif
	usleep(500*1000);
	SetYSXCtl(YSX_CAMERA_MOVE, false);
	usleep(1500*1000);
	pr_info(" in, aruning\n");
	return 0;
}

int auto_track(int *atrack_dtc_cnt)
{//atrack
	if (get_ptz_operate_flag(TYPE_PTRACK) == true)
		if (*atrack_dtc_cnt < P2ATRACK_DELAY){
//			pr_dbg(" in, atracking, dtc counting: %d\n", *atrack_dtc_cnt);
			(*atrack_dtc_cnt) ++;
			ptz_param_input.flag = false;
			return 0;
		}
	pr_info(" in, atracking\n");
	*atrack_dtc_cnt = 0;
	SetYSXCtl(YSX_CAMERA_MOVE, true);
#if defined (S301)
//	ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_MID_LOW_SPEED);
//	ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_MID_HIGH_SPEED);
	ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_HIGH_SPEED);
#endif
	pr_dbg("result: %d, %d\n",ptz_param_input.ptz_tracker_dir.rig_lef,
								ptz_param_input.ptz_tracker_delt.dx);
	if (RIG_DIR_YSX == ptz_param_input.ptz_tracker_dir.rig_lef){
		if (ptz_param_input.ptz_tracker_delt.dx > MAX_STEPS_X - get_steps_ysx(HORI_DIR_YSX)){
			ptz_param_input.ptz_tracker_delt.dx = MAX_STEPS_X - get_steps_ysx(HORI_DIR_YSX);
			pr_info("arrive max %d\n",get_steps_ysx(HORI_DIR_YSX));
		}
	}
	else if (LEF_DIR_YSX == ptz_param_input.ptz_tracker_dir.rig_lef){
		if (ptz_param_input.ptz_tracker_delt.dx > get_steps_ysx(HORI_DIR_YSX)){
			ptz_param_input.ptz_tracker_delt.dx = get_steps_ysx(HORI_DIR_YSX);
			pr_info("arrive min %d\n",get_steps_ysx(HORI_DIR_YSX));
		}
	}
	ptz_set_steps_h(ptz_param_input.ptz_tracker_dir.rig_lef,
					ptz_param_input.ptz_tracker_delt.dx);
//			usleep(500*1000);
#if defined (S301)
//	pr_dbg("in ptz cmd %d, speed %d\n",ptz_cmd, ptz_get_speed(HORI_DIR_YSX));
#endif
	SetYSXCtl(YSX_CAMERA_MOVE, false);
#if 0
	if (ptz_param_input.ptz_tracker_delt.dx > PTZ_DELAY_TIMES)
		usleep(ptz_param_input.ptz_tracker_delt.dx*1000);	//转动后画面稳定//Young, delay提升处理精度
	else
		usleep(PTZ_DELAY_TIMES*1000);
#endif
	ptz_param_input.flag = false;
	return 0;
}

void point_track(int *cir)
{//ptrack
//	int track_h_step = *h_step, track_v_step = *h_step;
	static int track_h_step[PTZ_POINT_NUM] = {0};
	static int track_v_step[PTZ_POINT_NUM] = {0};
	static int i_dir = 0, i_postion = 0;
	int i_cir = *cir;
	SetYSXCtl(YSX_CAMERA_MOVE, true);
	if (get_ptz_operate_flag(TYPE_PRE_PTRACK) == false)
	{
		ipcam_ptz_ptrack(track_h_step, track_v_step, &i_dir, &i_postion, &i_cir);
	}
//	pr_dbg(" in, ptracking:(%d,%d,%d)\n",i_postion, i_dir, i_cir);
//	pr_dbg(" in, i_postion:(%d,%d),next(%d,%d)\n",i_postion,i_dir,track_h_step[i_postion], track_v_step[i_postion]);
	ptz_goto_nextpoint(track_h_step[i_postion], track_v_step[i_postion]);
	usleep(500*1000);
	pr_info(" in, ptracking\n");
	if ((i_postion >= i_cir) || (i_postion <= 0))
		i_dir = !i_dir;
	if (i_dir == RIG_DIR_YSX)
		i_postion++;
	else
		i_postion--;
	SetYSXCtl(YSX_CAMERA_MOVE, false);
	*cir = i_cir;
//			sleep(PTRACK_DELAY);//16s
//			continue;
}
#endif

int ptz_move_hv(int cmd, bool flag)
{
	int steps = 0;
	if (cmd){
		set_ptz_operate_flag(TYPE_PRE_PTRACK, 0);
		flag = false;
	}
	SetYSXCtl(YSX_CAMERA_MOVE, true);
	switch(cmd)
	{//ptzmove
		case AVIOCTRL_PTZ_UP:
		{
		
			steps = ptz_set_steps_v(UP_DIR_YSX,MAX_STEPS_Y);
		}break;
		case AVIOCTRL_PTZ_DOWN:
		{
		  
			steps = ptz_set_steps_v(DOWN_DIR_YSX,MAX_STEPS_Y);
		}break;
		case AVIOCTRL_PTZ_LEFT:
		{
#if defined (S201A)
		//	ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_LOW_SPEED);
#endif
			steps = ptz_set_steps_h(RIG_DIR_YSX,MAX_STEPS_X);
		}break;
		case AVIOCTRL_PTZ_RIGHT:
		{
#if defined (S201A)
		//	ptz_set_speed(HORI_DIR_YSX, PTZ_MOTOR_LOW_SPEED);
#endif
			steps = ptz_set_steps_h(LEF_DIR_YSX,MAX_STEPS_X);
		}break;
		case AVIOCTRL_PTZ_STOP:
		{
//			pr_dbg("in\n");
			record_position(ptz_steps);
		}break;
		case AVIOCTRL_MOTOR_RESET_POSITION:
		{
			set_ptz_operate_flag(TYPE_AUTO_RUN, false);
			set_ptz_operate_flag(TYPE_PTZ_MOVE, false);
			set_ptz_operate_flag(TYPE_ATRACK, false);
			SetYSXCtl(YSX_PTRACK_TT, PTRACK_OFF);
			set_ptz_operate_flag(TYPE_PRE_PTRACK, false);
			set_ptz_operate_flag(TYPE_PTRACK, false);
			pr_info("clr all ptz %d, %d, %d, %d, %d, %d \n",
				get_ptz_operate_flag(TYPE_AUTO_RUN),
				get_ptz_operate_flag(TYPE_PTZ_MOVE),
				get_ptz_operate_flag(TYPE_ATRACK),
				get_ptz_operate_flag(TYPE_PRE_PTRACK),
				get_ptz_operate_flag(TYPE_PTRACK),
				GetYSXCtl(YSX_PTRACK_TT)
				);
			ptz_goto_nextpoint(GOLD_POS_H, GOLD_POS_V);
		}break;
		default:
			LOG("not support for ptz cmd %d\n",cmd);
			break;
	}
#if defined (S201A)
//	pr_dbg("in ptz cmd %d, speed (%d, %d)\n",cmd, ptz_get_speed(HORI_DIR_YSX), ptz_get_speed(VERT_DIR_YSX));
#endif
	SetYSXCtl(YSX_CAMERA_MOVE, false);
	return steps;
}

void *PTZ_thread(void *arg)
{
	if( PTZ_Init() != 0)
		return (void *)-1;
	prctl(PR_SET_NAME,"PTZ_thread");
	LOG("#### PTZ_thread start ...\n");

#if 1 //超时为侦测到移动，回到侦测前位置。
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned int cur_time = 0, last_time = tv.tv_sec;
	int step_delt = 0, dir = 0;
#endif
	int atrack_dtc_cnt = 0, i_cir = 0;
	int p_init_cnt = 0, p_init_time = 0;
	int last_point_h = 0, lpoint_flag = 0;
	// update move time;
	SetYSXCtl(YSX_CAMERA_MOVE, true);

#ifdef PATRACK
#if defined (S201A)
	if(!get_initboot_flag()){
		pr_info("tstreamer reboot in\n");
		last_point_h = Get_Point_from_conf(YSX_APOINT_RECORD_H, (int)RECORD_ATPOINT);
		if (last_point_h < 0)
			pr_info("in \n");
		lpoint_flag ++;
	}
	else{
		Set_Point_to_conf(YSX_APOINT_RECORD_H, (int)RECORD_ATPOINT, -1);
	}
#endif
	position_reset(&ptz_steps);
	Get_Motion_Conf();
#endif
	while(1)
	{
#ifdef PATRACK
		if (access(SD_INIT_FLAG,F_OK) == 0){
			position_reset(&ptz_steps);
		}
		if(get_ptz_operate_mutex(TYPE_AUTO_RUN)){
			/*自动巡航*/
			if (auto_run() == 0)
				continue;
		}

		pthread_mutex_lock(&ptz_mutex);
		while (patrack_flag){
//			pr_dbg("$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
			while (1){
//				pr_dbg("sd_insert_remove_dtc in ATRACK_DELAY, cnt: %d\n", p_init_cnt);
				gettimeofday(&now, NULL);
				outtime.tv_sec = now.tv_sec + 1;
				outtime.tv_nsec = now.tv_usec * 1000;
				pthread_cond_timedwait(&ptz_cond, &ptz_mutex, &outtime);
				if (access(SD_INIT_FLAG,F_OK) == 0){
					position_reset(&ptz_steps);
					break;
				}
				if ((p_init_cnt >= ATRACK_DELAY) || (ptz_flag == false)){
					break;
				}
				else{
					p_init_cnt ++;
				}
			}
			patrack_flag = false;
			p_init_cnt = 0;
		}
		patrack_flag = true;
		pthread_mutex_unlock(&ptz_mutex);
#if 1 //超时未侦测到移动，回到侦测前位置。
		gettimeofday(&tv, NULL);
		cur_time = tv.tv_sec;
		if((get_ptz_operate_mutex(TYPE_ATRACK == true)) &&
			(get_ptz_operate_flag(TYPE_PTRACK) == false)){
			if ((lpoint_flag != 0) && (last_point_h >= 0) &&
				(abs(cur_time - last_time) >= PTRACK_STEP_DELAY)){
				lpoint_flag = 0;
				last_time = cur_time;
				step_delt = get_steps_ysx(HORI_DIR_YSX)-last_point_h;
				dir = (step_delt < 0) ? RIG_DIR_YSX : LEF_DIR_YSX;
				SetYSXCtl(YSX_CAMERA_MOVE, true);
				ptz_set_steps_h(dir, abs(step_delt));
				SetYSXCtl(YSX_CAMERA_MOVE, false);
//				pr_dbg("dir %d, step %d\n", dir, abs(step_delt));
			}
		}
//		pr_dbg("time: %u, %u, flag: %d, point: %d\n",cur_time, last_time, lpoint_flag, last_point_h);
#endif
		if((ptz_param_input.flag) &&
			get_ptz_operate_mutex(TYPE_ATRACK)){
			/*移动追踪*/
			if ((lpoint_flag == 0) || (last_point_h < 0)){
				last_point_h = get_steps_ysx(HORI_DIR_YSX);
				Set_Point_to_conf(YSX_APOINT_RECORD_H, (int)RECORD_ATPOINT, last_point_h);
				lpoint_flag ++;
			}
#if 1 //侦测到移动，更新时间。
			last_time = cur_time;
#endif
			if (auto_track(&atrack_dtc_cnt) == 0)
				continue;
		}
		else{
			atrack_dtc_cnt = 0;
			ptz_param_input.flag = false;
		}

		if (get_ptz_operate_mutex(TYPE_PTRACK))
		{ 	/*定点巡航*/
//			pr_dbg("in\n");
			point_track(&i_cir);
		}
		else{
			if (motion_tracker == 0)
				i_cir = 0;
			set_ptz_operate_flag(TYPE_PRE_PTRACK, 0);
		}
#endif
		pthread_mutex_lock(&ptz_mutex);
		while (ptz_flag){
//			pr_dbg("@@@@@@@@@@@@@@@@@@@@@@@@\n");
			while (1){
//				pr_dbg("sd_insert_remove_dtc in PTRACK_DELAY, cnt: %d\n", p_init_cnt);
				if (get_ptz_operate_flag(TYPE_PTRACK) == true)
					p_init_time = PTRACK_DELAY-ATRACK_DELAY;
				else if (get_ptz_operate_flag(TYPE_ATRACK) == true)
					p_init_time = ATRACK_DELAY;
				else
					p_init_time = PTZ_DELAY;
				gettimeofday(&now, NULL);
				outtime.tv_sec = now.tv_sec + 1;
				outtime.tv_nsec = now.tv_usec * 1000;
				pthread_cond_timedwait(&ptz_cond, &ptz_mutex, &outtime);
				if (access(SD_INIT_FLAG,F_OK) == 0){
					position_reset(&ptz_steps);
					break;
				}
				if ((p_init_cnt >= p_init_time) || (ptz_flag == false)){
					break;
				}
				else{
					p_init_cnt ++;
				}
			}
			ptz_flag = false;
			p_init_cnt = 0;
		}
		ptz_flag = true;
		pthread_mutex_unlock(&ptz_mutex);

#ifdef PATRACK
		if (get_ptz_operate_mutex(TYPE_PMOVE)){
			/*定点移动*/
			SetYSXCtl(YSX_CAMERA_MOVE, true);
			ptz_goto_nextpoint(Get_Point_from_conf(YSX_POINT_CPOINT_H, get_ptz_point_flag()),
								Get_Point_from_conf(YSX_POINT_CPOINT_V, get_ptz_point_flag()));
			set_ptz_point_flag(0);
			SetYSXCtl(YSX_CAMERA_MOVE, false);
		}
#endif
		if (get_ptz_operate_mutex(TYPE_PTZ_MOVE)){
			/*云台操控*/
			ptz_move_hv(ptz_cmd, ptz_param_input.flag);
			atrack_dtc_cnt = 0;
			Set_Point_to_conf(YSX_APOINT_RECORD_H, (int)RECORD_ATPOINT, get_steps_ysx(HORI_DIR_YSX));
			lpoint_flag = 0;
			ptz_param_input.flag = false;
		}
	}
	LOG("PTZ_thread thread exit\n");
}

static unsigned int lst_cmd = 0;
void Ptz_Cmd(int SID, int avIndex, char *buf, int type, int iMediaNo)
{
	LOG("ptz cmd 0x%x\n",type);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int i_ret = 0;
	unsigned int cur_cmd = (tv.tv_sec*1000 + tv.tv_usec/1000);


	switch (type)
	{
		case IOTYPE_USER_IPCAM_PTZ_COMMAND:
		{
			SMsgAVIoctrlPtzCmd *p = (SMsgAVIoctrlPtzCmd*)buf;

			pthread_mutex_lock(&ptz_mutex);
			ptz_flag = false;
			ptz_cmd  = p->control;
#if 0
			if (p->control == AVIOCTRL_PTZ_UP)
				ptz_cmd  = p->control = AVIOCTRL_MOTOR_RESET_POSITION;
#endif
			switch(p->control)
			{
				case AVIOCTRL_PTZ_STOP:
				{
					LOG("AVIOCTRL_PTZ_STOP %d , %d\n",cur_cmd,lst_cmd);
					int diff = abs(cur_cmd - lst_cmd);
					if(diff <= ONE_TIME_STEPS)
						usleep(ONE_TIME_STEPS*1000);
					LOG("AVIOCTRL_PTZ_STOP now %d\n",diff);
#ifndef IOTYPE_USER_IPCAM_PTZ_AUTORUN_RESP
					if((*ptz_autoturn) == 1)
					{
						(*ptz_autoturn) = 0;
						SetYSXCtl(YSX_AUTOTURN, 0);
					}
#endif
					ptz_stop_turn(0);
				}break;
				case AVIOCTRL_PTZ_UP:
				{
					ptz_stop_turn(0);
					if(GetYSXCtl(YSX_INVERSION)) {
						ptz_cmd = AVIOCTRL_PTZ_DOWN;
					}
					LOG("AVIOCTRL_PTZ_UP\n");
				}break;
				case AVIOCTRL_PTZ_DOWN:
				{
					ptz_stop_turn(0);
					if(GetYSXCtl(YSX_INVERSION)) {
						ptz_cmd = AVIOCTRL_PTZ_UP;
					}
					LOG("AVIOCTRL_PTZ_DOWN\n");
				}break;
				case AVIOCTRL_PTZ_LEFT:
				{
					ptz_stop_turn(0);
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
					ptz_stop_turn(0);
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

#ifndef IOTYPE_USER_IPCAM_PTZ_AUTORUN_RESP
				case AVIOCTRL_PTZ_AUTO:
				{
					LOG("AVIOCTRL_PTZ_AUTO\n");
					if((*ptz_autoturn) == 0)
					{
						(*ptz_autoturn) = 1;
						SetYSXCtl(YSX_AUTOTURN, 1);
					}
				}break;
#endif
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
				case AVIOCTRL_MOTOR_RESET_POSITION:
				{
					ptz_stop_turn(0);
					//off ptrack ptrack
					off_ptz_switch(TYPE_AUTO_RUN);
					off_ptz_switch(TYPE_ATRACK);
					off_ptz_switch(TYPE_PTRACK);
					if (GetYSXCtl(YSX_PTRACK_TT) != PTRACK_OFF)
						SetYSXCtl(YSX_PTRACK_TT, PTRACK_OFF);
					LOG("AVIOCTRL_MOTOR_RESET_POSITION\n");
				}break;
				default:
					LOG("not support for ptz cmd %d\n",p->control);
				break;
			}
		if ((p->control == AVIOCTRL_PTZ_UP) || (p->control == AVIOCTRL_PTZ_DOWN) ||
			(p->control == AVIOCTRL_PTZ_LEFT) || (p->control == AVIOCTRL_PTZ_RIGHT) ||
			(p->control == AVIOCTRL_MOTOR_RESET_POSITION))
			set_ptz_operate_flag(TYPE_PTZ_MOVE, true);
		else
			set_ptz_operate_flag(TYPE_PTZ_MOVE, false);
		pthread_cond_signal(&ptz_cond);
		pthread_mutex_unlock(&ptz_mutex);
		lst_cmd = cur_cmd;
		}
		break;
#ifdef IOTYPE_USER_IPCAM_PTZ_AUTORUN_RESP
		/* 摇头机自动巡航
			success：result 0, autorun_en val
			fail:	result 1, autorun_en val
		*/
		case IOTYPE_USER_IPCAM_PTZ_SET_AUTORUN_REQ:
		{
			pthread_mutex_lock(&ptz_mutex);
			ptz_flag = false;
			SMsgAVIoctrlSetPTZAutoRunResp resp = {
					.autorun_en	= 0,
					.result 	= 0,
			};
			i_ret = set_ptz_arun_flag((SMsgAVIoctrlSetPTZAutoRunReq*)buf);
			if (!i_ret)
			{
				resp.result = 0;
				resp.autorun_en = get_ptz_operate_flag(TYPE_AUTO_RUN);
				SetYSXCtl(YSX_AUTOTURN, resp.autorun_en);
			}
			else
			{
				resp.result = 1;
				pr_error("ptz_cmd err\n");
			}
			pthread_cond_signal(&ptz_cond);
			pthread_mutex_unlock(&ptz_mutex);
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_AUTORUN_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetPTZAutoRunResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);
		}break;
		case IOTYPE_USER_IPCAM_PTZ_GET_AUTORUN_REQ:
		{
			SMsgAVIoctrlSetPTZAutoRunResp resp = {
					.autorun_en	= 0,
					.result 	= 0,
			};
			resp.autorun_en = get_ptz_operate_flag(TYPE_AUTO_RUN);
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_AUTORUN_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetPTZAutoRunResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);
		}break;
#endif
		/* 摇头机位置收藏点
			success：result 0, cpoint in no_bit
			fail:	result 1, cpoint 0
		*/
		case IOTYPE_USER_IPCAM_SET_PTZ_CPOINT_REQ:
		{
#ifdef APP_API_DBG
			SMsgAVIoctrlSetPTZSavePointReq *resp_s= (SMsgAVIoctrlSetPTZSavePointReq*)buf;
			pr_dbg("in IOTYPE_USER_IPCAM_SET_PTZ_CPOINT_REQ, var:(0x%x,%d)\n", resp_s->save_no, resp_s->idel);
#endif
			SMsgAVIoctrlPTZSavePointResp resp = {
					.save_no_bit 	= -1,
					.result			= 0,
			};
			if((!get_ptz_operate_flag(TYPE_AUTO_RUN)) &&
				(!get_ptz_operate_flag(TYPE_ATRACK)) &&
				(!get_ptz_operate_flag(TYPE_PTRACK)) &&
				(!get_ptz_operate_flag(TYPE_PTZ_MOVE)) &&
				(!get_ptz_operate_flag(TYPE_PMOVE)))
				resp.save_no_bit = ipcam_set_ptz_cpoint((SMsgAVIoctrlSetPTZSavePointReq*)buf);
			if (resp.save_no_bit != -1)
			{
				resp.result = 0;
				ipcam_get_ptz_cpoint(&resp);
			}
			else
			{
				resp.result = 1;
				pr_error("ptz_cmd err\n");
			}
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_CPOINT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPTZSavePointResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);
		}break;
		case IOTYPE_USER_IPCAM_GET_PTZ_CPOINT_REQ:
		{
#ifdef APP_API_DBG
			SMsgAVIoctrlPTZSavePointResp *resp_s = (SMsgAVIoctrlPTZSavePointResp*)buf;
			pr_dbg("in IOTYPE_USER_IPCAM_GET_PTZ_CPOINT_REQ, var:(0x%x,%d)\n", resp_s->save_no_bit, resp_s->result);
#endif
			SMsgAVIoctrlPTZSavePointResp resp = {
					.save_no_bit 	= 0,
					.result			= 0,
			};
			ipcam_get_ptz_cpoint(&resp);
#ifdef APP_API_DBG
			pr_dbg("in IOTYPE_USER_IPCAM_PTZ_CPOINT_RESP, var:(0x%x,%d)\n", resp.save_no_bit, resp.result);
#endif
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_CPOINT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPTZSavePointResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);
		}break;
		/* 摇头机移动追踪
			success：result 0, auto_track_en 1
			fail:	result 1, auto_track_en 0
		*/
		case IOTYPE_USER_IPCAM_SET_PTZ_ATRACK_REQ:
		{
			pthread_mutex_lock(&ptz_mutex);
			ptz_flag = false;
#ifdef APP_API_DBG
			SMsgAVIoctrlSetPTZAutoTrackReq *resp_s = (SMsgAVIoctrlSetPTZAutoTrackReq*)buf;
			pr_dbg("in IOTYPE_USER_IPCAM_SET_PTZ_ATRACK_REQ, var:(0x%x,%d)\n", resp_s->auto_track_en, resp_s->auto_track_en);
#endif
			SMsgAVIoctrlPTZAutoTrackResp resp = {
					.auto_track_en 	= 0,
					.result			= 0,
			};
			if (ipcam_set_ptz_atrack((SMsgAVIoctrlSetPTZAutoTrackReq*)buf) != -1)
			{
				ipcam_get_ptz_atrack(&resp);
				SetYSXCtl(YSX_ATRACK_EN, resp.auto_track_en);
				pr_dbg("in, var:(0x%x,%d)\n", resp.auto_track_en, resp.result);
			}
			else
			{
				resp.result = 1;
				pr_error("ptz_cmd err\n");
			}
			pthread_cond_signal(&ptz_cond);
			pthread_mutex_unlock(&ptz_mutex);
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_ATRACK_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPTZAutoTrackResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);

		}break;
		case IOTYPE_USER_IPCAM_GET_PTZ_ATRACK_STATUS_REQ:
		{
#ifdef APP_API_DBG
			SMsgAVIoctrlPTZAutoTrackResp *resp_s = (SMsgAVIoctrlPTZAutoTrackResp*)buf;
			pr_dbg("in IOTYPE_USER_IPCAM_GET_PTZ_ATRACK_STATUS_REQ, var:(0x%x,%d)\n", resp_s->auto_track_en, resp_s->result);
#endif
			SMsgAVIoctrlPTZAutoTrackResp resp;
			ipcam_get_ptz_atrack(&resp);
#ifdef APP_API_DBG
			pr_dbg("in IOTYPE_USER_IPCAM_PTZ_ATRACK_RESP, var:(0x%x,%d)\n", resp.auto_track_en, resp.result);
#endif
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_ATRACK_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPTZAutoTrackResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);

		}break;
		/* 摇头机定点巡航
			success：result 0, ptrack in no_bit
			fail:	result 1, ptrack 0
		*/
		case IOTYPE_USER_IPCAM_SET_PTZ_PTRACK_REQ:
		{
			pthread_mutex_lock(&ptz_mutex);
			ptz_flag = false;
			SMsgAVIoctrlSetPTZPointTrackReq *resp_s = (SMsgAVIoctrlSetPTZPointTrackReq*)buf;
#ifdef APP_API_DBG
			pr_dbg("in IOTYPE_USER_IPCAM_SET_PTZ_PTRACK_REQ, var:(0x%x,%d)\n", resp_s->point_no_bit, resp_s->point_no_bit);
#endif
			SMsgAVIoctrlPTZPointTrackResp resp = {
					.point_no_bit 	= 0,
					.result			= 0,
			};
			if (ipcam_set_ptz_ptrack((SMsgAVIoctrlSetPTZPointTrackReq*)buf) != -1)
			{
				int ptrack_en = 0;
				ptrack_en = ipcam_get_ptz_ptrack(&resp);
				SetYSXCtl(YSX_PTRACK_EN, ptrack_en);
				pr_dbg("in, var:(0x%x,%d,%d)\n", resp.point_no_bit, resp.result, ptrack_en);
#ifdef IOTYPE_USER_IPCAM_PTRACK_TIME_RESP
				if (get_ptz_point_flag() == 0){
					pr_dbg("in IOTYPE_USER_IPCAM_SET_PTRACK_TIME_REQ, var:(%d,%d,%d)\n", resp_s->ptrack_times, resp_s->time_start, resp_s->time_end);
					resp.ptrack_times = resp_s->ptrack_times;
					resp.time_start = resp_s->time_start;
					resp.time_end = resp_s->time_end;
					if ( (resp_s->time_start < 0) || (resp_s->time_end < 0) ||
						(resp_s->time_end > 24) || (resp_s->time_start>24 ))
					{
						pr_error("time range error\n");
	//					resp.result = RESP_FAIL;
					}
					else
					{
						SetYSXCtl(YSX_PTRACK_TT,resp_s->ptrack_times);
						SetYSXCtl(YSX_PTRACK_TS,resp_s->time_start);
						SetYSXCtl(YSX_PTRACK_TE,resp_s->time_end);
					}
				}
#endif
			}
			else
			{
				resp.result = 1;
				pr_error("ptz_cmd err\n");
			}
			pthread_cond_signal(&ptz_cond);
			pthread_mutex_unlock(&ptz_mutex);
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_PTRACK_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPTZPointTrackResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);

		}break;
		case IOTYPE_USER_IPCAM_GET_PTZ_PTRACK_REQ:
		{
#ifdef APP_API_DBG
			SMsgAVIoctrlPTZPointTrackResp *resp_s = (SMsgAVIoctrlPTZPointTrackResp*)buf;
			pr_dbg("in IOTYPE_USER_IPCAM_GET_PTZ_PTRACK_REQ, var:(0x%x,%d)\n", resp_s->point_no_bit, resp_s->result);
#ifdef IOTYPE_USER_IPCAM_PTRACK_TIME_RESP
			pr_dbg("in IOTYPE_USER_IPCAM_GET_PTZ_PTRACK_REQ, var:(%d,%d,%d)\n", resp_s->ptrack_times, resp_s->time_start, resp_s->time_end);
#endif
#endif
			SMsgAVIoctrlPTZPointTrackResp resp;
#ifdef IOTYPE_USER_IPCAM_PTRACK_TIME_RESP
			resp.ptrack_times = GetYSXCtl(YSX_PTRACK_TT);
			resp.time_start = GetYSXCtl(YSX_PTRACK_TS);
			resp.time_end = GetYSXCtl(YSX_PTRACK_TE);
			resp.status = ptrack_time_ok();
			pr_dbg("in IOTYPE_USER_IPCAM_GET_PTZ_PTRACK_REQ, var:(%d,%d,%d,%d)\n",
				resp.ptrack_times, resp.time_start, resp.time_end, resp.status);
#endif
			ipcam_get_ptz_ptrack(&resp);
#ifdef APP_API_DBG
			pr_dbg("in IOTYPE_USER_IPCAM_PTZ_PTRACK_RESP, var:(0x%x,%d)\n", resp.point_no_bit, resp.result);
#endif
			i_ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PTZ_PTRACK_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPTZPointTrackResp));
			if (i_ret != AV_ER_NoERROR)
				pr_error("in i_ret: %d\n", i_ret);
		}break;
		default:
			break;
	}

}
enum{
LEF_DIR = 0x0,
RIG_DIR = 0x1,
};
int get_limit_value(int value)
{
     char buf[128];
 	 int p_fd=-1,sz=0;
     memset(buf, '\0', sizeof(buf));
    // get(PATH_PTZ_MOTOR_STATUS, buf);
    if(value == LEF_DIR)
      p_fd=open(PATH_PTZ_MOTOR_LEFT_VALUE, (O_RDWR | O_CREAT), 0);
	else if(value == RIG_DIR)
	  p_fd=open(PATH_PTZ_MOTOR_RIGHT_VALUE, (O_RDWR | O_CREAT), 0);
	 if(p_fd==-1)
	 	return 0;
 	 sz = read(p_fd,buf,128);
	 buf[sz]='\0';
	 //LOG("motor:%s+++++++++++++++++++\n",buf);
	 close(p_fd);
     if(strstr(buf,"1"))
	 	return 1;
	 else
	 	return 0;
	 	
}



int ptz_set_steps_h(int direction, int steps)
{
	int ret_step = 0;
#ifdef STEP_RETURN_NOT_DOUBLE
//	pr_dbg("ptz_set_steps_h, var(%d, %d)\n", direction, steps);
	steps = steps/2;
#endif
	#if 1
	if ((steps == 0) ||
		((RIG_DIR_YSX == direction) && (ptz_steps.hori >= MAX_STEPS_X)) ||
		((LEF_DIR_YSX == direction) && (ptz_steps.hori <= MIN_STEPS_X)))
		return ret_step;
    #else

	if ((steps == 0) ||
		((RIG_DIR_YSX == direction) && (!get_limit_value(RIG_DIR))) ||
		((LEF_DIR_YSX == direction) && (!get_limit_value(LEF_DIR))))
		return ret_step;
	#endif
	
#if 0//defined (S201A) tstreamer may crash
	if(GetYSXCtl(YSX_VQUALITY) < 3)
	{
		if (QCamVideoInput_SetQualityLvl(0,4))
			pr_error("in\n");
		ret_step = 2*(ptz_set_steps_x(direction, steps));
		if (QCamVideoInput_SetQualityLvl(0,0))//MAIN_CHN, //3
			pr_error("in\n");
	}
	else{
		ret_step = 2*(ptz_set_steps_x(direction, steps));
	}
#else
	ret_step = 2*(ptz_set_steps_x(direction, steps));
#endif

	if (PTZ_STEPS_MOVE_ERR == ret_step)
	{
		pr_error("in\n");
		return ret_step;
	}
	if (RIG_DIR_YSX == direction)
		ptz_steps.hori += ret_step;
	else if (LEF_DIR_YSX == direction)
		ptz_steps.hori -= ret_step;
	if (MIN_STEPS_X > ptz_steps.hori)
		ptz_steps.hori = MIN_STEPS_X;
	else if (MAX_STEPS_X < ptz_steps.hori)
		ptz_steps.hori = MAX_STEPS_X;
//	pr_dbg("ptz_set_steps_h, out(%d, %d, %d)\n", ptz_steps.hori, ret_step, steps);
	record_position(ptz_steps);
	return ret_step;
}
int ptz_set_steps_v(int direction, int steps)
{
	int ret_step = 0;
	printf("~~~~~~~~~~~~~~~~~~~~~~direction :%d\n",direction);
#ifdef STEP_RETURN_NOT_DOUBLE
//	pr_dbg("ptz_set_steps_v, var(%d, %d)\n", direction, steps);
	steps = steps/2;
#endif
	if ((steps == 0) ||
		((UP_DIR_YSX == direction) && (ptz_steps.vert >= MAX_STEPS_Y)) ||
		((DOWN_DIR_YSX == direction) && (ptz_steps.vert <= MIN_STEPS_Y)))
		return ret_step;
#if 0//defined (S201A) tstreamer may crash
		if(GetYSXCtl(YSX_VQUALITY) < 3)
		{
			if (QCamVideoInput_SetQualityLvl(0,4))
				pr_error("in\n");
			ret_step = 2*(ptz_set_steps_y(direction, steps));
			if (QCamVideoInput_SetQualityLvl(0,0))
				pr_error("in\n");
		}
		else{
			ret_step = 2*(ptz_set_steps_y(direction, steps));
		}
#else
		ret_step = 2*(ptz_set_steps_y(direction, steps));
#endif
	if (PTZ_STEPS_MOVE_ERR == ret_step)
	{
		pr_error("in\n");
		return ret_step;
	}
	if (UP_DIR_YSX == direction)
		ptz_steps.vert += ret_step;
	else if (DOWN_DIR_YSX == direction)
		ptz_steps.vert -= ret_step;
	if (MIN_STEPS_Y > ptz_steps.vert)
		ptz_steps.vert = MIN_STEPS_Y;
	else if (MAX_STEPS_Y < ptz_steps.vert)
		ptz_steps.vert = MAX_STEPS_Y;
//	pr_dbg("ptz_set_steps_v, out(%d, %d, %d)\n", ptz_steps.vert, ret_step, steps);
	record_position(ptz_steps);
	return ret_step;
}

void position_reset(ptz_steps_t *ptz_pos)
{
	if (NULL == ptz_pos)
	{
		pr_error("in,\n");
		return;
	}
	int i_step_h_tmp = 0,i_step_v_tmp = 0;
	update_position(ptz_pos);
#if defined (S201A)
	if (access(SD_INIT_FLAG,F_OK) == 0)
		sleep(SDINITPOSITION_DELAY);
	if (get_initboot_flag() ||
		(access(SD_INIT_FLAG,F_OK) == 0))
#else
	if (get_initboot_flag())
#endif
	{//device reboot
//		pr_dbg("in\n");
		i_step_h_tmp = ptz_pos->hori;
		i_step_v_tmp = ptz_pos->vert;
		position_init();
		set_initboot_flag(false);
		ptz_set_steps_h(RIG_DIR_YSX, i_step_h_tmp);
		ptz_set_steps_v(UP_DIR_YSX, i_step_v_tmp);
//		record_position(*ptz_pos);//
	}
	if (access(SD_INIT_FLAG,F_OK) == 0)
		if (remove(SD_INIT_FLAG))
			pr_error("%s, delete failed\n", SD_INIT_FLAG);
//	pr_dbg(" in, step:(%d,%d), flag(%d,%d)\n", ptz_pos->hori, ptz_pos->vert,
//											 ptz_pos->dev_boot, ptz_pos->dev_boot);
}

void Get_Motion_Conf(void)
{
	*ptz_autoturn = Get_cfg_from_conf(YSX_AUTOTURN);
	motion_tracker = Get_cfg_from_conf(YSX_ATRACK_EN);
	set_ptz_ptrack(get_ptrack_nobit_from_conf(), 0, Get_cfg_from_conf(YSX_PTRACK_EN));
#ifdef IOTYPE_USER_IPCAM_PTRACK_TIME_RESP
	g_enviro_struct.ptrack_tt = Get_cfg_from_conf(YSX_PTRACK_TT);
	g_enviro_struct.ptrack_ts = Get_cfg_from_conf(YSX_PTRACK_TS);
	g_enviro_struct.ptrack_te = Get_cfg_from_conf(YSX_PTRACK_TE);
#endif
}
void Send_Motion_Sigal(int c_en)
{
	if (c_en == 0)
		return;
	pthread_mutex_lock(&ptz_mutex);
	ptz_flag = false;					//ptz go on when camera restart
	pthread_cond_signal(&ptz_cond);
	pthread_mutex_unlock(&ptz_mutex);
}
