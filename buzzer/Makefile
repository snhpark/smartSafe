KERNELDIR = /lib/modules/$(shell uname -r)/build

obj-m := buzzer_driver.o

PWD := $(shell pwd)

default : 
	make -C $(KERNELDIR) M=$(PWD) modules

clean : 
	make -C $(KERNELDIR) M=$(PWD) clean