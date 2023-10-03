#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
//for platform drivers....
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>


#define DRIVER_NAME "Sample_Pldrv"

MODULE_LICENSE("GPL");

struct custom_gpio_dev
{
	struct device *dev;
	dev_t dev_num;
	struct class *class;
	struct cdev cdev;
    struct gpio_desc *gpio_out;
    struct gpio_desc *gpio_input;
    int irq_gpio_number;
    int state_output;
};


static irqreturn_t gpio_irq_handler(int irq,void *dev_id)
{
    //Signal to another app know what happend
    return IRQ_HANDLED;
}

/*
** This function will be called when we open the Misc device file
*/
static int custom_gpio_open(struct inode *inode, struct file *file)
{
    int ret;
    struct custom_gpio_dev *gpio_dev;

    gpio_dev= container_of(inode->i_cdev, struct custom_gpio_dev, cdev);
    file->private_data = gpio_dev;

    pr_info("open was unsucessful\n");

    return 0;
}
/*
** This function will be called when we close the Misc Device file
*/
static int custom_gpio_close(struct inode *inodep, struct file *filp)
{
    printk(KERN_INFO "Custom GPIO device close\n");
    return 0;
}
/*
** This function will be called when we write the Misc Device file
*/
static ssize_t custom_gpio_write(struct file *file, const char __user *buf,
               size_t len, loff_t *ppos)
{
    printk(KERN_INFO "Custom GPIO device write\n");
    struct custom_gpio_dev *gpio_dev;
    char *str_user;
    int ret;

    gpio_dev = file->private_data;
    if(!gpio_dev)
    {
		printk(KERN_ERR "File dont have private data\n");
        return -ENOMEM;
    }

	str_user = kzalloc(len, GFP_KERNEL);
	if(NULL == str_user)
	{
		printk(KERN_ERR "fail to allocate str_user\n");
		return 0;
	}

	if(copy_from_user(str_user, buf, len))
	{
		printk(KERN_ERR "fail to get data from user buffer\n");
        ret = -EFAULT;
        goto exit_func;
	}

    ret = len;
    printk(KERN_ERR "User Set GPIO value %s", str_user);
    if(strncmp(str_user, "1", 1) == 0)
    {
        gpio_dev->state_output = 1;
    }
    else if(strncmp(str_user, "0", 1) == 0)
    {
        gpio_dev->state_output = 0;
    }
    else
    {
        printk(KERN_ERR "User CMD not support %s", str_user);
    }
    gpiod_set_value(gpio_dev->gpio_out, gpio_dev->state_output);

    /* We are not doing anything with this data now */
exit_func:
    kfree(str_user);
    return ret;

}

/*
** This function will be called when we read the Misc Device file
*/
static ssize_t custom_gpio_read(struct file *filp, char __user *buf,
                    size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "Custom GPIO device read\n");

    return 0;
}


//File operation structure
static const struct file_operations custom_gpio_fops = {
    .owner          = THIS_MODULE,
    .write          = custom_gpio_write,
    .read           = custom_gpio_read,
    .open           = custom_gpio_open,
    .release        = custom_gpio_close,
    .llseek         = no_llseek,
};


/***************/
static const struct of_device_id custom_gpio_of_dev[]=
{
	{ .compatible = "custome_gpio", },
	{0},
};

MODULE_DEVICE_TABLE(of,custom_gpio_of_dev);

/**************/
static int sample_drv_probe(struct platform_device *pdev)
{
	printk("Initialize Driver\n");
    int error;
    struct custom_gpio_dev *custgpio_dev;
	const struct  of_device_id *match;
    int gpio_no;

	match = of_match_device(custom_gpio_of_dev, &(pdev->dev));
	if(!match)
	{
		printk(KERN_ERR "Fail to matching device and device table\n");
		return -EINVAL;
	}

    custgpio_dev = kzalloc(sizeof(struct custom_gpio_dev), GFP_KERNEL);
    if(!custgpio_dev)
    {
		printk(KERN_ERR "Fail to Allocate memory\n");
		return -ENOMEM;
    }

    custgpio_dev->gpio_out = devm_gpiod_get(&pdev->dev, "output", GPIOD_OUT_LOW);
	if (IS_ERR(custgpio_dev->gpio_out)) {
		printk(KERN_ERR  "failed to get output gpio\n");
		error = PTR_ERR(custgpio_dev->gpio_out);
		goto init_failed;
	}

    custgpio_dev->gpio_input = devm_gpiod_get(&pdev->dev, "input", GPIOD_IN);
	if (IS_ERR(custgpio_dev->gpio_input)) {
		printk(KERN_ERR  "failed to get input gpio\n");
		error = PTR_ERR(custgpio_dev->gpio_input);
		goto init_failed;
	}

    gpio_no =  desc_to_gpio(custgpio_dev->gpio_out);
    if(!gpio_is_valid(gpio_no))
    {
		printk(KERN_ERR  "GPIO OUTPUT is invalid\n");
        error = -2;
        goto init_failed;
    }
    pr_info("GPIO OUTPUT number %d", gpio_no);

    gpio_no =  desc_to_gpio(custgpio_dev->gpio_input);
    if(!gpio_is_valid(gpio_no))
    {
		printk(KERN_ERR  "GPIO is invalid\n");
        error = -2;
        goto init_failed;
    }
    pr_info("GPIO Input number %d", gpio_no);

    //set interrupt for input gpio
    custgpio_dev->irq_gpio_number =  gpio_to_irq(gpio_no);
    pr_info("GPIO IRQ number %d", custgpio_dev->irq_gpio_number);

    error = request_irq(custgpio_dev->irq_gpio_number, (void *)gpio_irq_handler,
                        IRQF_TRIGGER_RISING,
                        "custom_gpio_irq", NULL);
    if(error)
    {
        printk(KERN_ERR "Failed to register interrupt %d\r\n", error);
        goto init_failed;
    }



	error = alloc_chrdev_region(&custgpio_dev->dev_num, 0, 1, "custom_gpio");
	if(error)
	{
		pr_err("Failed to major number for device file %d\r\n", error);
        error = -1;
        goto failed_alloc_dev;
	}
	pr_info("Kernel Module Infor MAJOR=%d MINOR=%d\r\n", MAJOR(custgpio_dev->dev_num), MINOR(custgpio_dev->dev_num));

	custgpio_dev->class = class_create(THIS_MODULE, "custom_gpio_class");
	if(IS_ERR(custgpio_dev->class))
	{
		pr_err("Failed to create class device %d\r\n", error);
		goto failed_create_class;
	}

	custgpio_dev->dev = device_create(custgpio_dev->class, NULL, custgpio_dev->dev_num, NULL, "custom_gpio_dev");
	if (IS_ERR(custgpio_dev->dev))
    {
		error = PTR_ERR(custgpio_dev->dev);
		pr_err("Failed to create device %d\r\n", error);
		goto failed_create_device;
	}

	cdev_init(&custgpio_dev->cdev, &custom_gpio_fops);

	error = cdev_add(&custgpio_dev->cdev, custgpio_dev->dev_num, 1);
	if(error < 0)
	{
		pr_err("Failed to create cdev %d\r\n", error);
		goto failed_create_cdev;
	}


	pr_info("Kernel Module Inserted Successfully...\n");
    platform_set_drvdata(pdev, custgpio_dev);
    custgpio_dev->state_output = 0;
    gpiod_set_value(custgpio_dev->gpio_out, custgpio_dev->state_output);
	return 0;

failed_create_cdev:
	device_destroy(custgpio_dev->class,custgpio_dev->dev_num);
failed_create_device:
	class_destroy(custgpio_dev->class);
failed_create_class:
	unregister_chrdev_region(custgpio_dev->dev_num, 1);
failed_alloc_dev:
    free_irq(custgpio_dev->irq_gpio_number, NULL);
init_failed:
    kfree(custgpio_dev);
    return error;
}
static int sample_drv_remove(struct platform_device *pdev)
{
    struct custom_gpio_dev *custgpio_dev;
	printk(KERN_INFO "Remove driver\n");
    custgpio_dev = (struct custom_gpio_dev *)platform_get_drvdata(pdev);

	cdev_del(&custgpio_dev->cdev);
	device_destroy(custgpio_dev->class,custgpio_dev->dev_num);
	class_destroy(custgpio_dev->class);
	unregister_chrdev_region(custgpio_dev->dev_num, 1);
    free_irq(custgpio_dev->irq_gpio_number, NULL);
    kfree(custgpio_dev);
	return 0;
}

static struct platform_driver sample_pldriver = {
    .probe          = sample_drv_probe,
    .remove         = sample_drv_remove,
    .driver = {
            .name  = DRIVER_NAME,
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(custom_gpio_of_dev),
    },
};
/**************/

static int __init ourinitmodule(void)
{
    printk(KERN_ALERT "Welcome to sample Platform driver.... \n");

    /* Registering with Kernel */
    int error = platform_driver_register(&sample_pldriver);
	printk("Return for register sample driver %d\r\n", error);
    return 0;
}

static void __exit ourcleanupmodule(void)
{
    printk(KERN_ALERT "Thanks....Exiting sample Platform driver... \n");

    /* Unregistering from Kernel */
    platform_driver_unregister(&sample_pldriver);

    return;
}

module_init(ourinitmodule);
module_exit(ourcleanupmodule);

MODULE_DESCRIPTION("Platform device and driver example");
MODULE_AUTHOR("PTA");
MODULE_LICENSE("GPL v2");