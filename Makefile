obj-m += pi_uart.o

all: module dt

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: echodev.dts
	dtc -@ -I dts -O dtb -o echodev.dtbo echodev.dts
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf echodev.dtbo
