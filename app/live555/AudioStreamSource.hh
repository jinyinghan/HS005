/*
 * Ingenic IMP RTSPServer AudioStreamSource equal to AudioStreamFramer.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef H264VIDEOSTREAMSOURCE_HH
#define H264VIDEOSTREAMSOURCE_HH

#include <pthread.h>
#include <semaphore.h>
#include "FramedSource.hh"
#include "AudioInput.hh"

class AudioStreamSource: public FramedSource {
public:
  static AudioStreamSource* createNew(UsageEnvironment& env, AudioInput& input);
  void* PollingThread1();
  AudioStreamSource(UsageEnvironment& env, AudioInput& input);
  // called only by createNew()
  virtual ~AudioStreamSource();

public:
  EventTriggerId eventTriggerId;


private:
  static void incomingDataHandler(void* clientData);
  void incomingDataHandler1();
  virtual void doGetNextFrame();

private:
  pthread_t polling_tid;
  sem_t asem;
  AudioInput& fInput;

protected:
  //Boolean isAudioStreamFramer() const { return True; }
  //unsigned maxFrameSize()  const { return 150000; }
};

#endif

