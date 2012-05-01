obj-m += zzz.o
zzz-objs := fsprotection.o getsyscalltable.o syscallinterception.o printstring.o communicate.o utilities.o notification.o

KERNELBUILD :=/lib/modules/$(shell uname -r)/build

default:
	make -C $(KERNELBUILD) M=$(shell pwd) modules
	gcc -o user user.c
	gcc -o complex complexuser.c
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions
