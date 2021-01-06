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
CFLAGS += -I $(PWD)/..

LDFLAGS :=
LDFLAGS += -Wall -g -O2 -muclibc

LIB_GPIO := $(PWD)/../libgpio.a

objs := 
objs += test.o

targets := 
targets += gpio_test

%.o: %.c
	$(CC) -c $^ $(CFLAGS)

.PHONY: all clean install distclean
all: $(targets)
gpio_test: $(objs) $(LIB_GPIO)
	$(CC) $^ -o $@ $(LDFLAGS)
	
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


