PWD		:= $(shell pwd)
CC 		:= mips-linux-gnu-gcc
CPP 	:= mips-linux-gnu-g++
STRIP 	:= mips-linux-gnu-strip

#VPATH=./lib
VPATH	:= $(PWD)/../../middleware/_install/lib

ISUC = y
CFLAGS += -O2 -Wall -march=mips32r2 -lpthread -Wunused-variable -DAUDIO_ENABLE
#-DFHD -Wunused-variable 



CFLAGS 	+= -muclibc
LDFLAGS += -L$(PWD)/../../ipc_sdk/lib


SDK_DIR =../../
IVS_INC += $(SDK_DIR)/ivs_samples/Ingenic-IVS-1.0.5-20161014/include/ivs/

INSTALL_DIR = $(APP_INSTALL)/bin

###CFLAGS += -I$(IVS_INC)
CFLAGS += -I$(SDK_DIR)include    
CFLAGS += -I$(PWD)/../../ipc_sdk/include    
CFLAGS += -I$(PWD)/../../ipc_sdk/include/ivs


LIB_DIR = lib

CFLAGS += -I live555/BasicUsageEnvironment/include 
CFLAGS += -I live555/groupsock/include 
CFLAGS += -I live555/liveMedia/include 
CFLAGS += -I live555/UsageEnvironment/include 
CFLAGS += -I $(PWD)

SRC =$(LIB_DIR)/libBasicUsageEnvironment.a
SRC +=$(LIB_DIR)/libliveMedia.a
SRC +=$(LIB_DIR)/libgroupsock.a
SRC +=$(LIB_DIR)/libUsageEnvironment.a



SRC += Sample-unicast-RTSPServer.cpp   
SRC +=H264VideoServerMediaSubsession.cpp
SRC +=Options.cpp    
SRC +=VideoInput.cpp  
SRC +=H264VideoStreamSource.cpp
SRC +=ysx_osd.c
SRC +=linklist.c


SRC +=AudioInput.cpp
SRC +=AudioStreamSource.cpp
SRC +=AudioServerMediaSubsession.cpp
SRC +=AudioRTPCommon.cpp 
LDFLAGS += -lysx_vid -limp  -lsysutils  -lalog  -lrt -lysx_vid -lysx_sys -lz -lm -ldl

#LDFLAGS += -L./lib	-lface_inf -lfigure_inf -lmove_inf -lperm_inf -lshade_inf   -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core


target = ysx_rtsp_server


all:$(target)


%.o : %.cpp
	$(CPP) -c $(CFLAGS) $< $(LDFLAGS)


ysx_rtsp_server:depends 
	$(CPP)  -o $@   $(SRC) $(CFLAGS) $(LDFLAGS) 
	$(STRIP) $@
#	cp ysx_rtsp_server  /home/haitao/winshare/T300

depends:
	if [ ! -d $(LIB_DIR) ];then \
        mkdir -p $(LIB_DIR);\
    fi
	make -C live555/liveMedia UCLIBC=$(ISUC)
	cp -av live555/liveMedia/libliveMedia.a $(LIB_DIR)
	make -C live555/BasicUsageEnvironment UCLIBC=$(ISUC)
	cp -av live555/BasicUsageEnvironment/libBasicUsageEnvironment.a $(LIB_DIR)
	make -C live555/groupsock UCLIBC=$(ISUC)
	cp -av live555/groupsock/libgroupsock.a $(LIB_DIR)
	make -C live555/UsageEnvironment  UCLIBC=$(ISUC)
	cp -av live555/UsageEnvironment/libUsageEnvironment.a $(LIB_DIR)

install:
	install -c $(target) $(INSTALL_DIR)
clean:
	rm -fr $(target)
