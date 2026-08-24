#include<linux/module.h>
#include<linux/device.h>
#include<linux/printk.h>
#define MAJOR_NUM 235 
#define MINOR_NUM 6
#define DYANAMIC_ALLOC 1
dev_t dev_num;
static int __init hello_init(void)
{
        int ret=0;
        #if DYANAMIC_ALLOC==0
          dev_num=MKDEV(MAJOR_NUM,MINOR_NUM); //Creating the dev with our custom major ,minor numbers
          register_chrdev_region(dev_num,1,"StaticAlloc")
          if(ret<0) //static Major:Min allocation
            {return ret;}
        #else
          alloc_chrdev_region(&dev_num,0,1,"DyanmicAlloc");
          if(ret<0) //static Major:Min allocation(Kernel picks major num  and stroes in dev_num
            {return ret;}
        #endif
        pr_info("Major: %d,  Minor: %d",MAJOR(dev_num),MINOR(dev_num));
	pr_info("Module Inserted\n");
	return ret;
}

static void __exit hello_CleanUp(void)
{
    unregister_chrdev_region(dev_num,1);
    pr_info("Module Removed\n");
}
module_init(hello_init);
module_exit(hello_CleanUp);

MODULE_LICENSE("GPL V2");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A Simple Module");
MODULE_INFO(board,"For a Prog for Host machine");

