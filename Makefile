obj-m += pi_uart.o

all: module dt
	echo Builded Device Tree Overlay and kernel module

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: pi_uart_overlay.dts
	dtc -@ -I dts -O dtb -o pi_uart_overlay.dtbo pi_uart_overlay.dts
	sudo rm /boot/firmware/overlays/pi_uart_overlay.dtbo || true
	sudo cp pi_uart_overlay.dtbo /boot/firmware/overlays/
	sudo sed -i '/dtoverlay=pi_uart_overlay.dtbo/d' /boot/config.txt
	sudo sed -i '$a \dtoverlay=pi_uart_overlay.dtbo' /boot/config.txt
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf pi_uart_overlay.dtbo
