#include<linux/module.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include<linux/poll.h>
#include <linux/version.h>
#include<linux/spi/spi.h>
#include <linux/gpio.h>
#include "NrfDefines.h"
#define DRIVER_NAME  "NRF24_SPI"
#define NRF_CE_GPIO 115
static int open_nrf(struct inode *inode,struct file *file);
static int close_nrf(struct inode *inode,struct file *file);
static ssize_t write_nrf(struct file *filp,const char __user*buff,size_t len,loff_t *fpos);
static ssize_t read_nrf(struct file *filp, char __user *buff,size_t len,loff_t *fpos);
int init_NrfDev(void);
void exit_NrfDev(void);
void nrf_init(void);
static int nrf_read_reg(char reg,char *buff);
static int nrf_write_reg(char reg,char data);

int			nrf_probe(struct spi_device *spi);
void			nrf_remove(struct spi_device *spi);
void			nrf_shutdown(struct spi_device *spi);
struct nrf_dev
{
    struct cdev nrf_cdev;
    struct device *nrf_device;
    struct class *nrf_class;
    dev_t  nrf_devnum;
    struct spi_device *spi_dev;
};
struct nrf_dev *nrf_dev;
static int nrf_write_reg(char reg,char data)
{
    /*W_REGISTER 001A AAAA 1 to 5
    LSByte first
    Write command and status registers. AAAAA = 5
    bit Register Map Address
    Executable in power down or standby modes
    only*/
   /*
   /* uint8_t buf[2];
    buf[0]=reg|(1<<5); 
    buf[1]=data;
    return spi_write(nrf_dev->spi_dev,buf,2);*/
    u8 buf[2] = { W_REGISTER | (reg & 0x1F), data };
    int ret =spi_write(nrf_dev->spi_dev, buf, 2);
    return ret;
    

}
static int nrf_read_reg(char reg,char *buff)
{
    //return (spi_write_then_read(nrf_dev->spi_dev,&reg,1,buff,1)<0?-1:0);
    u8 cmd = R_REGISTER | (reg & 0x1F);
    u8 buf[2];
    int ret = spi_write_then_read(nrf_dev->spi_dev, &cmd, 1, buff, 1);
    if (ret < 0)
        return ret;
    return 0;
}
struct file_operations fops=
{
    .open=open_nrf,
    .release=close_nrf,
    .write=write_nrf,
    .read=read_nrf,
    .owner=THIS_MODULE
};