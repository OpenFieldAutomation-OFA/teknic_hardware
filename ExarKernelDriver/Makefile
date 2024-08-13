# Note: This makefile has been modified to suppress build warnings because they
# are irrelevant to most users.
#
# If you plan on modifying this driver, or are otherwise interested in seeing
# the warnings, remove the -w flag from EXTRA_CFLAGS.
obj-m := xr_usb_serial_common.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

EXTRA_CFLAGS	:= -DDEBUG=0 -w

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions vtty
