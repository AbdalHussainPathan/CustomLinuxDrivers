#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include <linux/version.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt,__func__ //check printk.h
#define MAX_SIZE 1024
#define MAX_DEV 6


#define RONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11
static int ex_open(struct inode *inode, struct file *filp);
static ssize_t ex_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
static ssize_t ex_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
static int ex_release (struct inode *inode, struct file *filp);



