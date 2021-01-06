PWD     		:= $(shell pwd)
TARGET      	= mips-linux-uclibc-gnu
CROSS_COMPILE   = $(TARGET)-
CPP     		:= $(CROSS_COMPILE)g++
CC      		:= $(CROSS_COMPILE)gcc
AR      		:= $(CROSS_COMPILE)ar
RANLIB      	:= $(CROSS_COMPILE)ranlib
STRIP	      	:= $(CROSS_COMPILE)strip --strip-unneeded

INSTALL_DIR		?= $(PWD)/../_install 
##INSTALL_BIN     ?= $(INSTALL_DIR)/bin
##INSTALL_BIN     ?= /home/lzf/T31/HS005/image/user/bin
INSTALL_BIN     ?= ../../image/user/bin
IPC_SDK 		?= $(PWD)/../../ipc_sdk
CAMERA_TYPE		?= 1



#CFLAGS     		+= -I./ -g -O2 -Wno-write-strings -Wall 
#CFLAGS          +=  -rdynamic
CFLAGS     		+= -I $(IPC_SDK)/include 
CFLAGS     		+= -I $(PWD)/include 
CFLAGS     		+= -I $(PWD)/VAVA/include
CFLAGS     		+= -DCURL_DEBUG 
#CFLAGS          += -DUSE_WDT


LDFLAGS			+= -L $(IPC_SDK)/lib  
LDFLAGS			+= -L $(PWD)/lib
LDFLAGS			+= -L $(IPC_SDK)/../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/MXU/lib/uclibc  
LDFLAGS			+= -L $(IPC_SDK)/../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/IVS/lib/uclibc  

LDFLAGS			+= -lm
LDFLAGS			+= -lpthread
LDFLAGS			+= -liniparser
LDFLAGS			+= -ljson
LDFLAGS			+= -lstdc++
LDFLAGS			+= -lcurl
LDFLAGS			+= -lcrypto
LDFLAGS			+= -lssl
LDFLAGS			+= -lmp4v2
LDFLAGS			+= -limp
LDFLAGS			+= -lsysutils
LDFLAGS			+= -lalog
LDFLAGS 		+= -lrt
LDFLAGS 		+= -lm
LDFLAGS			+= -lz
LDFLAGS			+= -lmotor
LDFLAGS			+= -lfdk-aac
LDFLAGS			+= -lysx_vid
LDFLAGS			+= -lysx_aud
LDFLAGS			+= -lysx_sys
LDFLAGS			+= -lPPCS_API
LDFLAGS			+= -lzbar
LDFLAGS			+= -lsvpush

LDFLAGS         += -lfigure_inf -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core


#LDFLAGS			+= -lmxu_imgproc
#LDFLAGS			+= -lmxu_objdetect
#LDFLAGS			+= -lmxu_core
#LDFLAGS			+= -lmxu_merge
#LDFLAGS			+= -lmxu_video
#LDFLAGS			+= -lmxu_contrib

OBJS            += ysx_curl_ssl.c
OBJS			+= log.c 
OBJS			+= guesthandle.c 
OBJS			+= conf.c 
OBJS			+= utils.c 
OBJS			+= sdrecord.c 
#OBJS			+= linklist.c
OBJS			+= aes.c 
OBJS			+= http_curl.c 
OBJS			+= play.c
OBJS			+= ota.c
OBJS			+= circular_buf.c
##OBJS			+= ysx_cloud.c
#OBJS			+= ysx_rtmp.c
OBJS			+= ez-setup.c 
#OBJS			+= network.c
###OBJS			+= motion_track.c
OBJS			+= media.c
##OBJS			+= ff_muxer.c
OBJS			+= ysx_data_buf.c
OBJS			+= audio_codec.c
OBJS			+= linklist.c
OBJS			+= memory_debug.c
OBJS			+= network_detection.c
OBJS			+= main.c
##OBJS			+= ptz_s201.c
OBJS			+= ysx_thread.c 
OBJS			+= common_env.c
OBJS			+= monitor.c
OBJS			+= ppcs_service.c
OBJS			+= ppcs_protocal.c
OBJS			+= ysx_h264_udp.c
OBJS			+= qr_code.c
OBJS			+= vava_cloud.c
OBJS			+= motion_detection.c
OBJS			+= vava_svpush.c


target += tstreamer 
#target += rperson 
#target += net_manager
#target += avapi_test

#target	+=led_ctrl

#avi_demuxer index_parse

.PHONY: all install clean
all: $(target) version
%.o:%.c
	$(CC) $(CFLAGS) $ -c $^

tstreamer: $(OBJS) $(STATIC_LIBS) 
	$(CC) $^ -I./ $(LDFLAGS) $(CFLAGS) $(MXU_LIBS) -o tstreamer
	$(STRIP) $@

rperson: sample-personDet.c $(STATIC_LIBS) 
	$(CC) $^ -I./ $(LDFLAGS) $(CFLAGS) $(MXU_LIBS) -o rperson 
	$(STRIP) $@

alexaCam:alexaIPcam.c $(STATIC_LIBS) 
	$(CC) $^ -I./ $(LDFLAGS) $(CFLAGS) $(MXU_LIBS) -o rperson 
	$(STRIP) $@

avapi_test: AVAPIs_Server.c $(STATIC_LIBS) 
	$(CC) $^ -I./ $(LDFLAGS) $(CFLAGS) $(MXU_LIBS) -o $@
	$(STRIP) $@

demuxer_test: demuxer_test.c log.c $(STATIC_LIBS) 
	$(CC) $^ -I./ $(LDFLAGS) $(CFLAGS) $(MXU_LIBS) -o $@
	$(STRIP) $@


net_manager: net_manager.c utils.c log.c
	$(CC) $^ -I./ $(CFLAGS) -o $@ -lpthread
	$(STRIP) $@

avi_demuxer: avi_demuxer.c
	$(CC) -O2 -Wall $^ -I./ $(CFLAGS) $(LDFLAGS) -o $@
	$(STRIP)

index_parse:index_parse.c
	$(CC) -O2 -Wall $^ -I./ $(CFLAGS) $(LDFLAGS) -o $@


LED_LDFLAGS = -L$(MIDDLEWARE)/_install/lib -lysx_sys

led_ctrl:led_ctrl.c
	$(CC) -O2 -Wall $^ -I./ $(CFLAGS) $(LED_LDFLAGS)  -o $@
	$(STRIP) led_ctrl


clean:
	rm -rf *.o *~ *.bak $(target)

install:
	install -c $(target) $(INSTALL_BIN)
	


	

.PHONY:all $(target) version

