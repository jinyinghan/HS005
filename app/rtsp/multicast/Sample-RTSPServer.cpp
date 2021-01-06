/*
 * Ingenic IMP RTSPServer MAIN.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <PassiveServerMediaSubsession.hh>
#include <H264VideoRTPSink.hh>
#include <H264VideoStreamDiscreteFramer.hh>

#include <RCFServer.hh>
#include "Options.hh"
#include "VideoInput.hh"

#include "AudioServerMediaSubsession.hh"
#include "AudioInput.hh"

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

	VideoInput* videoInput1 = VideoInput::createNew(*env, 1);
	if (videoInput1 == NULL) {
		*env << "Video Input(Second) init failed\n";
		exit(1);
	}

	AudioInput* audioInput = AudioInput::createNew(*env, 0);
	if (audioInput == NULL) {
		*env << "Audio Input init failed\n";
		exit(1);
	}

	RTSPServer* rtspServer = RTSPServer::createNew(*env, rtspServerPortNum/*, authDB*/);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	// Create 'groupsocks' for RTP and RTCP:
	struct in_addr destinationAddress;
	destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
	// Note: This is a multicast address.  If you wish instead to stream
	// using unicast, then you should use the "testOnDemandRTSPServer"
	// test program - not this test program - as a model.

	const unsigned short rtpPortNum = 18888;
	const unsigned short rtcpPortNum = rtpPortNum+1;
	const unsigned char ttl = 255;

	const Port rtpPort(rtpPortNum);
	const Port rtcpPort(rtcpPortNum);
  
	Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
	rtpGroupsock.multicastSendOnly(); // we're a SSM source
	Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
	rtcpGroupsock.multicastSendOnly(); // we're a SSM source

	// Create a 'H264 Video RTP' sink from the RTP 'groupsock':
	OutPacketBuffer::maxSize = 500000;
	RTPSink* videoSinkMain = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);

	// Create (and start) a 'RTCP instance' for this RTP sink:
	const unsigned estimatedSessionBandwidth = 15000; // in kbps; for RTCP b/w share
	const unsigned maxCNAMElen = 100;
	unsigned char CNAME[maxCNAMElen+1];
	gethostname((char*)CNAME, maxCNAMElen);
	CNAME[maxCNAMElen] = '\0'; // just in case
	RTCPInstance* rtcpMain = RTCPInstance::createNew(*env, &rtcpGroupsock,
													 estimatedSessionBandwidth, CNAME,
													 videoSinkMain, NULL ,
													 True );

	ServerMediaSession* smsMain = ServerMediaSession::createNew(*env, "main", NULL,
																"Session streamed by \"testH264VideoStreamer\"",
																True );
	smsMain->addSubsession(AudioServerMediaSubsession::createNew(smsMain->envir(), *audioInput, audioSamplingFrequency_t*2));
	smsMain->addSubsession(PassiveServerMediaSubsession::createNew(*videoSinkMain, rtcpMain));
	rtspServer->addServerMediaSession(smsMain);

	char* url = rtspServer->rtspURL(smsMain);
	*env << "Play main stream using the URL \"" << url << "\"\n";
	delete[] url;

	/* Init second stream */
	// Create 'groupsocks' for RTP and RTCP ...Second:
	const unsigned short rtpPortNum1 = 18890;
	const unsigned short rtcpPortNum1 = rtpPortNum1+1;

	const Port rtpPort1(rtpPortNum1);
	const Port rtcpPort1(rtcpPortNum1);

	Groupsock rtpGroupsock1(*env, destinationAddress, rtpPort1, ttl);
	rtpGroupsock1.multicastSendOnly(); // we're a SSM source
	Groupsock rtcpGroupsock1(*env, destinationAddress, rtcpPort1, ttl);
	rtcpGroupsock1.multicastSendOnly(); // we're a SSM source

	RTPSink* videoSinkSecond = H264VideoRTPSink::createNew(*env, &rtpGroupsock1, 96);
	RTCPInstance* rtcpSecond = RTCPInstance::createNew(*env, &rtcpGroupsock1,
													   estimatedSessionBandwidth / 4, CNAME,
													   videoSinkSecond, NULL ,
													   True );

	ServerMediaSession* smsSecond = ServerMediaSession::createNew(*env, "second", NULL,
															"Session streamed by \"testH264VideoStreamer\"",
															True );

	smsSecond->addSubsession(PassiveServerMediaSubsession::createNew(*videoSinkSecond, rtcpSecond));
	rtspServer->addServerMediaSession(smsSecond);

	url = rtspServer->rtspURL(smsSecond);
	*env << "Play second stream using the URL \"" << url << "\"\n";
	delete[] url;

	*env << "Start Streaming from Camera...\n";
	H264VideoStreamDiscreteFramer* videoSourceMain = H264VideoStreamDiscreteFramer::createNew(*env, videoInput->videoSource());
	H264VideoStreamDiscreteFramer* videoSourceSecond = H264VideoStreamDiscreteFramer::createNew(*env, videoInput1->videoSource());

	videoSinkMain->startPlaying(*videoSourceMain, NULL, videoSinkMain);
	videoSinkSecond->startPlaying(*videoSourceSecond, NULL, videoSinkSecond);

	if(RCF_Server_Start() != 0) {
		*env << "RCF Server Start failed\n";
		exit(1);
	}

	env->taskScheduler().doEventLoop(); // does not return

	return 0; //only to prevent compiler warning
}
