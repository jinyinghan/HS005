PWD				:= $(shell pwd)
TARGET			?= mips-linux-gnu
CROSS_COMPILE	?= $(TARGET)-
CPP				:= $(CROSS_COMPILE)g++
CC				:= $(CROSS_COMPILE)gcc
AR				:= $(CROSS_COMPILE)ar rcs
RANLIB			:= $(CROSS_COMPILE)ranlib
LIB_INSTALL     ?= $(PWD)/../../../ipc_sdk/lib


CFLAGS += -Wall -O2 -muclibc -g
CFLAGS += -I $(QCAM_INC)
CFLAGS += -I $(YSX_INC)   
CFLAGS += -I ../../../ipc_sdk/include/
CFLAGS += -DD603_2=1	



LDFLAGS :=
LDFLAGS += -lpthread -lstdc++
##LDFLAGS += -L $(PWD)/../../../ipc_sdk/lib/
LDFLAGS += -L ./lib
LDFLAGS += -L .
LDFLAGS += -limp  -lalog  -lsysutils -lrt -Wl,-gc-sections -lm -ldl
###LDFLAGS += -limp  -lalog  -lsysutils  -lrt 

SRC =
SRC += ysx_audio.c

LIBS += $(PWD)/../../../ipc_sdk/lib/libimp.a
LIBS += $(PWD)/../../../ipc_sdk/lib/libalog.a
LIBS += $(PWD)/../../../ipc_sdk/lib/libsysutils.a


targets :=
targets += libysx_aud.a
targets += sample_record
#targets += ysx_sample_record
targets += sample_play
targets += libysx_aud.so
###########targets += deamonaudio_ysx
#targets += libysx_vid.a
targets += test_api
targets += sample_play_so 


objs := 
objs += ysx_audio.o

%.o : %.c
	$(CC) -c $^ $(CFLAGS) -fPIC

all: $(targets)
deamonaudio_ysx:deamonaudio_ysx.c
	$(CC) $^ -o $@ -lpthread -lrt -muclibc

audio_test:audio_test.c ysx_audio.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ 
	
ysx_sample_record:ysx_sample_record.c 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

sample_record:sample_record.c libysx_aud.a $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

ysx_sample_playback:ysx_sample_playback.c libysx_aud.a
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

sample_play:sample_play.c libysx_aud.a $(LIBS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ 
	
sample_play_so:sample_play.c  
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@  -lysx_aud 
sample_aec:sample_aec.c libysx_aud.a
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

libysx_aud.so:$(SRC)
	$(CC) -fPIC -shared $(CFLAGS) $(LDFLAGS) $^ -o $@	

libysx_aud.a: $(objs) 
	$(AR) $@ $^ 

test_api: test_api.o list.o libysx_aud.a $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ 
#	cp $@ /home/haitao/winshare/T300/mic_test

	
install:
	if [ ! -d $(LIB_INSTALL) ];then 	\
		mkdir -p $(LIB_INSTALL) ; 	\
	fi
	#$(STRIP) $(TARGET)
	install -c *.so $(LIB_INSTALL)
	
clean:
	rm -fr *.o  *.so $(TARGET)
