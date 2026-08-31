//Linux Includes
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/mutex.h>
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
int buf_len=0;
static DEFINE_MUTEX(buf_lock);         /* protects buf & buf_len   */
static wait_queue_head_t read_wq;          /* readers sleep here       */
static bool            device_open = false;
static struct class *ex_class;
static struct device *ex_dev;
static struct cdev ex_cdev;
static int ex_open(struct inode *inode, struct file *filp);
static ssize_t ex_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
static ssize_t ex_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
static int ex_release (struct inode *inode, struct file *filp);
static long ex_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static __poll_t ex_poll(struct file *filp, poll_table *wait);
static struct file_operations ex_fops=
{
  .open=ex_open,
  .write=ex_write,
  .read=ex_read,
  .release=ex_release,
  .unlocked_ioctl=ex_ioctl,
  .owner=THIS_MODULE,
  .poll=ex_poll
};