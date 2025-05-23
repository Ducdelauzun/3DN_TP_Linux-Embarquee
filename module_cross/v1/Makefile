obj-m += hello_module.o

KDIR := /home/ensea/linux-socfpga
PWD  := $(shell pwd)

CFLAGS_MODULE += -fno-pic

all:
	$(MAKE) -C $(KDIR) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$(PWD) clean
