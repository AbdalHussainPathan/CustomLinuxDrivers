#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/slab.h> //kfree
#include<linux/kthread.h>
#include<linux/wait.h>
#include<linux/err.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h> //copy_from_user
static int ex_open(struct inode *inode,struct file *flip);
static int ex_release(struct inode *inode,struct file *flip);
ssize_t ex_read(struct file *filp, char __user *buf,size_t len,loff_t *fpos);
ssize_t ex_write(struct file *filp, const char __user *buf,size_t len,loff_t *fpos);

static int __init init_function(void);
staitc void __exit exit_function(void);
struct file_operations fops=
{
    .open=ex_open,
    .release=ex_release,
    .owner=THIS_MODULE,
    .read=ex_read,
    .write=ex_write
};
dev_t dev_num;
struct class *class_dev;
struct cdev cdev_dev;
static int __init init_function(void)
{
    int ret;
    ret=alloc_chrdev_region(&dev_num,0,1,"Queues_Dev");
    if(ret<0) return -ENOMEM;
    cdev_init(&cdev_dev,&fops);
    cdev_dev.owner=THIS_MODULE;
    cdev_dev.ops=&fops;

    if(cdev_add(&cdev_dev,dev_num,1)<0)
    {
        pr_info("Cannot add device to system\n");
        goto unreg_chrdev;
    }
    class_dev=create_class(THIS_MODULE,"class_dev");
    if(IS_ERR(class_dev))
    {
        pr_info("Cannot create class\n");
        goto unreg_chrdev;
    }
    if(IS_ERR(device_create(class_dev,NULL,dev_num,NULL,"ex_device")))
    {
        pr_info("Cannot create Device\n");
        goto class_des;
    }
    return 0;
unreg_chrdev:
    unregister_chrdev_region(dev_num,1);
class_des:
    class_destroy(class_dev);
    return -1;
}