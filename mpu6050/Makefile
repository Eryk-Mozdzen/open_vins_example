
KERNEL_SRC := /lib/modules/$(shell uname -r)/build
ARCH := arm64

obj-m := mpu6050.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) ARCH=$(ARCH) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
