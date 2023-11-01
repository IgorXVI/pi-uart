#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/serdev.h>
#include <linux/proc_fs.h>

/* Meta Information */
MODULE_LICENSE("GPL");

/* Buffer for data */
static char global_buffer[255];
static int global_buffer_head = 0;
static int global_buffer_size = 0;

static struct proc_dir_entry *proc_file;

static ssize_t proc_read(struct file *file_pointer, char *user_buffer, size_t count, loff_t *offset) {
	printk("pi_uart - file /proc/pi-uart-data - read was called!\n");

	if (*offset >= global_buffer_size || copy_to_user(user_buffer, global_buffer, global_buffer_size)) {
		printk("pi_uart - file /proc/pi-uart-data - copy_to_user failed\n");
		return 0;
	} else {
		*offset += global_buffer_size;
	}

	return global_buffer_size;
}

static struct proc_ops pi_uart_proc_fops = {
	.proc_read = proc_read
};

/* Declate the probe and remove functions */
static int pi_uart_probe(struct serdev_device *serdev);
static void pi_uart_remove(struct serdev_device *serdev);

static struct of_device_id pi_uart_ids[] = {
	{
		.compatible = "brightlight,piuartdev"
	},

	{/* sentinel */}
};

MODULE_DEVICE_TABLE(of, pi_uart_ids);

static struct serdev_device_driver pi_uart_driver = {
	.probe = pi_uart_probe,
	.remove = pi_uart_remove,
	.driver = {
		.name = "pi-uart",
		.of_match_table = pi_uart_ids
	},
};

static int receive_data_by_uart_and_save_last_byte(struct serdev_device *serdev, const unsigned char *buffer, size_t size) {
	char *last_char_ptr = buffer + size - 1;
	char last_char = (char)(*last_char_ptr);

	printk("pi_uart - Received %ld bytes with \"%s\"\n", size, buffer);

	if (global_buffer_head >= 255) {
		global_buffer_head = 0;
	}

	global_buffer[global_buffer_head] = last_char;

	if (global_buffer_size == global_buffer_head) {
		global_buffer_size++;
	}

	global_buffer_head++;

	return size;
}

static const struct serdev_device_ops pi_uart_ops = {
	.receive_buf = receive_data_by_uart_and_save_last_byte
};

static int pi_uart_probe(struct serdev_device *serdev) {
	int status;

	printk("pi_uart - now im in the probe function!\n");

	proc_file = proc_create("pi-uart-data", 0666, NULL, &pi_uart_proc_fops);
	if(proc_file == NULL) {
		printk("pi_uart - Error creating /proc/pi-uart-data\n");
		return -ENOMEM;
	}

	serdev_device_set_client_ops(serdev, &pi_uart_ops);

	status = serdev_device_open(serdev);

	if(status > 1) {
		printk("pi_uart - error when opening the serial device!\n");
		return -1;
	}

	serdev_device_set_baudrate(serdev, 9600);

	serdev_device_set_flow_control(serdev, false);

	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);

	return 0;
}

static void pi_uart_remove(struct serdev_device *serdev) {
	printk("pi_uart - now im in the remove function!\n");
	proc_remove(proc_file);
	serdev_device_close(serdev);
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("pi_uart - Hello, Kernel!\n");

	if(serdev_device_driver_register(&pi_uart_driver)) {
		printk("pi_uart - could not load driver!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("pi_uart - Goodbye, Kernel\n");

	serdev_device_driver_unregister(&pi_uart_driver);
}

module_init(my_init);
module_exit(my_exit);