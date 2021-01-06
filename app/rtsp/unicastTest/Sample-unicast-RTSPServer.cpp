/*
 * Ingenic IMP RTSPServer MAIN.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <BasicUsageEnvironment.hh>
#include <RCFServer.hh>
#include "Options.hh"
#include "H264VideoServerMediaSubsession.hh"
#include "VideoInput.hh"

#include "AudioServerMediaSubsession.hh"
#include "AudioInput.hh"
#define AUDIO_ENABLE

int main(int argc, char** argv) {
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	// Print an introduction message:
	*env << "sample-RTSPServer: An RTSP server for Ingenic libimp\n";
	*env << "Built using \"LIVE555 Streaming Media\": <http://www.live555.com/liveMedia/>\n";
	*env << "Built time: " << __DATE__ << ", " <<__TIME__ << "\n";

	checkArgs(*env, argc, argv);
	*env << "\nArgument print:\n";
	*env << "    " << "Main VideoWidth = " << gconf_Main_VideoWidth << "\n";
	*env << "    " << "Main VideoHeight = " << gconf_Main_VideoHeight << "\n";
	*env << "    " << "Second VideoWidth = " << gconf_Second_EncWidth << "\n";
	*env << "    " << "Second VideoHeight = " << gconf_Second_EncHeight << "\n";
	*env << "    " << "FPSNum = " << gconf_FPS_Num << "\n";
	*env << "    " << "FPSDen = " << gconf_FPS_Den << "\n";
	*env << "    " << "cli = " << gconf_cli << "\n";

	// Parse command-line options:
	*env << "\nFunction usage:\n";
	*env << "\tshowfps: Show FPS and bitrate infomation\n";
	*env << "\tosd: Switch on/off timestamp OSD\n";
	*env << "\tsnap: Snapshot a JPEG picture\n";
	*env << "\tdisplay: Display on LCD\n";

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
	RTSPServer* rtspServer = NULL;
	// Normal case: Streaming from a built-in RTSP server:
	rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, authDB);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	*env << "...done initializing\n";

	ServerMediaSession* sms_main =
		ServerMediaSession::createNew(*env, "main", NULL, streamDescription, False);

#ifdef AUDIO_ENABLE
	sms_main->addSubsession(AudioServerMediaSubsession::createNew(sms_main->envir(), *audioInput, audioSamplingFrequency_t*2));
#endif
	sms_main->addSubsession(H264VideoServerMediaSubsession::createNew(sms_main->envir(), *videoInput, gconf_Main_VideoWidth * gconf_Main_VideoHeight * 3 / 2 + 128));

	rtspServer->addServerMediaSession(sms_main);
	char *url = rtspServer->rtspURL(sms_main);
	*env << "Play this video stream using the URL:\n\t" << url << "\n";
	delete[] url;

	VideoInput* videoInput1 = VideoInput::createNew(*env, 1);
	if (videoInput1 == NULL) {
		*env << "Video Input init failed\n";
		exit(1);
	}

	ServerMediaSession* sms_second =
		ServerMediaSession::createNew(*env, "second", NULL, streamDescription, False);

	sms_second->addSubsession(H264VideoServerMediaSubsession::createNew(sms_second->envir(), *videoInput1, gconf_Main_VideoWidth * gconf_Main_VideoHeight * 3 / 2 + 128));

	rtspServer->addServerMediaSession(sms_second);
	url = rtspServer->rtspURL(sms_second);
	*env << "Play this video stream using the URL:\n\t" << url << "\n";
	delete[] url;

	if(RCF_Server_Start() != 0) {
		*env << "RCF Server Start failed\n";
		exit(1);
	}

	// Begin the LIVE555 event loop:
	env->taskScheduler().doEventLoop(); // does not return

	return 0; //only to prevent compiler warning
}
