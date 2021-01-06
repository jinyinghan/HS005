PWD             := $(shell pwd)

CC=mips-linux-gnu-gcc
STRIP=mips-linux-gnu-strip

INSTALL_DIR     ?= $(PWD)/../_install
INSTALL_BIN     ?= ../../image/user/bin
IPC_SDK         ?= $(PWD)/../../ipc_sdk


C_FLAGS +=-muclibc
C_FLAGS += -I $(IPC_SDK)/include
C_FLAGS += -I $(PWD)/zbar
    
YSX_INC = $(MIDDLEWARE)/_install/lib

LDFLAGS :=
LDFLAGS += -L $(IPC_SDK)/lib
LDFLAGS += -lpthread -lstdc++ -limp  -lalog  -lrt  -lysx_vid -lysx_sys
LDFLAGS += -lsysutils -lz -lalog -lm
LDFLAGS += -L $(IPC_SDK)/../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/MXU/lib/uclibc
LDFLAGS += -L $(IPC_SDK)/../free-renxinrenlian/Ingenic-figure-t31-202008101600/4.7.2/IVS/lib/uclibc
LDFLAGS         += -lfigure_inf -lmxu_merge -lmxu_objdetect -lmxu_contrib -lmxu_video -lmxu_imgproc -lmxu_core

SRC += qr_code.c
SRC += ./libzbar.a

target ?=qr_scan

all: $(target)

qr_scan:$(SRC)
	$(CC) $^ $(C_FLAGS) $(LDFLAGS)  -o $@  
	$(STRIP) $@



install:
	if [ ! -d $(INSTALL_BIN) ] ;then \
		mkdir -p $(INSTALL_BIN);\
	fi
	install -c $(target) $(INSTALL_BIN)

clean:
	rm -fr *.o $(target)

.PHONY:clean	

