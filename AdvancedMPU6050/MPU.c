#include "MPU.h"
#include <linux/timer.h>
void Timer_Callback(struct timer_list *data)
{
	 /* from_timer gives us back the containing struct */
	struct Mpu_I2cDev *mpu=timer_container_of(mpu,data,timer);
	schedule_work(&mpu->work);
	mod_timer(&mpu->timer, jiffies + msecs_to_jiffies(TIMEOUT));
}
void Work_Callback(struct work_struct *work)
{
	struct Mpu_I2cDev *mpu=container_of(work,struct Mpu_I2cDev,work);
	struct mpu6050_sample read_sample;
	u8 Raw_data[MPU6050_BURST_LEN];
	mutex_lock(&mpu->lock);
    short ret=mpu_readburst(mpu->client,Raw_data);
	if (ret < 0) 
	{
		mutex_unlock(&mpu->lock);
		pr_err("mpu6050: burst read failed (%d)\n", ret);
		return;
	}
	Decode_MPU(&read_sample,Raw_data);
	mpu->DataArr[mpu->write_idx]=read_sample;
	mod_timer(&mpu->timer, jiffies + msecs_to_jiffies(TIMEOUT));
	mutex_unlock(&mpu->lock);

}
static int mpu_read_reg(struct i2c_client *client,u8 reg)
{
	return i2c_smbus_read_byte_data(client,reg);
}
static int mpu_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	return i2c_smbus_write_byte_data(client, reg, val);
}
static int mpu_wakeup(struct i2c_client *client)
{
	/* PWR_MGMT_1: writing 0x00 clears the SLEEP bit and selects
	 * the default clock source - takes the chip out of sleep mode. */
	int ret=mpu_write_reg(client, MPU6050_REG_PWR_MGMT_1, 0x00);
	if(ret<0)
		return ret;
	/* PWR_MGMT_2: bring all accel/gyro axes out of standby.
	 * Chip was defaulting to 0x3F (all axes standby) on this board. */
	ret = mpu_write_reg(client, MPU6050_REG_PWR_MGMT_2, 0x00);
	if (ret < 0)
		return ret;
	msleep(50);
	return 0;
}
/* Burst-read all 14 data registers in one I2C transaction */
static int mpu_readburst(struct i2c_client *client, u8 *buf)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(client, MPU6050_REG_ACCEL_XOUT_H,MPU6050_BURST_LEN, buf);
	if (ret < 0)
		return ret;
	if (ret != MPU6050_BURST_LEN)
		return -EIO;
	return 0;
}
static void Decode_MPU(struct mpu6050_sample *sample, uint8_t *data)
{
	sample->accel_x=(s16)((data[0]<<8)|data[1]);
	sample->accel_y=(s16)((data[2]<<8)|data[3]);
	sample->accel_z=(s16)((data[4]<<8)|data[5]);
	sample->temp_raw=(s16)((data[6]<<8)|data[7]);
	sample->gyro_x=(s16)((data[8]<<8)|data[9]);
	sample->gyro_y=(s16)((data[10]<<8)|data[11]);
	sample->gyro_z=(s16)((data[12]<<8)|data[13]);
}
static int close_mpu (struct inode *inode, struct file *filp)
{
   pr_info("close was success\n");
  return 0;
}
static int open_mpu(struct inode *inode, struct file *filp)
{
  pr_info("mpu6050 : Open\n");
  filp->f_pos=0;
  return 0;
}
static ssize_t write_mpu(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("write is called\n");
  return count;
}
static ssize_t read_mpu(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  struct Mpu_I2cDev *mpu=pI2cMpu_Handle;
  struct mpu6050_sample read_sample;
  size_t to_copybytes;

  pr_info("read is called\n");

  if(!mpu||!mpu->client)
  	{return -ENODEV;}

  mutex_lock(&mpu->lock);
  read_sample=mpu->DataArr[mpu->Read_idx];
  mpu->Read_idx = (mpu->Read_idx + 1) % DATA_ARR_SIZE;
  mpu->msg_len=scnprintf(mpu->msg,sizeof(mpu->msg),
  "ax=%d ay=%d az=%d temp_raw=%d gx=%d gy=%d gz=%d\n",
  read_sample.accel_x,read_sample.accel_y,read_sample.accel_z,
  read_sample.temp_raw,read_sample.gyro_x,read_sample.gyro_y,read_sample.gyro_z);
  
  to_copybytes=min(count,mpu->msg_len);
  
  if(copy_to_user(buff,mpu->msg,to_copybytes))
  {
  	mutex_unlock(&mpu->lock);
	return -EFAULT;
  }
  mutex_unlock(&mpu->lock);
  *f_pos += to_copybytes;
  return to_copybytes;
}
int I2C_init(struct Mpu_I2cDev *mpu)
{
    int ret=0;
    ret=alloc_chrdev_region(&mpu->dev_num,0,1,"MPU");
    if(ret<0)
        return ret;
    cdev_init(&mpu->cdev_mpu,&fops);
    if(cdev_add(&mpu->cdev_mpu,mpu->dev_num,1)<0)
        goto unreg_dev;
    mpu->class_mpu=class_create("Class_MPU");
    if(IS_ERR(mpu->class_mpu))
        goto cdev_del;
    mpu->device_mpu=device_create(mpu->class_mpu,NULL,mpu->dev_num,NULL,"MPU_6050");
    if(IS_ERR(mpu->device_mpu))
        goto class_destroy;
    pr_info("Major %d : Minor %d",MAJOR(mpu->dev_num),MINOR(mpu->dev_num));

    return 0;
class_destroy:
    class_destroy(mpu->class_mpu);
cdev_del:
    cdev_del(&mpu->cdev_mpu); 
unreg_dev:
    unregister_chrdev_region(mpu->dev_num,1);
    return -1;
}
void I2C_exit(struct Mpu_I2cDev *mpu)
{
    device_destroy(mpu->class_mpu,mpu->dev_num);
    unregister_chrdev_region(mpu->dev_num,1);
    cdev_del(&mpu->cdev_mpu);
    class_destroy(mpu->class_mpu);
}
static int mpu_probe(struct i2c_client *client)
{
	struct Mpu_I2cDev *pMpu_Dev;
	int ret=0;
	int nWhoami=mpu_read_reg(client,MPU6050_REG_WHO_AM_I);
	if(nWhoami<0)
	{
	   return nWhoami;
	}
	if (nWhoami != MPU6050_WHOAMI_MPU6050 && nWhoami != MPU6050_WHOAMI_MPU6500)
	{
		dev_err(&client->dev, "Expected 0x68 or 0x70 instead got 0x%02x", nWhoami);
		return -ENODEV;
	}
	pMpu_Dev=devm_kzalloc(&client->dev,sizeof(*pMpu_Dev),GFP_KERNEL);
	if(pMpu_Dev==NULL)
	{
		return -ENOMEM;
	}
	pMpu_Dev->client=client;
	mutex_init(&pMpu_Dev->lock);
	pMpu_Dev->write_idx=0;
	pMpu_Dev->Read_idx=0;
	i2c_set_clientdata(client,pMpu_Dev);
	pI2cMpu_Handle=pMpu_Dev;

	ret=mpu_wakeup(client);
	if(ret<0)
	{
		dev_err(&client->dev, "failed to wake device (%d)\n", ret);
		return ret;
	}
	ret=I2C_init(pMpu_Dev);
	if(ret<0)
	{
		dev_err(&client->dev, "failed to init char device (%d)\n", ret);
		return ret;
	}
	//Work item runs in process context
	INIT_WORK(&pMpu_Dev->work,Work_Callback);
	 /* setup your timer to call my_timer_callback */
    timer_setup(&pMpu_Dev->timer, Timer_Callback, 0);
	mod_timer(&pMpu_Dev->timer,jiffies +msecs_to_jiffies(TIMEOUT));
	dev_info(&client->dev, "mpu6050 driver probe complete\n");
	return ret;	
}
static void mpu_remove(struct i2c_client *client)
{
	struct Mpu_I2cDev *mpu=i2c_get_clientdata(client);
	if(mpu)
	{
		I2C_exit(mpu);
		if(mpu==pI2cMpu_Handle)
			pI2cMpu_Handle=NULL;
	}
	dev_info(&client->dev, "mpu6050 driver removed\n");
}

/*
 * Matches when a client is created with this name - either via
 * manual sysfs new_device echo ("mpu6050 0x68") or a DT node with
 * compatible = "invensense,mpu6050" (once you add the of_match_table).
 */
static const struct i2c_device_id mpu6050_id[] = {
	{ "mpu6050", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mpu6050_id);
static struct i2c_driver mpu_driver=
{
	.driver=
	{
	  .name=DRIVER_NAME
	},
	.probe=mpu_probe,
	.remove=mpu_remove,
	.id_table=mpu6050_id
};

module_i2c_driver(mpu_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABDAL");
MODULE_DESCRIPTION("MPU6050 Char Driver");
MODULE_INFO(board,"Prog for BeagleBone");