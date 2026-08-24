#include<linux/module.h>
#include<linux/device.h>
#include<linux/printk.h>

dev_t dev_num;
static struct class *dev_class;
static int __init hello_init(void)
{
        int ret=0;
        alloc_chrdev_region(&dev_num,0,1,"DyanmicAlloc");
        if(ret<0) //static Major:Min allocation(Kernel picks major num  and stroes in dev_num
            {return ret;}
        dev_class=class_create("Example_Class");
        if(IS_ERR(dev_class))
        {
          pr_err("Cannot create class\n");
          goto unreg_dev;
        }
        if(IS_ERR(device_create(dev_class,NULL,dev_num,NULL,"Example_Device")))
        {
           pr_err("Cannot create class\n");
           goto class_des;
        }
        pr_info("Major: %d,  Minor: %d",MAJOR(dev_num),MINOR(dev_num));
	pr_info("Module Inserted\n");
	return 0;
unreg_dev:
    unregister_chrdev_region(dev_num,1);
class_des:
    class_destroy(dev_class);
    return -1;
}
static void __exit hello_CleanUp(void)
{
    device_destroy(dev_class,dev_num);
    unregister_chrdev_region(dev_num,1);
    class_destroy(dev_class);
    pr_info("Module Removed\n");
}
module_init(hello_init);
module_exit(hello_CleanUp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A Simple Module");
MODULE_INFO(board,"For a Prog for Host machine");

