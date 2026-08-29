#include "Defines.h"
char device1_buff[MAX_SIZE];
char device2_buff[MAX_SIZE];
char device3_buff[MAX_SIZE];
char device4_buff[MAX_SIZE];
char device5_buff[MAX_SIZE];
char device6_buff[MAX_SIZE];
struct dev_private_data
{
  unsigned  size;
  int perm;
  char *buffer;
  const char* Dev_Name;
  struct cdev ex_cdev;
};
struct drv_private_data
{
  int total_dev;
  dev_t ex_devnum;
  struct class *ex_class;
  struct device *ex_dev;
  struct dev_private_data pcdev_data[MAX_DEV];
};
struct drv_private_data drv_data=
{
  .total_dev=MAX_DEV,
  .pcdev_data=
  {
    [0]=
    {
      .size=MAX_SIZE,
      .perm=RONLY,
      .Dev_Name="Device1",
      .buffer=device1_buff
    },
    [1]=
    {
      .size=MAX_SIZE,
      .perm=RONLY,
      .Dev_Name="Device2",
      .buffer=device2_buff
    },
     [2]=
    {
      .size=MAX_SIZE,
      .perm=WRONLY,
      .Dev_Name="Device3",
      .buffer=device3_buff
    },
     [3]=
    {
      .size=MAX_SIZE,
      .perm=WRONLY,
      .Dev_Name="Device4",
      .buffer=device4_buff
    },
     [4]=
    {
      .size=MAX_SIZE,
      .perm=RDWR,
      .Dev_Name="Device4",
      .buffer=device4_buff
    },
     [5]=
    {
      .size=MAX_SIZE,
      .perm=RDWR,
      .Dev_Name="Device5",
      .buffer=device5_buff
    }
    }
};
static struct file_operations ex_fops=
{
  .open=ex_open,
  .write=ex_write,
  .read=ex_read,
  .release=ex_release,
  .owner=THIS_MODULE
};
