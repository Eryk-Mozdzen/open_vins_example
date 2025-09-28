MODULE := mpu6050
KERNEL := /lib/modules/$(shell uname -r)/build
ARCH := arm64

obj-m := $(MODULE).o

all:
	$(MAKE) -C $(KERNEL) M=$(PWD) ARCH=$(ARCH) modules

clean:
	$(MAKE) -C $(KERNEL) M=$(PWD) clean
