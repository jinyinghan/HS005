/*
 * Ingenic IMP RTSPServer subsession equal to AudioFileServerMediaSubsession.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <stdio.h>
#include "AudioServerMediaSubsession.hh"
#include "AudioRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "AudioInput.hh"
#include "AudioRTPCommon.hh"

AudioServerMediaSubsession*
AudioServerMediaSubsession
::createNew(UsageEnvironment& env, AudioInput& audioInput, unsigned estimatedBitrate) {
	return new AudioServerMediaSubsession(env, audioInput, estimatedBitrate);
}

AudioServerMediaSubsession
::AudioServerMediaSubsession(UsageEnvironment& env, AudioInput& audioInput, unsigned estimatedBitrate)
	: OnDemandServerMediaSubsession(env, True/*reuse the first source*/),
	  fAudioInput(audioInput) {
	fEstimatedKbps = (estimatedBitrate + 500)/1000;
}

AudioServerMediaSubsession
::~AudioServerMediaSubsession() {
}

FramedSource* AudioServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
	estBitrate = fEstimatedKbps;

	return createAudioSource(envir(), fAudioInput.audioSource());
}

RTPSink* AudioServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
	return createAudioRTPSink(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
