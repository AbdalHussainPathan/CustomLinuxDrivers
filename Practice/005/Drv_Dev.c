#include "Drv_Dev.h"
static int ex_release (struct inode *inode, struct file *filp)
{
   pr_info("close was success\n");
  return 0;
}
static int ex_open(struct inode *inode, struct file *filp)
{
  pr_info("Open is called\n");
  short ret =0;
  int minor_n;
  struct dev_private_data *device_data;
  //find out on which device file open was attempted by the user space
  minor_n=MINOR(inode->i_rdev);
//driver distinguishes between devices using minor number
  pr_info("Minor Access=%d\n",minor_n);
 //include/linux/kernel.h ->container_of() gives addr of 
 //container holding it's memeber element//linux/include/linux/  fs.h -> *i_cdev 
  device_data=container_of(inode->i_cdev,struct dev_private_data,ex_cdev);
//to supply device private data to other methods of the driver
  filp->private_data=device_data;
//check permission
  //For example if pcdev1 is rdonly hence driver shouldn't open it with rwonly permissions
  ret=chk_permission(device_data->perm,filp->f_mode);
  (!ret)?pr_info("open was Success\n"):pr_info("open wasn't Success\n");
  return ret;
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
static ssize_t ex_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("write is called\n");
  pr_info("Current file pos is: %lld\n",*f_pos);
  pr_info("Requested bytes to write: %zu\n",count);

  struct dev_private_data *device_data=(struct dev_private_data*)filp->private_data;//In open mtd we already saved
  //device private data(type cast it bcz it returns void)
  int max_size=device_data->size;
  if(*f_pos+count>max_size)
      count=max_size-*f_pos;
  if(!count) return -ENOMEM;
  if(copy_from_user(device_data->buffer+*f_pos,buff,count))
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
  struct dev_private_data *device_data=(struct dev_private_data*)filp->private_data;//In open mtd we already saved
  //device private data(type cast it bcz it returns void)
  int max_size=device_data->size;
  if(*f_pos+count>max_size)
      count=max_size-*f_pos;
  if(copy_to_user(buff,device_data->buffer+*f_pos,count))
    return -EFAULT;
  *f_pos+=count;
  pr_info("Succesfully read bytes : %zu\n",count);
  pr_info("Updated file pos is: %lld\n",*f_pos);
  return count;
}

static int __init hello_init(void)
{
        int ret=0;
        short i=0;
        alloc_chrdev_region(&drv_data.ex_devnum,0,MAX_DEV,"Pseudo_Device");
        if(ret<0) //static Major:Min allocation(Kernel picks major num  and stroes in dev_num
            {return ret;}
        drv_data.ex_class=class_create("Pseudo_Device_Class");
        if(IS_ERR(drv_data.ex_class))
        {
          pr_err("Cannot create Pseudo_Device_Class\n");
          goto unreg_dev;
        }
        for(short i=0;i<MAX_DEV;i++)
        {
          pr_info("Device Number Major: %d,  Minor: %d",MAJOR(drv_data.ex_devnum+i),MINOR(drv_data.ex_devnum+i));
          cdev_init(&drv_data.pcdev_data[i].ex_cdev,&ex_fops);
          drv_data.pcdev_data[i].ex_cdev.owner=THIS_MODULE;
          if(cdev_add(&drv_data.pcdev_data[i].ex_cdev,drv_data.ex_devnum+i,1)<0)
          {
            pr_err("Cannot add device to the system\n");
            goto unreg_dev;
          }
          drv_data.ex_dev=device_create(drv_data.ex_class,NULL,drv_data.ex_devnum+i,NULL,"Device-%d",i+1);
          if(IS_ERR(drv_data.ex_dev))
          {
             pr_err("Cannot create Device\n");
             goto class_des;
          } 
        }
	pr_info("Module Inserted\n");
	return 0;
unreg_dev:
    unregister_chrdev_region(drv_data.ex_devnum,1);
class_des:
for(;i>=0;i--)
    {
      device_destroy(drv_data.ex_class,drv_data.ex_devnum+i);
      cdev_del(&drv_data.pcdev_data[i].ex_cdev);
    }
    class_destroy(drv_data.ex_class);
    
    
    return -1;
}
static void __exit hello_CleanUp(void)
{ 
    short i;
    for(i=0;i<MAX_DEV;i++)
    {
      device_destroy(drv_data.ex_class,drv_data.ex_devnum+i);
      cdev_del(&drv_data.pcdev_data[i].ex_cdev);
    }
    unregister_chrdev_region(drv_data.ex_devnum,1);
    class_destroy(drv_data.ex_class);
    pr_info("Module Removed\n");
}
module_init(hello_init);
module_exit(hello_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A Simple Module");
MODULE_INFO(board,"For a Prog for Host machine");

