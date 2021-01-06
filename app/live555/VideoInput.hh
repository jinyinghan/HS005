/*
 * Ingenic IMP RTSPServer VideoInput.hh
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __VIDEOINPUT_HH__
#define __VIDEOINPUT_HH__

using namespace std;

#include <list>

#include <pthread.h>
#include <MediaSink.hh>
#include <semaphore.h>
#include <imp/imp_osd.h>
#include <imp/imp_ivs.h>

#if 0
#include <ivs_common.h>
#include <ivs_inf_face.h>
#include <ivs_inf_figure.h>
#include <ivs_inf_line.h>
#include <ivs_inf_move.h>
#include <ivs_inf_perm.h>	
#include <ivs_inf_shade.h>
#include <ivs_interface.h>
#endif 

#include "linklist.h"
#define CHN_MAIN 1
#define MAX_VID_BUF 40

#define MAX_STREAM_CNT		2
#define MAX_IVS_OSD_REGION			25		/* 20 line and rect */
#define MAX_LINE_NUN			5
#define MAX_POINT_NUN			5
#define MAX_PERM_NUM			8


/* move is one switch */
#define MOVE_IVS_SWITCH	1
#define MOVE_ROI_SWITCH	2
#define MOVE_SDC_SWITCH	4
#define MOVE_ISV_SWITCH	8

class VideoInput: public Medium {
public:
  static VideoInput* createNew(UsageEnvironment& env, int streamNum = 0);
  FramedSource* videoSource();
  static void clear_line(void);
  static int insert_line(int x0, int y0, int x1, int y1);
  static int remove_line(int x0, int y0, int x1, int y1);

  int getStream(void* to, unsigned int* len, struct timeval* timestamp, unsigned fMaxSize);
  int pollingStream(void);
  int streamOn(void);
  int streamOff(void);
  int scheduleThread1(void);
  int ivsProcess1(void);
  static void *osdUpdateThread(void *p);
  static int SetResolution(int w, int h);
  static int setFrameSourceFormat(char *path, uint32_t format);

public:
  static pthread_t cmdTid;
  static pthread_t ispTuneTid;
  static Boolean fFontAvailable;
  static Boolean fCoverAvailable;
  static Boolean fPicAvailable;
  static Boolean fOsdAvailable;
  static Boolean fIVSAvailable;
  static Boolean fISVAvailable;
  static Boolean fpsIsOn[MAX_STREAM_CNT];
  static Boolean fontIsOn[MAX_STREAM_CNT];
  static Boolean coverIsOn[MAX_STREAM_CNT];
  static Boolean picIsOn[MAX_STREAM_CNT];
  static int bUseMixFigure;
  static Boolean IsOn3d[MAX_STREAM_CNT];
  static int permPoint[MAX_PERM_NUM];
  static int permLastTime;
  static uint32_t moveIvsOnBitmap;
  static Boolean roiIsOn;
  static Boolean sdcIsOn;
  static Boolean isvIsOn;

  static double gFps[MAX_STREAM_CNT];
  static double gBitRate[MAX_STREAM_CNT];

  static IMPRgnHandle fontRgnHandler;
  static IMPRgnHandle fontRgnHandler_sec;
  static IMPRgnHandle coverRgnHandler;
  static IMPRgnHandle picRgnHandler;
  static IMPRgnHandle picRgnHandler_sec;
  static int fontRgnThreadRefCnt;
  static int coverRgnThreadRefCnt;
  static int picRgnThreadRefCnt;
  static IMPRgnHandle ivsRgnHandler[MAX_IVS_OSD_REGION];
  static int ivsRgnindex;
  static int ivsRgnThreadRefCnt;
  static int ivsOsd0StartCnt;
  FramedSource* fVideoSource;
  static Boolean isvIsStart;
  static int	  isvfd;
  static Boolean b_dynamic_mode;

  static IMPCell secondLastPrev;

private:
  VideoInput(UsageEnvironment& env, int streamNum = 0);
  virtual ~VideoInput();
  static bool initialize(UsageEnvironment& env);
  static void *cmdListenThread(void *p);
  static void *ispAutoTuningThread(void *p);

private:
  static Boolean fHaveInitialized;
  Boolean fpsIsStart;
  Boolean fontIsStart;
  Boolean coverIsStart;
  Boolean picIsStart;
  Boolean osdIsStart;
  Boolean roiIsStart;
  Boolean sdcIsStart;
  int     osdStartCnt;
  IMPRect ivsFigureResultBuf[64];
  int	  ivsFigureResultID[64];
  int	  ivsFigureResultScore[64];
  int	  ivsFigureResultBufIndex;
  IMPRect ivsFaceResultBuf[64];
  int	  ivsFaceResultID[64];
  int	  ivsFaceResultScore[64];
  int	  ivsFaceResultBufIndex;
  unsigned int nrFrmFps;
  unsigned int totalLenFps;
  uint64_t startTimeFps;
  int streamNum;
  pthread_t scheduleTid;
  pthread_t ivsTid;
  int orgfrmRate;
  Boolean hasSkipFrame;
};
int system_osd_init()   ;
uint8_t *ysx_osd_image(char *filepath, uint32_t *w, uint32_t *h);
int set_osd_rgn(uint8_t grp_id, char *pic_path,int pic_x , int pic_y);
int set_osd_rect(uint8_t grp_id);


#endif
