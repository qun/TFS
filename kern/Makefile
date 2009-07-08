ifneq ($(KERNELRELEASE),)
	obj-m := tfs.o
	tfs-objs := super.o file.o dir.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

