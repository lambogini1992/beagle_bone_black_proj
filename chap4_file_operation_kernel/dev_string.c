#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define DEVICE_FILE_NAME		"invert_string"

static struct _buffer_invert_
{
	char 	*invert_buf;
	size_t  buf_size;
}buf_invert;

static struct
{
	dev_t dev;
	struct class *class;
	struct cdev cdev;
}kern_dev_info;


static int 			dev_open(struct inode *, struct file *);
static int 			dev_close(struct inode *, struct file *);
static ssize_t 		dev_read(struct file*, char __user *, size_t, loff_t *);
static ssize_t 		dev_write(struct file *, const char __user *, size_t, loff_t *);
static int 			dev_invert_string(const char *src_buff, size_t str_len);

static int dev_invert_string(const char *src_buff, size_t str_len)
{
	int 	buf_idx 	= 0;
	char 	val_temp 	= 0;

	buf_invert.invert_buf  = kzalloc((str_len + 1), GFP_KERNEL);
	if(!buf_invert.invert_buf)
	{
		pr_err("Faile to alocate memory for invert buffer\r\n");
		return -1;
	}

	if(copy_from_user(buf_invert.invert_buf, src_buff, str_len))
	{
		pr_err("Faile to copy buffer\r\n");
		return -1;
	}

	buf_invert.buf_size = str_len;
	for(buf_idx = 0; buf_idx < (((uint32_t)buf_invert.buf_size)/2); buf_idx++)
	{
		val_temp 									= buf_invert.invert_buf[buf_invert.buf_size - buf_idx -1];
		buf_invert.invert_buf[buf_invert.buf_size - buf_idx -1]	= buf_invert.invert_buf[buf_idx];
		buf_invert.invert_buf[buf_idx]				= val_temp;

	}
	return 0;
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	pr_info("open character device\n");
	return 0;
}
static int dev_close(struct inode *inodep, struct file *filep)
{
	pr_info("close\n");
	return 0;
}

static ssize_t dev_read(struct file*filep, char __user *buf, size_t len, loff_t *offset)
{
	pr_info("read\n");
	if(!buf_invert.invert_buf)
	{
		pr_err("Should be Write String before read\r\n");
		return -EFAULT;
	}


	ssize_t len_invert = (ssize_t)strlen(buf_invert.invert_buf);
	if(copy_to_user(buf, buf_invert.invert_buf, buf_invert.buf_size))
	{
		return -EFAULT;
	}
	kfree(buf_invert.invert_buf);
	buf_invert.invert_buf = NULL;
	buf_invert.buf_size   = 0;
	return len_invert;
}

static ssize_t dev_write(struct file*filep, const char __user *buf, size_t len, loff_t *offset)
{
	pr_info("write\n");
	buf_invert.buf_size 	= 0;
	buf_invert.invert_buf 	= NULL;
	if(dev_invert_string(buf, len))
	{
		return 0;
	}
	return len;
}


static struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.read = dev_read,
	.write = dev_write,
};

static int __init exam_init(void)
{
	int ret;
	pr_info("Start Create Device File\n");
	ret = alloc_chrdev_region(&kern_dev_info.dev, 0, 1, "invert_str");
	if(ret)
	{
		pr_err("Failed to major number for device file");
		return -1;
	}
	pr_info("Kernel Module Infor MAJOR=%d MINOR=%d\r\n", MAJOR(kern_dev_info.dev), MINOR(kern_dev_info.dev));

	kern_dev_info.class = class_create(THIS_MODULE, "invert_class");
	if(IS_ERR(kern_dev_info.class))
	{
		pr_err("Failed to create class device\r\n");
		goto failed_create_class;
	}

	ret = (int)device_create(kern_dev_info.class, NULL, kern_dev_info.dev, NULL, "invert_dev");
	if(ret <= 0)
	{
		pr_err("Failed to create device\r\n");
		goto failed_create_device;
	}

	cdev_init(&kern_dev_info.cdev, &dev_fops);

	ret = cdev_add(&kern_dev_info.cdev, kern_dev_info.dev, 1);
	if(ret < 0)
	{
		pr_err("Failed to create cdev\r\n");
		goto failed_create_cdev;
	}


	pr_info("Kernel Module Inserted Successfully...\n");
	return 0;

failed_create_cdev:
	device_destroy(kern_dev_info.class,kern_dev_info.dev);
failed_create_device:
	class_destroy(kern_dev_info.class);
failed_create_class:
	unregister_chrdev_region(kern_dev_info.dev, 1);
	return -1;
}

static void __exit exam_exit(void)
{
	cdev_del(&kern_dev_info.cdev);
	device_destroy(kern_dev_info.class,kern_dev_info.dev);
	class_destroy(kern_dev_info.class);
	unregister_chrdev_region(kern_dev_info.dev, 1);
	pr_info("Kernel Module Removed Successfully...\n");
}

module_init(exam_init);
module_exit(exam_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pham Tuan Anh");