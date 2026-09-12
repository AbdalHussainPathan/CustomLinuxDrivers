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
#include <linux/version.h>
#define DRIVER_NAME			       "mpu6050"
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_WHO_AM_I		0x75
#define MPU6050_REG_ACCEL_XOUT_H	0x3B	/* first of 14 consecutive data regs */
#define MPU6050_BURST_LEN		    14	/* accel(6) + temp(2) + gyro(6) */
#define MPU6050_WHOAMI_MPU6050      0x68
#define MPU6050_WHOAMI_MPU6500      0x70
#define MPU6050_REG_PWR_MGMT_2		0x6C
#define DATA_ARR_SIZE 5
#define TIMEOUT 1000

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

//Functions
int I2C_init(struct Mpu_I2cDev *mpu);
void I2C_exit(struct Mpu_I2cDev *mpu);

static int mpu_probe(struct i2c_client *client);
static void mpu_remove(struct i2c_client *client);

static int open_mpu(struct inode *inode,struct file *filp);
static int close_mpu(struct inode *inode,struct file *filp);
static ssize_t write_mpu(struct file *filp,const char __user *buff,size_t count,loff_t *fpos);
static ssize_t read_mpu(struct file *filp, char __user *buff,size_t count,loff_t *fpos);

static void Timer_Callback(struct timer_list *t);
static void Work_Callback(struct work_struct *work);

struct file_operations fops=
{
    .read=read_mpu,
    .write=write_mpu,
    .release=close_mpu,
    .open=open_mpu,
    .owner=THIS_MODULE
};


