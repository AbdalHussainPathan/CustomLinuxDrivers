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
static ssize_t ex_read(struct file *filp, char __user *buf,size_t len,loff_t *fpos);
static ssize_t ex_write(struct file *filp, const char __user *buf,size_t len,loff_t *fpos);
static int ThreadFunction(void *unused);
static int __init init_function(void);
static void __exit exit_function(void);

static struct task_struct *wait_thread;
DECLARE_WAIT_QUEUE_HEAD(wait_queue_ex); //Static method
//wait_queue_head_t wait_queue_ex;//Dynamic method
int wait_value=0;
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
    class_dev=class_create("class_dev");
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
    wait_thread=kthread_create(ThreadFunction,NULL,"ThreadFunction");
    if(wait_thread)
    {
        pr_info("Created ThreadFunction\n");
        wake_up_process(wait_thread);
    }
    else
        pr_info("Cannot Create ThreadFunction\n");
    pr_info("Device Driver Inserted!\n");
    return 0;
unreg_chrdev:
    unregister_chrdev_region(dev_num,1);
class_des:
    class_destroy(class_dev);
    return -1;
}
static void __exit exit_function(void)
{
    cdev_del(&cdev_dev);                          // 1. stop new I/O
    kthread_stop(wait_thread);                    // 2. tell thread to exit
    device_destroy(class_dev, dev_num);           // 3.
    class_destroy(class_dev);                     // 4.
    unregister_chrdev_region(dev_num, 1);
    wait_value=2;
    wake_up(&wait_queue_ex);
    pr_info("Removed Device Driver\n");
}
static int ThreadFunction(void *unused)
{
    while (!kthread_should_stop())
    {
        pr_info("Waiting for event!\n");
        wait_event_timeout(wait_queue_ex,
                           wait_value != 0 || kthread_should_stop(),
                           msecs_to_jiffies(2000));

        if (kthread_should_stop())
            break;

        if (wait_value == 2)
            pr_info("Thread Function called from __Exit function\n");
        else if (wait_value == 1)
            pr_info("Thread Function called from ex_read function\n");
        else if (wait_value == 3)
            pr_info("Thread Function called from ex_write function\n");
        else
            pr_info("Timeout!\n");

        wait_value = 0;
    }
    pr_info("Thread exiting cleanly\n");
    return 0;
}

static int ex_open(struct inode *inode,struct file *flip)
{
    pr_info("Open is called\n");
    return 0;
}
static int ex_release(struct inode *inode,struct file *flip)
{
    pr_info("close is called\n");
    return 0;
}
static ssize_t ex_write (struct file *flip,const char __user *buf,size_t len,loff_t *fpos)
{
    pr_info("Write is Called\n");
    wait_value=3;
    wake_up(&wait_queue_ex);
    return len;
}
static ssize_t ex_read (struct file *flip, char __user *buf,size_t len,loff_t *fpos)
{
    pr_info("Read is Called\n");
    wait_value=1;
    wake_up(&wait_queue_ex);
    return 0;
}
module_init(init_function);
module_exit(exit_function);   

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AbdalHussain");
MODULE_DESCRIPTION("Simple linux driver (Waitqueue Static/Dyanmic method)");
MODULE_VERSION("1.7");