KDIR = /lib/modules/`uname -r`/build

kbuild:
	make -C $(KDIR) M=$(PWD)

clean:
	make -C $(KDIR) M=`pwd` clean
