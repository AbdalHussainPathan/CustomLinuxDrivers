#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/version.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt,__func__ //check printk.h
#define DEV_MEM_SIZE 512

/*This Holds the device number*/
dev_t device_number;
/*pseudo Device Memory*/
char device_buffer[DEV_MEM_SIZE];
/*Cdev Var*/
struct cdev pcd_cdev;
static loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    pr_info("seek requested\n");
    pr_info("Current value of the file position=%lld\n",filp->f_pos);
    loff_t temp;
    switch(whence)
    {
      case SEEK_SET:
        if((offset>DEV_MEM_SIZE)||(offset<0))
          return -EINVAL;
        filp->f_pos=offset;
      break;
      case SEEK_CUR:
        temp=filp->f_pos+offset;
        if((temp>DEV_MEM_SIZE)||(temp<0))
          return -EINVAL;
        filp->f_pos=temp;
      break;
      case SEEK_END:
        temp=DEV_MEM_SIZE+offset;
        if((temp>DEV_MEM_SIZE)||(temp<0))
          return -EINVAL;
        filp->f_pos=temp;
      break;
     default:
      return -EINVAL;
    }
    pr_info("New Value of the file position=%lld\n",filp->f_pos);
    return filp->f_pos;
}
static ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("Read requested for %zu bytes\n",count);
  pr_info("Current file position %lld\n",*f_pos );
	/*Adjust the count*/
	if((*f_pos+count)>DEV_MEM_SIZE)
		count=DEV_MEM_SIZE-*f_pos;
	/*copy to user*/
	if(copy_to_user(buff,&device_buffer[*f_pos],count))
	{
	return -EFAULT;
	}
	/*update the current file position*/

	*f_pos+=count;
pr_info("Number of bytes successfully read= %zu\n",count);
pr_info("Updated file position %lld\n",*f_pos );

/*return number of bytes which have been successfully read*/
	return count;
}
static ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("Write requested for %zu bytes\n",count);
  pr_info("Current file position %lld\n",*f_pos );
  /*Adjust the count*/
  if((*f_pos+count)>DEV_MEM_SIZE)
	  count=DEV_MEM_SIZE-*f_pos;
  if(!count)
    return -ENOMEM;
  /*copy from user*/
  if(copy_from_user(&device_buffer[*f_pos],buff,count))
  {
  return -EFAULT;
  }
  /*update the current file position*/

  *f_pos+=count;
      pr_info("Number of bytes successfully write= %zu\n",count);
      pr_info("Updated file position %lld\n",*f_pos );

      /*return number of bytes which have been successfully read*/
        return count;
}
static int pcd_open (struct inode *inode, struct file *filp)
{
  pr_info("open was success\n");
  return 0;
}
static int pcd_release (struct inode *inode, struct file *filp)
{
   pr_info("close was success\n");
  return 0;
}
/*file operations of the driver*/
struct file_operations pcd_fops=
{
  .open=pcd_open,
  .write=pcd_write,
  .read=pcd_read,
  .llseek=pcd_lseek,
  .release=pcd_release,
  .owner=THIS_MODULE
};
struct class *class_pcd;
struct device *device_pcd;
static int __init pcd_driver_init(void)
{
	/*Dynamically allocate a device number*/
	int ret;
	ret=alloc_chrdev_region(&device_number,0,1,"pcd_devices"); //linux4.14/fs/char_dev.c
	if(ret<0)
	{
	  pr_err("Alloc chr dev failed\n");
	  goto out;
	}
	pr_info("Device number <major>:<minor>=%d:%d\n",MAJOR(device_number),MINOR(device_number)); 
	/*Init the Cdev structure with fops*/
	cdev_init(&pcd_cdev,&pcd_fops);//linux4.14/fs/char_dev.c
	pcd_cdev.owner=THIS_MODULE;
	/*3.Register a device (cdev struct) with VFS*/
	ret=cdev_add(&pcd_cdev,device_number,1);//linux4.14/fs/char_dev.c
	if(ret<0)
	{
	  pr_err("Device Registration failed\n");
	  goto unreg_chrdev;
	}
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
            class_pcd = class_create("pcd_class"); // linux4.14/drivers/base/class.c
        #else
            class_pcd = class_create(THIS_MODULE, "pcd_class");
        #endif
        if(IS_ERR(class_pcd))
        {
          pr_err("Class Creation failed\n");
          ret=PTR_ERR(class_pcd);
          goto cdev_del;
        }
	/*create device class*/
	//class_pcd=class_create(THIS_MODULE,"pcd_class");
	/*populate the sysfs with device information*/
	device_pcd=device_create(class_pcd,NULL,device_number,NULL,"pcd");// linux4.14/drivers/base/base.c
	if(IS_ERR(device_pcd))
        {
          pr_err("Device Creation failed\n");
          ret=PTR_ERR(device_pcd);
          goto class_del;
        }
	pr_info("Module info was success!\n");
	return 0;
	class_del:
	  device_destroy(class_pcd,device_number);
	cdev_del:
	  cdev_del(&pcd_cdev);
	unreg_chrdev:
	  unregister_chrdev_region(device_number,1);
	out:
	  pr_err("Module Insertion failed\n");
	  return ret;
}

static void __exit pcd_driver_cleanup(void)
{
  device_destroy(class_pcd,device_number);
  class_destroy(class_pcd);
  cdev_del(&pcd_cdev);
  unregister_chrdev_region(device_number,1);
  pr_info("module unloaded\n");

}
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABDAL");
MODULE_DESCRIPTION("A pseudo character Driver");
