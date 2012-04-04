obj-m += zzz.o
zzz-objs := fsprotection.o getsyscalltable.o syscallinterception.o printstring.o communicate.o utilities.o

KERNELBUILD :=/lib/modules/$(shell uname -r)/build

default:
	make -C $(KERNELBUILD) M=$(shell pwd) modules
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions
