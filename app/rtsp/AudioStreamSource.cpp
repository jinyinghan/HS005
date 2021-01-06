/*
 * Ingenic IMP RTSPServer AudioStreamSource
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>
#include <imp/imp_framesource.h>

#include "Options.hh"
#include "AudioStreamSource.hh"

void* PollingThread_a(void *p);

AudioStreamSource*
AudioStreamSource::createNew(UsageEnvironment& env, AudioInput& input)
{
	AudioStreamSource* newSource = new AudioStreamSource(env, input);

	return newSource;
}

AudioStreamSource::AudioStreamSource(UsageEnvironment& env, AudioInput& input)
	: FramedSource(env), eventTriggerId(0), fInput(input) {
	eventTriggerId = envir().taskScheduler().createEventTrigger(incomingDataHandler);

	sem_init(&asem, 0, 0);
	memset(&fPresentationTime,0,sizeof(struct timeval));
	int error;
	error = pthread_create(&polling_tid, NULL, PollingThread_a, this);
	if (error) {
		envir() << "PollingThread_a create error:" << strerror(errno);
	}

	if(fInput.streamOn()<0){
		printf("Stream on error !\n");
		exit(0);
	}
}

void* PollingThread_a(void *p) {
	AudioStreamSource *ass = (AudioStreamSource *)p;

	return ass->PollingThread1();
}

void* AudioStreamSource::PollingThread1() {
	int ret = -1;
	while (1) {
		sem_wait(&asem);

		int old_state, ret;
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
again:
		ret = fInput.pollingStream();
		if (ret < 0) {
			envir() << "Polling Timeout, try again:\n" << strerror(errno);
			goto again;
		}

		pthread_setcancelstate(old_state, NULL);

		if (eventTriggerId != 0) {
			envir().taskScheduler().triggerEvent(eventTriggerId, this);
		}
	}

	return NULL;
}

AudioStreamSource::~AudioStreamSource() {
	envir().taskScheduler().deleteEventTrigger(eventTriggerId);
	eventTriggerId = 0;

	pthread_cancel(polling_tid);
	pthread_join(polling_tid, NULL);
	sem_destroy(&asem);
	fInput.streamOff();
	fInput.fAudioSource = NULL;
}

void AudioStreamSource::incomingDataHandler(void* clientData) {
	((AudioStreamSource*)clientData)->incomingDataHandler1();
}

void AudioStreamSource::incomingDataHandler1() {
	fInput.getStream((void *)fTo, &fFrameSize, &fPresentationTime, fMaxSize);
	afterGetting(this);
}

void AudioStreamSource::doGetNextFrame() {
	sem_post(&asem);
}
