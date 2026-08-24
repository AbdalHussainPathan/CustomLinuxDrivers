#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/version.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt,__func__ //check printk.h
#define DEV_MEM_SIZE_PCDEV1 1024
#define DEV_MEM_SIZE_PCDEV2 1024
#define DEV_MEM_SIZE_PCDEV3 1024
#define DEV_MEM_SIZE_PCDEV4 1024
#define NO_OF_DEV           4

#define RONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11

int chk_permission(int dev_perm,int access_mode);

/*pseudo Device Memory*/
char device_buffer_pcdev1[DEV_MEM_SIZE_PCDEV1];
char device_buffer_pcdev2[DEV_MEM_SIZE_PCDEV2];
char device_buffer_pcdev3[DEV_MEM_SIZE_PCDEV3];
char device_buffer_pcdev4[DEV_MEM_SIZE_PCDEV4];

struct pcdev_private_data
{
  char *buffer;
  unsigned size;
  const char *serial_number;
  int perm;
  struct cdev cdev;
};
struct pcdrv_private_data
{
  int total_devices;
  /*This Holds the device number*/
  dev_t device_number;
  struct class *class_pcd;
  struct device *device_pcd;
  struct pcdev_private_data pcdev_data[NO_OF_DEV];
};
struct pcdrv_private_data pcdrv_data=
{
  .total_devices=NO_OF_DEV,
  .pcdev_data ={
                  [0]=
                    {
                      .buffer=device_buffer_pcdev1,
                      .size=DEV_MEM_SIZE_PCDEV1,
                      .serial_number="PCDEV1",
                      .perm=RONLY //RONLY
                    },
                    [1]=
                    {
                      .buffer=device_buffer_pcdev2,
                      .size=DEV_MEM_SIZE_PCDEV2,
                      .serial_number="PCDEV2",
                      .perm=WRONLY //WRONLY
                    },
                    [2]=
                    {
                      .buffer=device_buffer_pcdev3,
                      .size=DEV_MEM_SIZE_PCDEV3,
                      .serial_number="PCDEV3",
                      .perm=RDWR //RDWR
                    },
                    [3]=
                    {
                      .buffer=device_buffer_pcdev4,
                      .size=DEV_MEM_SIZE_PCDEV4,
                      .serial_number="PCDEV4",
                      .perm=RDWR //RDWR
                    }
                }        
};
static loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    pr_info("seek requested\n");
    pr_info("Current value of the file position=%lld\n",filp->f_pos);
    loff_t temp;
     struct pcdev_private_data *pcdev_data=(struct pcdev_private_data*)filp->private_data;//type cast it bcz it's a void data
  int max_size=pcdev_data->size;//in Open method we already saved the device private data in filp
    switch(whence)
    {
      case SEEK_SET:
        if((offset>max_size)||(offset<0))
          return -EINVAL;
        filp->f_pos=offset;
      break;
      case SEEK_CUR:
        temp=filp->f_pos+offset;
        if((temp>max_size)||(temp<0))
          return -EINVAL;
        filp->f_pos=temp;
      break;
      case SEEK_END:
        temp=max_size+offset;
        if((temp>max_size)||(temp<0))
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
  struct pcdev_private_data *pcdev_data=(struct pcdev_private_data*)filp->private_data;//type cast it bcz it's a void data
  int max_size=pcdev_data->size;//in Open method we already saved the device private data in filp
	/*Adjust the count*/
	if((*f_pos+count)>max_size)
		count=max_size-*f_pos;
	/*copy to user*/
	if(copy_to_user(buff,pcdev_data->buffer+(*f_pos),count))
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
    struct pcdev_private_data *pcdev_data=(struct pcdev_private_data*)filp->private_data;//type cast it bcz it's a void data
  int max_size=pcdev_data->size;//in Open method we already saved the device private data in filp
  /*Adjust the count*/
  if((*f_pos+count)>max_size)
	  count=max_size-*f_pos;
  if(!count)
    return -ENOMEM;
  /*copy from user*/
  if(copy_from_user(pcdev_data->buffer+(*f_pos),buff,count))
  {
    return -EFAULT;
  }
  /*update the current file position*/

  *f_pos+=count;
      pr_info("Number of bytes successfully write= %zu\n",count);
      pr_info("Updated file position %lld\n",*f_pos );

      /*return number of bytes which have been successfully written*/
        return count;
}
int chk_permission(int dev_perm,int access_mode)
{
  if(dev_perm==RDWR)
    return 0;
  if((dev_perm==RONLY)&&((access_mode&FMODE_READ)&&!(access_mode&FMODE_WRITE))) //RONLY access
    return 0;
  if((dev_perm==WRONLY)&&((access_mode&FMODE_WRITE)&&!(access_mode&FMODE_READ))) //WONLY access
    return 0;
  return -EPERM;
}
static int pcd_open (struct inode *inode, struct file *filp)
{
  short int ret=0;
  int minor_n;
  struct pcdev_private_data *pcdev_data;
  //find out on which device file open was attempted by the user space
  minor_n=MINOR(inode->i_rdev); //driver distinguishes between devices using minor number
  pr_info("Minor Access =%d\n",minor_n);

  //get device's private data structure 

  //include/linux/kernel.h ->container_of() gives addr of container holding it's memeber element//linux/include/linux/  fs.h -> *i_cdev 
  pcdev_data=container_of(inode->i_cdev,struct pcdev_private_data,cdev); 
  //to supply device private data to other methods of the driver
  filp->private_data=pcdev_data;

  //check permission
  //For example if pcdev1 is rdonly hence driver shouldn't open it with rwonly permissions
  ret = chk_permission(pcdev_data->perm,filp->f_mode);
  (!ret)?pr_info("open was success\n"):pr_info("open was unsuccess\n");
  return ret;
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

static int __init pcd_driver_init(void)
{
        int ret;
        short int i=0;
	ret=alloc_chrdev_region(&pcdrv_data.device_number,0,NO_OF_DEV,"pcd_devices"); //linux4.14/fs/char_dev.c
	if(ret<0)
	{
	  pr_err("Alloc chr dev failed\n");
	  goto out;
	}
	
	
	/*create device class*/
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
            pcdrv_data.class_pcd = class_create("pcd_class"); // linux4.14/drivers/base/class.c
        #else
            pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
        #endif
        if(IS_ERR(pcdrv_data.class_pcd))
        {
          pr_err("Class Creation failed\n");
          ret=PTR_ERR(pcdrv_data.class_pcd);
          goto unreg_chrdev;
        }
        
          
	for(i=0;i<NO_OF_DEV;i++)
	{
	    pr_info("Device number <major>:<minor>=%d:%d\n",MAJOR(pcdrv_data.device_number+i),MINOR(pcdrv_data.device_number+i)); 
	    
	    
	  /*Init the Cdev structure with fops*/
	  cdev_init(&pcdrv_data.pcdev_data[i].cdev,&pcd_fops);//linux4.14/fs/char_dev.c
	  pcdrv_data.pcdev_data[i].cdev.owner=THIS_MODULE;
	  
	  
	  /*3.Register a device (cdev struct) with VFS*/
	  ret=cdev_add(&pcdrv_data.pcdev_data[i].cdev,pcdrv_data.device_number+i,1);//linux4.14/fs/char_dev.c
	  if(ret<0)
	  {
	    pr_err("Device Registration failed\n");
	    goto cdev_del;
	  }
	  /*populate the sysfs with device information*/
	  pcdrv_data.device_pcd=device_create(pcdrv_data.class_pcd,NULL,pcdrv_data.device_number+i,NULL,"pcdev-%d",i+1);// linux4.14/drivers/base/base.c
	  if(IS_ERR(pcdrv_data.device_pcd))
          {
            pr_err("Device Creation failed\n");
            ret=PTR_ERR(pcdrv_data.device_pcd);
            goto class_del;
          }
        }
	pr_info("Module info was success!\n");
	return 0;
	cdev_del:
	class_del:
	  for(;i>=0;i--)
	  {
	    device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+i);//linux/drivers/base/core.c
	    cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	  }
	  class_destroy(pcdrv_data.class_pcd);
	unreg_chrdev:
	  unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEV);
	out:
	  pr_err("Module Insertion failed\n");
	  return ret;
}
static void __exit pcd_driver_cleanup(void)
{
short int i=0;
    for(i=0;i<NO_OF_DEV;i++)
    {
      device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+i);//linux/drivers/base/core.c
      cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);

    unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEV);

    pr_info("Module unloaded\n");
}
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABDAL");
MODULE_DESCRIPTION("A pseudo character Driver for N Devices");
