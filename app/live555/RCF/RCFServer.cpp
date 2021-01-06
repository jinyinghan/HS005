#include <algorithm>
#include <iostream>
#include <string>
#include <pthread.h>
#include <dlfcn.h>

#include <RCF/RCF.hpp>

#include "RCFServer.hh"

#include "class/CarrierCommand.hh"

#define PORT 50001

static int rcf_run = 0;
static pthread_t rcf_server_thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static inline void RCF_Server_Wait()
{
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
}

static inline void RCF_Server_Post()
{
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

static void *RCF_Server_Thread(void *argv)
{
	RCF::RcfInitDeinit rcfInit;
	CarrierCommandImpl CarrierCommand;
    RCF::RcfServer server( RCF::TcpEndpoint("0.0.0.0", PORT) );
    server.bind<I_CarrierCommand>(CarrierCommand);
    server.start();

	if(rcf_run)
		RCF_Server_Wait();

	pthread_exit(0);
}

int RCF_Server_Start()
{
	int ret = -1;

	if(rcf_run == 1)
		return 0;

	rcf_run = 1;
	ret = pthread_create(&rcf_server_thread, NULL, RCF_Server_Thread, NULL);
	if(ret != 0) {
		printf("RCF Server thread cretate error\n");
		return ret;
	}

    return 0;
}

int RCF_Server_Stop()
{
	if(rcf_run == 0)
		return 0;

	rcf_run = 0;
	RCF_Server_Post();
	pthread_join(rcf_server_thread, NULL);
	return 0;
}
