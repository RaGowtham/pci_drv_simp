obj-m:=pci.o
pci-objs := driver.o char.o

modules:
	make -C /lib/modules/`uname -r`/build M=`pwd` modules

clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean
