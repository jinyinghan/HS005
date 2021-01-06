CROSS_COMPILE ?= mips-linux-gnu-

ISVP_ENV_KERNEL_DIR = /home/lzf/T31/AP6PCT00/opensource/kernel 
ifeq (${ISVP_ENV_KERNEL_DIR}, )
    ISVP_ENV_KERNEL_DIR = ../../../kernel
endif

KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := uln2803
.PHONY: modules clean
all: modules
$(MODULE_NAME)-objs := motor.o
obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@
	cp uln2803.ko /home/lzf/nfs
		
install:
	 cp -av *.ko $(DRIVERS) 
clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
