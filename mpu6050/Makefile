
MODULE := mpu6050
KERNEL_SRC := /linux
ARCH := arm64
CROSS_COMPILE := aarch64-none-linux-gnu-

obj-m := $(MODULE).o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
