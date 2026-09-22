#include "Nrf24Char.h"

static int open_nrf(struct inode *inode,struct file *file)
{

    return 0;
}
static int close_nrf(struct inode *inode,struct file *file)
{
    return 0;
}
static ssize_t write_nrf(struct file *filp,const char __user*buff,size_t len,loff_t *fpos)
{
    return 0;
}
static ssize_t read_nrf(struct file *filp, char __user *buff,size_t len,loff_t *fpos)
{
    return len;
}


int init_NrfDev(void)
{

    int ret=alloc_chrdev_region(&nrf_dev->nrf_devnum,0,1,"Nrf_spi");
    if(ret<0)
    {
        return ret;
    }
    cdev_init(&nrf_dev->nrf_cdev,&fops);
    if(cdev_add(&nrf_dev->nrf_cdev,nrf_dev->nrf_devnum,1)<0)
    {
        goto unreg_dev;
    }
    nrf_dev->nrf_class=class_create("class_Nrf");
    if(IS_ERR(nrf_dev->nrf_class))
    {  
        goto cdev_del;
    }
     nrf_dev->nrf_device=device_create(nrf_dev->nrf_class,NULL,nrf_dev->nrf_devnum,NULL,DRIVER_NAME);
     if(IS_ERR(nrf_dev->nrf_device))
     {
        goto class_des;
     }
    pr_info("Major %d : Minor %d",MAJOR(nrf_dev->nrf_devnum),MINOR(nrf_dev->nrf_devnum));
     return 0;
class_des:
     class_destroy(nrf_dev->nrf_class);
cdev_del:
     cdev_del(&nrf_dev->nrf_cdev);
unreg_dev:
     unregister_chrdev_region(nrf_dev->nrf_devnum,1);
return -1;
    
}
void exit_NrfDev(void)
{
    device_destroy(nrf_dev->nrf_class,nrf_dev->nrf_devnum);
    class_destroy(nrf_dev->nrf_class);
    cdev_del(&nrf_dev->nrf_cdev);
    unregister_chrdev_region(nrf_dev->nrf_devnum,1);
    pr_info("NRF24 module Removed\n");
}

/*struct spi_driver {
	const struct spi_device_id *id_table;
	int			(*probe)(struct spi_device *spi);
	void			(*remove)(struct spi_device *spi);
	void			(*shutdown)(struct spi_device *spi);
	struct device_driver	driver;
};
*/
void nrf_init(void)
{
    nrf_write_reg(CONFIG,0);//Will be configured later
    nrf_write_reg(EN_AA,0);//No auto ACK
    nrf_write_reg(EN_RXADDR,0);////NOt Enabling any data piperight now
    nrf_write_reg(SETUP_AW,0x03);//set up addr 5 bytes
    nrf_write_reg(RF_SETUP,0X0E);//Setup output power and Data rate=2mbps
    nrf_write_reg(RF_CH,0);//
    nrf_write_reg(SETUP_RETR,0);//No Retransmission
    nrf_write_reg(CONFIG,0);//Will be configured later

}
int	nrf_probe(struct spi_device *spi)
{
    //nrf_init();
    pr_info("NRF Spi Detected\n");
    nrf_write_reg(CONFIG,0x08);
    char buff[256];
    if(nrf_read_reg(CONFIG,buff)<0)
    {
        pr_info("Cannot Read CONFIG Reg\n");
    }
    return 0;
}
void nrf_remove(struct spi_device *spi)
{
    pr_info("NRF Spi Removed\n");
}
void nrf_shutdown(struct spi_device *spi)
{

    pr_info("NRF Spi Shutdown\n");
}
static const struct spi_device_id nrf_spi_id[] =
{
    {"nrf_spi",0},
    {}
};
MODULE_DEVICE_TABLE(spi,nrf_spi_id);
static struct spi_driver nrf_driver=
{
    .driver=
    {
        .name=DRIVER_NAME
    },
    .id_table=nrf_spi_id,
    .probe=nrf_probe,
    .remove=nrf_remove,
    .shutdown=nrf_shutdown
};
module_spi_driver(nrf_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABDAL");
MODULE_DESCRIPTION("NRF24L01 Char Driver");
MODULE_INFO(board,"Prog for BeagleBone");