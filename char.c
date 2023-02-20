#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>


extern void *v_buffer;
size_t datalen = 4*1024*1024;

// max Minor devices
#define MAX_DEV 1
static int gcount = 0;
static int mychardev_open(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device open\n");
    gcount = 0;
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device close\n");
    return 0;
}

static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{

    printk("MYCHARDEV: Device read\n");

    if(gcount == datalen)
    {
	    return 0;
    }

    if (count > datalen) {
        count = datalen;
        gcount = datalen;
    }

    if(count < datalen)
    {
	    *offset = count;
    }
    gcount = count;


    if (copy_to_user(buf, &(((unsigned char*)v_buffer)[*offset]), count)) {
        return -EFAULT;
    }

    return count;
}

// initialize file_operations
static const struct file_operations fops = {
    .owner      = THIS_MODULE,
    .open       = mychardev_open,
    .release    = mychardev_release,
    .read       = mychardev_read,
};

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

int create_entry(struct device dev_g)
{
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
		pr_err("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
	/*Creating cdev structure*/
	cdev_init(&etx_cdev,&fops);
	/*Adding character device to the system*/
	if((cdev_add(&etx_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_class;
	}
	/*Creating struct class*/
	if(IS_ERR(dev_class = class_create(THIS_MODULE,"etx_class"))){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}
	/*Creating device*/
	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
		pr_err("Cannot create the Device 1\n");
		goto r_device;
	}
	pr_info("Device Driver Insert...Done!!!\n");
	return 0;
r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

int  delete_entry(struct device dev_g)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
	return 0;
}
