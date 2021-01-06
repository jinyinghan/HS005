/*
 * Ingenic IMP RTSPServer MAIN.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <BasicUsageEnvironment.hh>
#include "Options.hh"
#include "H264VideoServerMediaSubsession.hh"
#include "VideoInput.hh"
#include <signal.h>

#include "AudioServerMediaSubsession.hh"
#include "AudioInput.hh"

static RTSPServer* rtspServer = NULL;
UsageEnvironment* env;
TaskScheduler* scheduler;

void sig_fun(int sig)
{
	printf("********EXIT*******\n");
	Medium::close(rtspServer);
	
	env->reclaim();
	delete scheduler;	
	exit(0);
}
int main(int argc, char** argv) {

	signal(SIGTERM,sig_fun);
	signal(SIGINT,sig_fun);

	checkArgs(*env, argc, argv);
	
	// Begin by setting up our usage environment:
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	*env << "\nArgument print:\n";
	*env << "    " << "Main VideoWidth = " << gconf_Main_VideoWidth << "\n";
	*env << "    " << "Main VideoHeight = " << gconf_Main_VideoHeight << "\n";
	*env << "    " << "FPSNum = " << gconf_FPS_Num << "\n";
	*env << "    " << "FPSDen = " << gconf_FPS_Den << "\n";
	*env << "    " << "cli = " << gconf_cli << "\n";

	// Parse command-line options:
	*env << "Initializing...\n";

	VideoInput* videoInput = VideoInput::createNew(*env, 0);
	if (videoInput == NULL) {
		*env << "Video Input init failed\n";
		exit(1);
	}
#ifdef AUDIO_ENABLE
	AudioInput* audioInput = AudioInput::createNew(*env, 0);
	if (audioInput == NULL) {
		*env << "Audio Input init failed\n";
		exit(1);
	}
#endif	

	// Create the RTSP server:
	// Normal case: Streaming from a built-in RTSP server:
	rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, authDB);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	*env << "...done initializing\n";

	ServerMediaSession* sms_main =
		ServerMediaSession::createNew(*env, "main", NULL, streamDescription, False);

	sms_main->addSubsession(H264VideoServerMediaSubsession::createNew(sms_main->envir(), *videoInput, gconf_Main_VideoWidth * gconf_Main_VideoHeight * 3 / 2 + 128));
	rtspServer->addServerMediaSession(sms_main);
	char *url = rtspServer->rtspURL(sms_main);
	*env << "Play this video stream using the URL:\n\t" << url << "\n";
	delete[] url;

#ifdef AUDIO_ENABLE
	ServerMediaSession* sms_second =
		ServerMediaSession::createNew(*env, "audio", NULL, streamDescription, False);
	sms_second->addSubsession(AudioServerMediaSubsession::createNew(sms_main->envir(), *audioInput, audioSamplingFrequency_t*2));

	rtspServer->addServerMediaSession(sms_second);
	url = rtspServer->rtspURL(sms_second);
	*env << "Play this audio stream using the URL:\n\t" << url << "\n";
	delete[] url;
#endif	

	// Begin the LIVE555 event loop:
	env->taskScheduler().doEventLoop(); // does not return


	//exit 


	return 0; //only to prevent compiler warning
}
