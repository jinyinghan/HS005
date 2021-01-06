PWD				:= $(shell pwd)
TARGET			?= mips-linux-gnu
CROSS_COMPILE	?= $(TARGET)-
CC				:= $(CROSS_COMPILE)gcc
AR				:= $(CROSS_COMPILE)ar rcs

INSTALL_DIR ?= $(PWD)/_install
LIB_INS_DIR	:= $(INSTALL_DIR)/lib
INC_INS_DIR := $(INSTALL_DIR)/include

CFLAGS :=
CFLAGS += -Wall -g -O2 -muclibc
CFLAGS += -lpthread 
CFLAGS += -fPIC

LDFLAGS :=
LDFLAGS += -Wall -g -O2 -muclibc

objs := 
objs += ysx_gpio.o
objs += utils.o

targets := 
targets += libgpio.a
LIBGPIO_HEADFILE := 
LIBGPIO_HEADFILE += ysx_gpio.h

%.o: %.c
	$(CC) -c $^ $(CFLAGS)

.PHONY: all clean install distclean
all: $(targets)
libgpio.a: $(objs)
	$(AR) $@ $^ $(LIBGPIO)
	
install:
	if [ ! -d $(LIB_INS_DIR) ]; \
	then \
	install -d $(LIB_INS_DIR); \
	fi
	if [ ! -d $(INC_INS_DIR) ]; \
	then \
	install -d $(INC_INS_DIR); \
	fi
	install $(targets) $(LIB_INS_DIR)
	install $(LIBGPIO_HEADFILE) $(INC_INS_DIR)

clean:
	-rm -rf  $(targets) *.o $(LIBGPIO)

distclean: clean


