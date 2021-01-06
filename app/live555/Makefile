include Makefile.param
PWD := $(shell pwd)
SDK_DIR=$(PWD)/../../
CC = mips-linux-gnu-gcc
CPP = mips-linux-gnu-g++
STRIP = mips-linux-gnu-strip

VPATH=./lib

ISUC = y
CFLAGS += -g -O2 -Wall -march=mips32r2 -lpthread -DFHD 
#CFLAGS += -DAUDIO_ENABLE
CFLAGS += -muclibc

####IVS_INC += $(SDK_DIR)/ivs_samples/Ingenic-IVS-1.0.5-20161014/include/ivs/

INSTALL_DIR = $(PWD)/bin

#####3CFLAGS += -I$(IVS_INC)
CFLAGS += -I$(SDK_DIR)/ipc_sdk/include    
####CFLAGS += -I$(SDK_DIR)/middleware/_install/include    

LIB_DIR = lib

CFLAGS += -I live555/BasicUsageEnvironment/include 
CFLAGS += -I live555/groupsock/include 
CFLAGS += -I live555/liveMedia/include 
CFLAGS += -I live555/UsageEnvironment/include 
CFLAGS += -I $(PWD)/../../ipc_sdk/include

SRC =$(LIB_DIR)/libBasicUsageEnvironment.a
SRC +=$(LIB_DIR)/libliveMedia.a
SRC +=$(LIB_DIR)/libgroupsock.a
SRC +=$(LIB_DIR)/libUsageEnvironment.a
#SRC +=./libdynamic_tracker_inf.a
#SRC +=./4.7.2/MXU/lib/uclibc/libmxu_contrib.a
#SRC +=./4.7.2/MXU/lib/uclibc/libmxu_core.a
#SRC +=./4.7.2/MXU/lib/uclibc/libmxu_imgproc.a
#SRC +=./4.7.2/MXU/lib/uclibc/libmxu_merge.a
#SRC +=./4.7.2/MXU/lib/uclibc/libmxu_objdetect.a 
#SRC +=./4.7.2/MXU/lib/uclibc/libmxu_video.a 




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

LDFLAGS += -muclibc
#LDFLAGS += -L $(SDK_DIR)/lib/uclibc
LDFLAGS += -L $(PWD)/../../ipc_sdk/lib
LDFLAGS += -L ./ 
TEST_IVS=n
ifeq (${TEST_IVS},y)
CFLAGS += -I $(PWD)/../../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/MXU/include
CFLAGS += -I $(PWD)/../../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/IVS/include
LDFLAGS += -L $(PWD)/../../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/MXU/lib/uclibc/
LDFLAGS += -L $(PWD)/../../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/IVS/lib/uclibc/ 
LDFLAGS += -lfigure_inf -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core
endif
###LDFLAGS += -L ./4.7.2/MXU/lib/uclibc/ 
#LDFLAGS += $(LIB) -lysx_vid -limp  -lsysutils  
LDFLAGS += -lysx_vid -limp  -lsysutils -lysx_sys###########-ldynamic_tracker_inf 
LDFLAGS += -pthread -lalog -ldl -lrt  -lz
####LDFLAGS += -lmxu_contrib -lmxu_core -lmxu_imgproc -lmxu_merge -lmxu_objdetect -lmxu_video
#LDFLAGS += -ldynamic_tracker_inf

#LDFLAGS += -L./lib	-lface_inf -lfigure_inf -lmove_inf -lperm_inf -lshade_inf   -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core


targets := 
targets += rtsp_server


all:$(targets)
%.o : %.cpp
	$(CPP) -c $(CFLAGS) $< $(LDFLAGS)


rtsp_server:depends 
	$(CPP)  -o $@   $(SRC) $(CFLAGS) $(LDFLAGS) 
	cp rtsp_server  /home/lzf/nfs
	
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
