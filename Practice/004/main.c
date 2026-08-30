//Linux Includes
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/version.h>
#include<linux/ioctl.h>
//Defines
#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt,__func__ //check printk.h
#define MAX_SIZE 1024
#define WR_ONLY _IOW('a','a',int32_t)
#define RD_ONLY _IOR('a','b',int32_t)
#define RD_WR   _IOWR('a','c',int32_t)

//Delarations
char device_buff[MAX_SIZE];
dev_t ex_devnum;
int value=0;
static struct class *ex_class;
static struct device *ex_dev;
static struct cdev ex_cdev;
static int ex_open(struct inode *inode, struct file *filp);
static ssize_t ex_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
static ssize_t ex_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
static int ex_release (struct inode *inode, struct file *filp);
static long ex_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static struct file_operations ex_fops=
{
  .open=ex_open,
  .write=ex_write,
  .read=ex_read,
  .release=ex_release,
  .unlocked_ioctl=ex_ioctl,
  .owner=THIS_MODULE
};
//Defination of functions
static long ex_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  pr_info("IOCTL is called\n");
  pr_info("cmd is: %u\n",cmd);
  pr_info("arg is: %lu\n",arg);

  switch(cmd)
  {
    case WR_ONLY:
      pr_info("IOCTL CMD WR_ONLY is called\n");
      if(copy_from_user(&value,(int32_t*)arg,sizeof(value)))
        return -EFAULT;
      pr_info("Written value is %d\n",value);
      break;
    case RD_ONLY:
      pr_info("IOCTL CMD RD_ONLY is called\n");
      if(copy_to_user((int32_t*)arg,&value,sizeof(value)))
        return -EFAULT;
      pr_info("value is Read %d\n",value);
      break;
    case RD_WR:
      pr_info("IOCTL CMD RD_WR is called\n");
      // 1. Copy the current kernel value out to the user's buffer first
      int32_t temp_val = value; 
      if(copy_to_user((int32_t*)arg, &temp_val, sizeof(temp_val)))
          return -EFAULT;
          
      // 2. Fetch the new value the user wants to write into the kernel variable
      if(copy_from_user(&value, (int32_t*)arg, sizeof(value)))
          return -EFAULT;
          
      pr_info("RD_WR updated value from %d to %d\n", temp_val, value);
      break;
    default:
      pr_info("Invalid IOCTL command\n");
      return -EINVAL;
  }
  return 0;
}
static int ex_release (struct inode *inode, struct file *filp)
{
   pr_info("close was success\n");
  return 0;
}
static int ex_open(struct inode *inode, struct file *filp)
{
  pr_info("Open is called\n");

  return 0;
}
static ssize_t ex_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("write is called\n");
  pr_info("Current file pos is: %lld\n",*f_pos);
  pr_info("Requested bytes to write: %zu\n",count);
  if(*f_pos+count>MAX_SIZE)
      count=MAX_SIZE-*f_pos;
  if(!count) return -ENOMEM;
  if(copy_from_user(&device_buff[*f_pos],buff,count))
    return -EFAULT;
  *f_pos+=count;
  pr_info("Succesfully write bytes : %zu\n",count);
  pr_info("Updated file pos is: %lld\n",*f_pos);
  return count;
}
static ssize_t ex_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("read is called\n");
  pr_info("Current file pos is: %lld\n",*f_pos);
  pr_info("Requested bytes to read: %zu\n",count);
  if(*f_pos+count>MAX_SIZE)
      count=MAX_SIZE-*f_pos;
  if(copy_to_user(buff,&device_buff[*f_pos],count))
    return -EFAULT;
  *f_pos+=count;
  pr_info("Succesfully read bytes : %zu\n",count);
  pr_info("Updated file pos is: %lld\n",*f_pos);
  return count;
}

static int __init hello_init(void)
{
        int ret=0;
        alloc_chrdev_region(&ex_devnum,0,1,"DyanmicAlloc");
        if(ret<0) //static Major:Min allocation(Kernel picks major num  and stroes in dev_num
            {return ret;}
        cdev_init(&ex_cdev,&ex_fops);
        if(cdev_add(&ex_cdev,ex_devnum,1)<0)
        {
          pr_err("Cannot device to the system\n");
          goto unreg_dev;
        }
        ex_class=class_create("Example_Class");
        if(IS_ERR(ex_class))
        {
          pr_err("Cannot create class\n");
          goto cdev_del_err;
        }
        ex_dev=device_create(ex_class,NULL,ex_devnum,NULL,"Example_Device");
        if(IS_ERR(ex_dev))
        {
           pr_err("Cannot create class\n");
           goto class_des;
        }
        pr_info("Major: %d,  Minor: %d",MAJOR(ex_devnum),MINOR(ex_devnum));
	pr_info("Module Inserted\n");
	return 0;
unreg_dev:
    unregister_chrdev_region(ex_devnum,1);
cdev_del_err:
    cdev_del(&ex_cdev);
class_des:
    class_destroy(ex_class);
    return -1;
}
static void __exit hello_CleanUp(void)
{
    device_destroy(ex_class,ex_devnum);
    cdev_del(&ex_cdev);
    unregister_chrdev_region(ex_devnum,1);
    class_destroy(ex_class);
    pr_info("Module Removed\n");
}
module_init(hello_init);
module_exit(hello_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A Simple Module");
MODULE_INFO(board,"For a Prog for Host machine");

