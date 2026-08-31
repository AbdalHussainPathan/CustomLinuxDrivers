#include "main.h"
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
  mutex_lock(&buf_lock);
  device_open = false;
  mutex_unlock(&buf_lock);
  wake_up_interruptible(&read_wq);
  pr_info("close was success\n");
  return 0;
}
static int ex_open(struct inode *inode, struct file *filp)
{
  mutex_lock(&buf_lock);
  device_open = true;
  mutex_unlock(&buf_lock);
  pr_info("Open is called\n");
  return 0;
}
static ssize_t ex_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("write is called\n");
  pr_info("Current file pos is: %lld\n",*f_pos);
  pr_info("Requested bytes to write: %zu\n",count);

  if(mutex_lock_interruptible(&buf_lock))
    return -ERESTARTSYS;
  if(*f_pos+count>MAX_SIZE)
      count=MAX_SIZE-*f_pos;
  if(!count) 
  {
    mutex_unlock(&buf_lock);
    return -ENOMEM;
  }
  if(copy_from_user(&device_buff[*f_pos],buff,count))
  {
    mutex_unlock(&buf_lock);
    return -EFAULT;
  }
  *f_pos+=count;
  buf_len+=count;
  pr_info("Succesfully write bytes : %zu\n",count);
  pr_info("Updated file pos is: %lld\n",*f_pos);
  mutex_unlock(&buf_lock);
  wake_up_interruptible(&read_wq);
  return count;
}
static ssize_t ex_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    int ret;
  pr_info("read is called\n");
  pr_info("Current file pos is: %lld\n",*f_pos);
  pr_info("Requested bytes to read: %zu\n",count);
  // Wait safely until bytes are available or device closes
  ret=wait_event_interruptible(read_wq,buf_len>0||!device_open);
  if(ret) 
    return -EINTR;
  if(mutex_lock_interruptible(&buf_lock))
    return -ERESTARTSYS;
    //If the device is closed and no buffered data remains, safely exit out with EOF (0)
  if(!device_open&&buf_len==0)
  {
    mutex_unlock(&buf_lock);
    return 0;
  }
   if(buf_len==0)
  {
    mutex_unlock(&buf_lock);
    return 0;
  }
  if(*f_pos+count>MAX_SIZE)
      count=MAX_SIZE-*f_pos;
  if(copy_to_user(buff,&device_buff[*f_pos],count))
  {
    mutex_unlock(&buf_lock);
    return -EFAULT;
  }
  *f_pos+=count; 
   buf_len -= count;
   // Reset indices back down to zero when buffer matches exhaustion 
  if(buf_len == 0) {
      *f_pos = 0;
  }
  pr_info("Succesfully read bytes : %zu\n",count);
  pr_info("Updated file pos is: %lld\n",*f_pos);
  mutex_unlock(&buf_lock);
  return count;
}
static __poll_t ex_poll(struct file *filp, poll_table *wait)
{
    __poll_t mask = 0;
    poll_wait(filp, &read_wq, wait);  /* register with poll/select */
    mutex_lock(&buf_lock);
    if (buf_len > 0)
        mask |= EPOLLIN | EPOLLRDNORM;
    mutex_unlock(&buf_lock);
    return mask;
}
static int __init hello_init(void)
{
        int ret=0;
        //Initialized wait queue to prevent instant kernel null pointer hangs
         init_waitqueue_head(&read_wq);
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
