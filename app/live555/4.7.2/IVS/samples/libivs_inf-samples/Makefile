CONFIG_UCLIBC_BUILD=y
CUR_DIR=$(shell pwd)

#SDK_DIR ?= $(ISVP_ENV_RELEASE_DIR_SDK)
SDK_DIR ?= /home/haitao/workspace/newhome/ingenic_sdk/Ingenic-SDK-T21-1.0.30-20190430
SDK_INC_DIR = $(SDK_DIR)/include

MXU_DIR ?= $(CUR_DIR)/../../../MXU
IVS_DIR ?= $(CUR_DIR)/../../../IVS
IVS_INC_DIR = $(IVS_DIR)/include

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

INCLUDES = -I$(SDK_INC_DIR) -I$(IVS_INC_DIR)
CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2
LDFLAG = -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif

ifeq ($(CONFIG_UCLIBC_BUILD), y)
SDK_LIB_DIR = $(SDK_DIR)/lib/uclibc
MXU_LIB_DIR = $(MXU_DIR)/lib/uclibc
IVS_LIB_DIR = $(IVS_DIR)/lib/uclibc
else
SDK_LIB_DIR = $(SDK_DIR)/lib/glibc
MXU_LIB_DIR = $(MXU_DIR)/lib/glibc
IVS_LIB_DIR = $(IVS_DIR)/lib/glibc
endif

LIBS = $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a \
	   $(MXU_LIB_DIR)/libmxu_merge.a $(MXU_LIB_DIR)/libmxu_objdetect.a \
	   $(MXU_LIB_DIR)/libmxu_contrib.a $(MXU_LIB_DIR)/libmxu_video.a \
	   $(MXU_LIB_DIR)/libmxu_imgproc.a $(MXU_LIB_DIR)/libmxu_core.a

SAMPLES = sample-dynamic-tracker

all: $(SAMPLES)

sample-dynamic-tracker: sample-common.o sample-dynamic-tracker.o
	$(CC) -o $@ $^  $(IVS_LIB_DIR)/libdynamic_tracker_inf.a $(LIBS) $(LDFLAG)
	$(STRIP) $@

%.o:%.c sample-common.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(SAMPLES)
