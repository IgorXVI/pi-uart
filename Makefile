obj-m += serdev_echo.o

all: module dt
	echo Builded Device Tree Overlay and kernel module

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dt: serdevoverlay.dts
	dtc -@ -I dts -O dtb -o serdevoverlay.dtbo serdevoverlay.dts
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf serdevoverlay.dtbo