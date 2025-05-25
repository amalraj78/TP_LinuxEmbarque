obj-m:=gpio-leds.o

KDIR :=/home/ensea/linux-socfpga
PWD := $(shell pwd)

ccflags-y := -fno-pic

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
install:
	make -C $(KDIR) M=$(PWD) install
