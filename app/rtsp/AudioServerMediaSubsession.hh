/*
 * Ingenic IMP RTSPServer subsession equal to H264VideoFileServerMediaSubsession.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef AUDIO_SERVER_MEDIA_SUBSESSION_HH
#define AUDIO_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ONDEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif
#include "AudioInput.hh"

class AudioServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
  static AudioServerMediaSubsession*
  createNew(UsageEnvironment& env, AudioInput& audioInput, unsigned estimatedBitrate);

protected:

  AudioServerMediaSubsession(UsageEnvironment& env, AudioInput& audioInput, unsigned estimatedBitrate);
      // called only by createNew()
  virtual ~AudioServerMediaSubsession();
  unsigned fEstimatedKbps;

protected:
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);

private:
  AudioInput& fAudioInput;
};

#endif
