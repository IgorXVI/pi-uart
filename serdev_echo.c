#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/serdev.h>

/* Meta Information */
MODULE_LICENSE("GPL");

/* Declate the probe and remove functions */
static int serdev_echo_probe(struct serdev_device *serdev);
static void serdev_echo_remove(struct serdev_device *serdev);

static struct of_device_id serdev_echo_ids[] = {
	{
		.compatible = "brightlight,echodev"
	},

	{/* sentinel */}
};

MODULE_DEVICE_TABLE(of, serdev_echo_ids);

static struct serdev_device_driver serdev_echo_driver = {
	.probe = serdev_echo_probe,
	.remove = serdev_echo_remove,
	.driver = {
		.name = "serdev-echo",
		.of_match_table = serdev_echo_ids
	},
};

static int serdev_echo_receive(struct serdev_device *serdev, const unsigned char *buffer, size_t size) {
	printk("serdev_echo - received %ld bytes with '%s'\n", size, buffer);
	return size;
}

static const struct serdev_device_ops serdev_echo_ops = {
	.receive_buf = serdev_echo_receive
};

static int serdev_echo_probe(struct serdev_device *serdev) {
	int status;

	printk("serdev_echo - now im in the probe function!\n");

	serdev_device_set_client_ops(serdev, &serdev_echo_ops);

	status = serdev_device_open(serdev);

	if(status > 1) {
		printk("serdev_echo - error when opening the serial device!\n");
		return -1;
	}

	serdev_device_set_baudrate(serdev, 9600);

	serdev_device_set_flow_control(serdev, false);

	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);

	return 0;
}

static void serdev_echo_remove(struct serdev_device *serdev) {
	printk("serdev_echo - now im in the remove function!\n");
	serdev_device_close(serdev);
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("serdev_echo - Hello, Kernel!\n");

	if(serdev_device_driver_register(&serdev_echo_driver)) {
		printk("serdev_echo - could not load driver!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("serdev_echo - Goodbye, Kernel\n");

	serdev_device_driver_unregister(&serdev_echo_driver);
}

module_init(my_init);
module_exit(my_exit);