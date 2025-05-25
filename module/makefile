obj-m:=le_module.o module_param.o module_proc.o module_timer.o

KERNEL_SOURCE = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNEL_SOURCE) M=$(PWD) modules

clean:
	make -C $(KERNEL_SOURCE) M=$(PWD) clean

install:
	make -C $(KERNEL_SOURCE) M=$(PWD) install
