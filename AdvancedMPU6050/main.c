#include<linux/module.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/version.h>
int __init I2C_init(void);
void __exit I2C_exit(void);

dev_t dev_num;
struct cdev cdev_mpu;
struct class *class_mpu;
struct device *device_mpu;

static int open_mpu(struct inode *inode,struct file *filp);
static int close_mpu(struct inode *inode,struct file *filp);
static ssize_t write_mpu(struct file *filp,const char __user *buff,size_t count,loff_t *fpos);
static ssize_t read_mpu(struct file *filp, char __user *buff,size_t count,loff_t *fpos);
struct file_operations fops=
{
    .read=read_mpu,
    .write=write_mpu,
    .release=close_mpu,
    .open=open_mpu,
    .owner=THIS_MODULE
};
static int close_mpu (struct inode *inode, struct file *filp)
{
   pr_info("close was success\n");
  return 0;
}
static int open_mpu(struct inode *inode, struct file *filp)
{
  pr_info("Open is called\n");

  return 0;
}
static ssize_t write_mpu(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("write is called\n");
  return count;
}
static ssize_t read_mpu(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("read is called\n");
  return 0;
}
int __init I2C_init(void)
{
    int ret=0;
    ret=alloc_chrdev_region(&dev_num,0,1,"MPU");
    if(ret<0)
        return ret;
    cdev_init(&cdev_mpu,&fops);
    if(cdev_add(&cdev_mpu,dev_num,1)<0)
        goto unreg_dev;
    class_mpu=class_create("Class_MPU");
    if(IS_ERR(class_mpu))
        goto cdev_del;
    device_mpu=device_create(class_mpu,NULL,dev_num,NULL,"MPU_6050");
    if(IS_ERR(device_mpu))
        goto class_destroy;
    pr_info("Major %d : Minor %d",MAJOR(dev_num),MINOR(dev_num));

    return 0;
class_destroy:
    class_destroy(class_mpu);
cdev_del:
    cdev_del(&cdev_mpu); 
unreg_dev:
    unregister_chrdev_region(dev_num,1);
    return -1;
}
void __exit I2C_exit(void)
{
    device_destroy(class_mpu,dev_num);
    unregister_chrdev_region(dev_num,1);
    cdev_del(&cdev_mpu);
    class_destroy(class_mpu);
}
module_init(I2C_init);
module_exit(I2C_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABDAL");
MODULE_DESCRIPTION("MPU6050 Char Driver");
MODULE_INFO(board,"Prog for BeagleBone");