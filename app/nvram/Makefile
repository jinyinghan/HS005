PWD     		:= $(shell pwd)
TARGET      	?= mips-linux-gnu
CROSS_COMPILE   ?= $(TARGET)-
CPP     		:= $(CROSS_COMPILE)g++
CC      		:= $(CROSS_COMPILE)gcc
AR      		:= $(CROSS_COMPILE)ar
RANLIB      	:= $(CROSS_COMPILE)ranlib
STRIP	      	:= $(CROSS_COMPILE)strip --strip-unneeded
CP				:= cp -rf
SDK_DIR 		?= $(PWD)
INSTALL_DIR 	:= ${INS_DIR}

CFLAGS :=
CFLAGS += -muclibc
CFLAGS += -Wall -g -O2 
CFLAGS += -I $(PWD)
CFLAGS += -I $(SDK_DIR)

LDFLAGS :=
LDFLAGS += -muclibc

OBJS :=
OBJS += .o

%.o: %.c
	$(CC) -c $^ $(CFLAGS)

targets :=
targets += nvram_get
targets += nvram_set

.PHONY: all clean
all: $(targets)
nvram_get: nvram_get.o 
	$(CC) $^ -o $@ $(LDFLAGS)
	cp $@ /home/lzf/winshare/S214/

nvram_set: nvram_set.o 
	$(CC) $^ -o $@ $(LDFLAGS)
	cp $@ /home/lzf/winshare/S214/

clean:
	rm *.o
	rm $(targets) -rf

install:
	if [ ! -d $(INSTALL_DIR)/bin ] ;then \
		mkdir -p $(INSTALL_DIR)/bin ;\
	fi
	install -c $(targets) $(INSTALL_DIR)/bin







