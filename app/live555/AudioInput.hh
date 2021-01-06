/*
 * Ingenic IMP RTSPServer AudioInput.hh
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __AUDIOINPUT_HH__
#define __AUDIOINPUT_HH__

using namespace std;

#include <pthread.h>
#include <MediaSink.hh>
#include <semaphore.h>

class AudioInput: public Medium {
public:
  static AudioInput* createNew(UsageEnvironment& env, int streamNum = 0);
  FramedSource* audioSource();

  int getStream(void* to, unsigned int* len, struct timeval* timestamp, unsigned fMaxSize);//audio
  int pollingStream(void);//audio
  int streamOn(void);//audio
  int streamOff(void);//audio
  static void streamRecv(void);//audio
  int scheduleThread1(void);//audio
  static int chnID;
  static int devID;
  static int devIDr;
  static int ptfunc;

public:

  FramedSource* fAudioSource;
private:
  AudioInput(UsageEnvironment& env, int streamNum = 0);
  virtual ~AudioInput();
  static bool initialize(UsageEnvironment& env);//audio

private:
  static Boolean fHaveInitialized;//audio
  int streamNum;
  pthread_t scheduleTid_a;//audio
};

#endif
