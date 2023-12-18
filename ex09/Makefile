obj-m += mymounts.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

debug:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules CFLAGS_$(obj-m)=-DDEBUG

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
