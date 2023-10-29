#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

/* Meta Information */
MODULE_LICENSE("GPL");

/* Declate the probe and remove functions */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "brightlight,mydev"
	},

	{/* sentinel */}
};

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
	.probe = dt_probe,
	.remove = dt_remove,
	.driver = {
		.name = "my_device_driver",
		.of_match_table = my_driver_ids
	},
};

static int dt_probe(struct platform_device*pdev) {
	struct device *dev = &pdev->dev;

	const char *label;

	int my_value, ret;

	printk("dt-probe - now im in the probe function!\n");

	if(!device_property_present(dev, "label")) {
		printk("dt-probe - device property label not found!\n");
		return -1;
	}

	if(!device_property_present(dev, "my_value")) {
		printk("dt-probe - device property my_value not found!\n");
		return -1;
	}

	ret = device_property_read_string(dev, "label", &label);
	if(ret) {
		printk("dt-probe - count not read label!\n");
		return -1;
	}

	ret = device_property_read_u32(dev, "my_value", &my_value);
	if(ret) {
		printk("dt-probe - count not read my_value!\n");
		return -1;
	}

	printk("dt-probe - label is %s", label);
	printk("dt-probe - my_value is %d", my_value);

	return 0;
}

static int dt_remove(struct platform_device *pdev) {
	printk("dt-probe - now im in the remove function!\n");
	return 0;
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("dt-probe - Hello, Kernel!\n");

	if(platform_driver_register(&my_driver)) {
		printk("dt-probe - could not load driver!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("dt-probe - Goodbye, Kernel\n");

	platform_driver_unregister(&my_driver);
}

module_init(my_init);
module_exit(my_exit);