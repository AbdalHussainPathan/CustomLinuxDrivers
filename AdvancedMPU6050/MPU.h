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
#define DRIVER_NAME			       "mpu6050"
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_WHO_AM_I		0x75
#define MPU6050_REG_ACCEL_XOUT_H	0x3B	/* first of 14 consecutive data regs */
#define MPU6050_BURST_LEN		    14	/* accel(6) + temp(2) + gyro(6) */
#define MPU6050_WHOAMI_MPU6050      0x68
#define MPU6050_WHOAMI_MPU6500      0x70
#define MPU6050_REG_PWR_MGMT_2		0x6C
#define DATA_ARR_SIZE 				5
#define TIMEOUT 					1000
#define MPU_IOC_MAGIC  'M'
#define MPU_IOC_SET_TIMEOUT   _IOW(MPU_IOC_MAGIC, 1, uint32_t)
#define MPU_IOC_RESET_BUF  _IO(MPU_IOC_MAGIC, 2)

DECLARE_WAIT_QUEUE_HEAD(BufferFull_Queue);
//MPU Helpers
struct mpu6050_sample {
	s16 accel_x, accel_y, accel_z;
	s16 temp_raw;
	s16 gyro_x, gyro_y, gyro_z;
};
static int mpu_readburst(struct i2c_client *client, u8 *buf);
static int mpu_read_reg(struct i2c_client *client,u8 reg);
static int mpu_wakeup(struct i2c_client *client);
static int mpu_write_reg(struct i2c_client *client, u8 reg, u8 val);
static void Decode_MPU(struct mpu6050_sample *sample, uint8_t *data);
//Device
struct Mpu_I2cDev
{
	char msg[256];
	size_t msg_len;
	struct mutex lock;
	dev_t dev_num;
	struct cdev cdev_mpu;
	struct class *class_mpu;
	struct device *device_mpu;
	struct i2c_client *client;

	struct work_struct work;
	struct timer_list timer;
	struct  mpu6050_sample  DataArr[DATA_ARR_SIZE];

	u8 write_idx;// next slot the workqueue will fill
	u8 Read_idx; //read pos
};
struct Mpu_I2cDev *pI2cMpu_Handle;
//Vars
unsigned int nTimeout=0;
//Functions
int I2C_init(struct Mpu_I2cDev *mpu);
void I2C_exit(struct Mpu_I2cDev *mpu);

static int mpu_probe(struct i2c_client *client);
static void mpu_remove(struct i2c_client *client);

static int open_mpu(struct inode *inode,struct file *filp);
static int close_mpu(struct inode *inode,struct file *filp);
static ssize_t write_mpu(struct file *filp,const char __user *buff,size_t count,loff_t *fpos);
static ssize_t read_mpu(struct file *filp, char __user *buff,size_t count,loff_t *fpos);
static unsigned int poll_mpu(struct file *filp, struct poll_table_struct *pt);
static long ioctl_mpu(struct file *filp,unsigned int cmd,unsigned long arg);
static void Timer_Callback(struct timer_list *t);
static void Work_Callback(struct work_struct *work);

struct file_operations fops=
{
    .read=read_mpu,
    .write=write_mpu,
    .release=close_mpu,
    .open=open_mpu,
	.poll=poll_mpu,
	.unlocked_ioctl=ioctl_mpu,
    .owner=THIS_MODULE
};
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

