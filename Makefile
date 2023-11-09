obj-m += pi_uart.o

all: module dt

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: echodev.dts
	dtc -@ -I dts -O dtb -o echodev.dtbo echodev.dts
	sudo cp echodev.dtbo /boot/overlays/
	sudo sed -i '$$a\dtoverlay=echodev.dtbo' /boot/config.txt
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf echodev.dtbo
cleandt:
	sudo rm /boot/overlays/echodev.dtbo || true
	sudo sed -i '/dtoverlay=echodev.dtbo/d' /boot/config.txt
