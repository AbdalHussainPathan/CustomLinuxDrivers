#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "platform.h"
#include <linux/mod_devicetable.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt,__func__ //check printk.h


int chk_permission(int dev_perm,int access_mode);
int pcd_platform_driver_probe(struct platform_device* pdev);
 void  pcd_platform_driver_remove(struct platform_device* pdev);
struct pcdev_private_data
{
  struct pcdev_platform_data pdata;
  char *buffer;
  dev_t dev_num;
  struct cdev cdev;
};
struct pcdrv_private_data
{
  int total_devices;
  /*This Holds the device number*/
  dev_t device_num_base;
  struct class *class_pcd;
  struct device *device_pcd;
};
struct pcdrv_private_data pcdrv_data;
struct device_config
{
    int config_item1;
    int config_item2;
};
enum pcdevs_names
{
    PCDEVA1X,
	PCDEVB1X,
	PCDEVC1X,
	PCDEVD1X
};
struct device_config pcdev_config[]=
{
    [PCDEVA1X]={.config_item1=60,.config_item2=21},
    [PCDEVB1X]={.config_item1=50,.config_item2=22},
    [PCDEVC1X]={.config_item1=40,.config_item2=23},
    [PCDEVD1X]={.config_item1=30,.config_item2=24}
};
struct platform_device_id pcdevs_ids[]=
{
    [0]={.name="pcdev-A1x",.driver_data=PCDEVA1X},  //MAtching by Id's
    [1]={.name="pcdev-B1x",.driver_data=PCDEVB1X},
    [2]={.name="pcdev-C1x",.driver_data=PCDEVC1X},
    [3] ={.name = "pcdev-D1x",.driver_data=PCDEVD1X},
    {}
};
struct platform_driver pcd_platform_driver=
{
  .probe=pcd_platform_driver_probe,
  .remove=pcd_platform_driver_remove,
  .id_table=pcdevs_ids,
  .driver=
  {
    .name="pseudo-char-device" //use same name as in Device_setup.c for matching mechanicsm to work
  }
};
int chk_permission(int dev_perm,int access_mode)
{
  if(dev_perm==RDWR)
    return 0;
  if((dev_perm==RDONLY)&&((access_mode&FMODE_READ)&&!(access_mode&FMODE_WRITE))) //RONLY access
    return 0;
  if((dev_perm==WRONLY)&&((access_mode&FMODE_WRITE)&&!(access_mode&FMODE_READ))) //WONLY access
    return 0;
  return -EPERM;
}
static loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    pr_info("seek requested\n");
    
    return 0;
}
static ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  
  pr_info("Read requested for %zu bytes\n",count);
  
	return 0;
}
static ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("Write requested for %zu bytes\n",count);
  
        return 0;
}
static int pcd_open (struct inode *inode, struct file *filp)
{
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
//Gets called when the platform device is match found
int pcd_platform_driver_probe(struct platform_device* pdev)//Chk proto type in include/linux/plaform_device.h
{
  int ret=0;
  struct pcdev_private_data *dev_data;
  struct pcdev_platform_data *pdata;
  pr_info("Device Detected\n");
  //1.Get the Platform data
  pdata=(struct pcdev_platform_data*)dev_get_platdata(&pdev->dev);
  if(!pdata)
  {
    pr_info("No platform data available\n");
    return -EINVAL;
  }
  //2. Dyanmically allocate memory fopr the device private data
 // dev_data=kzalloc(sizeof(struct pcdev_private_data),GFP_KERNEL);
 dev_data=devm_kzalloc(&pdev->dev,sizeof(*dev_data),GFP_KERNEL);
  if(!dev_data)
  {
    pr_info("Cannot allocte memory\n");
    return -ENOMEM;
  }
  //save the device private data pointer in platform device structure
  dev_set_drvdata(&pdev->dev,dev_data);
  
  dev_data->pdata.size=pdata->size;
  dev_data->pdata.perm=pdata->perm;
  dev_data->pdata.serial_number=pdata->serial_number;
  
  pr_info("Device Serial number= %s\n",dev_data->pdata.serial_number);
  pr_info("Device Permission= %d\n",dev_data->pdata.perm);
  pr_info("Device size= %d\n",dev_data->pdata.size);
   
  pr_info("Config item1 = %d\n",pcdev_config[pdev->id_entry->driver_data].config_item1);
  pr_info("Config item2 = %d\n",pcdev_config[pdev->id_entry->driver_data].config_item2);
   //3. Dynamically allocate device buffer using size info from platform data
   //dev_data->buffer=kzalloc(dev_data->pdata.size,GFP_KERNEL);
   dev_data->buffer=devm_kzalloc(&pdev->dev,dev_data->pdata.size,GFP_KERNEL);
  if(!dev_data->buffer)
  {
    pr_info("Cannot allocte memory\n");
    return -ENOMEM;
  }
  //4.Get the device number
  dev_data->dev_num=pcdrv_data.device_num_base+pdev->id;
  
  //5. Do cdev init and cdev add
  cdev_init(&dev_data->cdev,&pcd_fops);
  dev_data->cdev.owner=THIS_MODULE;
  ret=cdev_add(&dev_data->cdev,dev_data->dev_num,1);
  if(ret<0)
  {
    pr_err("Cdev add failed\n");
    return ret;
  }
  //6. Create device file for the detected platform device
  pcdrv_data.device_pcd=device_create(pcdrv_data.class_pcd,NULL,dev_data->dev_num,NULL,"pcdev-%d",pdev->id);
  if(IS_ERR(pcdrv_data.device_pcd))
  {
    pr_err("Device Create Failed\n");
    ret=PTR_ERR(pcdrv_data.device_pcd);
    cdev_del(&dev_data->cdev);
    return ret;
  }
  pcdrv_data.total_devices++;
  pr_info("Device Detected,Probe was success\n");
  return 0;
}

//Gets called when the device is removed from the system
void  pcd_platform_driver_remove(struct platform_device* pdev)//Chk proto type in include/linux/plaform_device.h
{
    struct pcdev_private_data *dev_data=dev_get_drvdata(&pdev->dev);
    //1. remove a device that was created with device_create()
    device_destroy(pcdrv_data.class_pcd,dev_data->dev_num);
    //2. remove a cdev entry from the system
    cdev_del(&dev_data->cdev);
    //3. Free the memory held by the device
    /*kfree(dev_data->buffer);
    kfree(dev_data);*/ //No need to Use kfree for devm_kzalloc
    pcdrv_data.total_devices--;
    pr_info("Device Removed\n");
  
}
#define MAX_DEVICES 10
static int __init pcd_driver_init(void)
{
//1. Dyanmically allocate a device number for MAX_DEVICES
      int ret;
      ret=alloc_chrdev_region(&pcdrv_data.device_num_base,0,MAX_DEVICES,"pcdevs"); //linux4.14/fs/char_dev.c
      if(ret<0)
      {
        pr_err("Alloc chr dev failed\n");
        return ret;
      }
      //2 . Create Device Class under /sys/class
       pcdrv_data.class_pcd = class_create("pcd_class"); 
        if(IS_ERR(pcdrv_data.class_pcd))
        {
          pr_err("Class Creation failed\n");
          ret=PTR_ERR(pcdrv_data.class_pcd);
          unregister_chrdev_region(pcdrv_data.device_num_base,MAX_DEVICES);
        }
  //3. Register a Platform Device
        platform_driver_register(&pcd_platform_driver);
        pr_info("Device Setup loaded\n");
	  return 0;
}
static void __exit pcd_driver_cleanup(void)
{

    platform_driver_unregister(&pcd_platform_driver);
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_num_base,MAX_DEVICES);

    pr_info("Device Setup unloaded\n");
}
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABDAL");
MODULE_DESCRIPTION("A pseudo character Driver for N Devices");
