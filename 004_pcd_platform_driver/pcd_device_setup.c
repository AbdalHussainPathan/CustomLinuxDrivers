#include<linux/module.h>
#include<linux/platform_device.h>
#include"platform.h"
void pcdev_release(struct device *dev);
void pcdev_release(struct device *dev)
{
  pr_info ("Device released\n");
}
//1. create 2 platform data
struct pcdev_platform_data pcdev_pdata[]=
{
  [0]={.size=512,.perm=RDWR,.serial_number="PCDEVABC1111"},
  [1]={.size=1024,.perm=RDWR,.serial_number="PCDEVABC2222"},
  [2]={.size=128,.perm=RDONLY,.serial_number="PCDEVABC3333"},
  [3]={.size=32,.perm=WRONLY,.serial_number="PCDEVABC4444"}
  
};
//2. Create 2 platform device
struct platform_device platform_pcdev_1=
{
	.name="pcdev-A1x",//"pseudo-char-device",
	.id=0, //this can be used as idx purpose i.e this device is at idx 0
	.dev={
	  .platform_data=&pcdev_pdata[0],
	  .release=pcdev_release
	  }
};
struct platform_device platform_pcdev_2 =  
{ 
        .name="pcdev-B1x",//"pseudo-char-device",
        .id=1,
        .dev={
	  .platform_data=&pcdev_pdata[1],
	  .release=pcdev_release
	  }
};
struct platform_device platform_pcdev_3 =  
{ 
        .name="pcdev-C1x",//"pseudo-char-device",
        .id=2,
        .dev={
	  .platform_data=&pcdev_pdata[2],
	  .release=pcdev_release
	  }
};
struct platform_device platform_pcdev_4 =  
{ 
        .name="pcdev-D1x",//"pseudo-char-device",
        .id=3,
        .dev={
	  .platform_data=&pcdev_pdata[3],
	  .release=pcdev_release
	  }
};
struct platform_device *platform_pcdevs[]=
{
  &platform_pcdev_1,
  &platform_pcdev_2,
  &platform_pcdev_3,
  &platform_pcdev_4 
};
static int __init pcdev_platform_init(void)
{
  //register platform device
  //platform_device_register(&platform_pcdev_1); //Adding Single Device
  //platform_device_register(&platform_pcdev_2);
  platform_add_devices(platform_pcdevs,ARRAY_SIZE(platform_pcdevs));
  pr_info ("Module Inserted\n");
	return 0;
}
static void __exit pcdev_platform_exit(void)
{ 
//platform_device_unregister(&platform_pcdev_1);
//platform_device_unregister(&platform_pcdev_2);
for (short i = ARRAY_SIZE(platform_pcdevs) - 1; i >= 0; i--) {
        platform_device_unregister(platform_pcdevs[i]);
    }
  pr_info ("Module Removed\n");
}
module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module which registers platform devices");
