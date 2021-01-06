PWD				:= $(shell pwd)
TARGET			?= mips-linux-gnu
CROSS_COMPILE	?= $(TARGET)-
CPP				:= $(CROSS_COMPILE)g++
CC				:= $(CROSS_COMPILE)gcc
AR				:= $(CROSS_COMPILE)ar rcs
RANLIB			:= $(CROSS_COMPILE)ranlib
LIB_INSTALL		?= $(PWD)/../../../ipc_sdk/lib 

USE_IVS=y
USE_MOTION=y

###USED_LIB_VID_VERSION="0.01.02.01_201030" 

CFLAGS :=
CFLAGS += -Wall  -O2 -muclibc -march=mips32r2
CFLAGS += -I $(PWD)/../../gpio/rootfs/include
CFLAGS += -I $(PWD)/../../../ipc_sdk/include/
ifeq (${USE_IVS},y)
CFLAGS += -DUSE_IVS_FREE
CFLAGS += -DUSE_VERITY

CFLAGS += -I $(PWD)/ivs/Ingenic-figure-t31-202008101600/4.7.2/IVS/include
CFLAGS += -I $(PWD)/ivs/Ingenic-figure-t31-202008101600/4.7.2/MXU/include
endif
CFLAGS += -I $(PWD)
CFLAGS += -DUSE_FHD
CFLAGS += -DUSE_HD
CFLAGS += -DUSE_VGA
CFLAGS += -DUSE_IRCUT
ifeq (${USE_MOTION},y)
CFLAGS += -DUSE_MOTION
endif


##CFLAGS += -DUSE_REDUCE_FPS_RIPPLE

ifeq (${YSX_PERSONDET},y)
CFLAGS          += -DYSX_PERSONDET                  #开人型识别
endif


LDFLAGS :=
LDFLAGS += -muclibc
LDFLAGS += -lpthread 
###LDFLAGS += -L $(PWD)/../../../ipc_sdk/lib/
LDFLAGS += -L ./lib_a 
ifeq (${USE_IVS},y)
LDFLAGS += -L $(PWD)/ivs/Ingenic-figure-t31-202008101600/4.7.2/IVS/lib/uclibc/
LDFLAGS += -L $(PWD)/ivs/Ingenic-figure-t31-202008101600/4.7.2/MXU/lib/uclibc/
LDFLAGS += -lfigure_inf -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core
endif
LDFLAGS += -limp  -lalog  -lsysutils -lrt -Wl,-gc-sections -lm -lstdc++#####-liaac -lverify #-lysx_utils #####-lpersonDet_inf -ljzdl 

LDFLAGS2 :=
LDFLAGS2 += -muclibc
LDFLAGS2 += -lpthread
LDFLAGS2 += -L ./lib_so 
ifeq (${USE_IVS},y)
LDFLAGS2 += -L $(PWD)/ivs/Ingenic-figure-t31-202008101600/4.7.2/IVS/lib/uclibc/
LDFLAGS2 += -L $(PWD)/ivs/Ingenic-figure-t31-202008101600/4.7.2/MXU/lib/uclibc/
LDFLAGS2 += -lfigure_inf -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core
endif
LDFLAGS2 += -limp  -lalog  -lsysutils -lrt -Wl,-gc-sections -lm#####-liaac -lverify #-lysx_utils #####-lpersonDet_inf -ljzdl 

SRC := 
SRC += ysx_video.c
SRC += utils.c
ifeq (${USE_MOTION},y)
SRC += ysx_motion.c
endif
SRC += ysx_osd.c
SRC += sample-common.c
SRC += ingenic_video.c
SRC += video_input.c
##SRC += ringfifo.c
##SRC += rtputils.c 
##SRC += rtspservice.c  
##SRC += rtsputils.c
SRC += ipc_ircut.c
SRC += ysx_verify.c 
ifeq (${USE_IVS},y)
SRC += ivs_figure.c
endif

objs := 
objs += ysx_video.o
objs += utils.o
ifeq (${USE_MOTION},y)
objs += ysx_motion.o
endif
objs += ysx_osd.o
objs += sample-common.o
objs += ingenic_video.o
objs += video_input.o
##objs += ringfifo.o
##objs += rtputils.o  
##objs += rtspservice.o  
##objs += rtsputils.o
objs += ipc_ircut.o
objs += ysx_verify.o 
ifeq (${USE_IVS},y)
objs += ivs_figure.o
endif

targets :=
#targets += libysx_vid.so
targets += libysx_vid.a
##targets += test_api 
##targets += test_list
##targets += test_motion
##targets += test_media_buf
##targets += rtsp_server
targets += test_main 
##targets += test_main_so 

LIB_GPIO := 
LIB_GPIO += $(PWD)/../../gpio/rootfs/lib/libgpio.a  
###LIB_GPIO += $(PWD)/../../../ipc_sdk/lib/libalog.a 

LIB_INGENIC := 
LIB_INGENIC += 

STATIC_LIBS :=
STATIC_LIBS += $(LIB_GPIO)

%.o: %.c
	$(CC) -c $^ $(CFLAGS) -fPIC

.PHONY: clean distclean
all: $(targets)
#libysx_vid.so: $(SRC) $(STATIC_LIBS) 
#	$(CC) $^ -o $@ -fPIC -shared $(LDFLAGS) $(CFLAGS)  
#$(CC) -fPIC -shared $(CFLAGS) $(LDFLAGS2) $^ -o $@ 

libysx_vid.a: $(objs) $(STATIC_LIBS)
	$(AR) $@ $^ 

test_main: main.c libysx_vid.a $(STATIC_LIBS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 
#test_main_so: main.c 
#	$(CC) $^ -o $@ $(LDFLAGS2) $(CFLAGS) -L . -lysx_vid -I include/  
test_api: list.c queue_buf.c test_api.c libysx_vid.a $(STATIC_LIBS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 
	
rtsp_server: rtsp_h265.c libysx_vid.a $(STATIC_LIBS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 


test: test.c
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 


test_list: list.c test_list.c libysx_vid.a $(STATIC_LIBS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 


test_motion: list.c test_motion.c libysx_vid.a $(STATIC_LIBS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 

test_media_buf: media_buf.c test_media_buf.c libysx_vid.a $(STATIC_LIBS)
	$(CC) $^ -o $@ $(LDFLAGS) $(CFLAGS) 

install:
	if [ ! -d $(LIB_INSTALL) ];then 	\
		mkdir -p $(LIB_INSTALL) ; 	\
	fi
#	$(STRIP) $(targets)
	install -c *.so $(LIB_INSTALL)

clean:
	rm -fr *.o $(targets)

distclean: clean



