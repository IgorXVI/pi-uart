obj-m += pi_uart.o

all: module dt
	echo Builded Device Tree Overlay and kernel module

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: pi_uart_overlay.dts
	dtc -@ -I dts -O dtb -o pi_uart_overlay.dtbo pi_uart_overlay.dts
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf pi_uart_overlay.dtbo
