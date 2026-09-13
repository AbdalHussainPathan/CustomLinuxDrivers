# MPU6050 Linux Char Driver (BeagleBone Black)

A Linux char device driver for the MPU6050/MPU6500 IMU over I2C, built on
a BeagleBone Black (Yocto, kernel 6.6.21-bone-standard). Built to go past
basic open/read/write/lseek and get real experience with blocking I/O,
wait queues, poll/select, workqueues, and ioctl - built in stages, each
verified on hardware before adding the next.

## Architecture

- **I2C** - registers as an `i2c_driver`, binds via `probe()` when a
  `mpu6050` client appears (currently instantiated manually via sysfs).
- **Sampling** - a kernel timer triggers a **workqueue**, which does the
  actual I2C burst read (14 bytes: accel/temp/gyro) and pushes the
  decoded sample into a ring buffer.
- **Char device** (`/dev/MPU_6050`) - `read()` blocks until a new sample
  arrives; `.poll` shares the same wait queue for `poll()`/`select()`.
- **ioctl** - in progress (runtime sample-rate change, buffer reset).

**Not yet done:** device tree overlay (client is manual for now),
per-fd read position (single shared index currently), mmap, and a
second minor device for motion-event detection.

## Real bugs hit while building this

1. **Wrong chip assumed** - expected MPU6050 (WHO_AM_I 0x68), board
   actually had an MPU6500 (0x70). Fixed by accepting both.
2. **Built-in kernel driver (`inv-mpu6050-i2c`) grabbed the I2C client
   first**, so my `probe()` never ran. Had to unbind it manually and
   rebind mine - still needs a permanent fix (blacklist or DT edit).
3. **All-zero accel/gyro** despite a successful burst read. Traced with
   raw `i2cget` to `PWR_MGMT_2` defaulting to `0x3F` (all axes in
   standby) - clearing the sleep bit alone wasn't enough.
4. **`BUG: scheduling while atomic` kernel panic** - was doing the I2C
   read directly inside the timer callback, which runs in atomic
   (softirq) context where sleeping is illegal, and I2C transfers sleep
   internally. Fixed by moving the actual read into a workqueue; the
   timer now only schedules work and re-arms itself.
5. **Reader racing an unsynchronized producer** - before blocking I/O,
   `read()` just returned whatever was in the buffer, unrelated to
   whether the timer had actually produced anything new.
6. **Deadlock** - first blocking-I/O attempt held the mutex *while*
   calling `wait_event_interruptible()`, so the producer could never
   get the same lock to write new data and wake the reader. `cat` hung
   forever. Fixed by waiting before acquiring the lock.
7. **Wrong wait condition** - used `write_idx >= DATA_ARR_SIZE`
   ("buffer full"), which is wrong for a continuous stream. Replaced
   with `write_idx != read_idx` (standard producer/consumer check).

## Hardware

BeagleBone Black, MPU6050/MPU6500 breakout over I2C (bus 2, addr 0x68).

## Building

```
make
sudo insmod MPU.ko
echo mpu6050 0x68 | sudo tee /sys/bus/i2c/devices/i2c-2/new_device
cat /dev/MPU_6050
```

## Status

Core driver (I2C bring-up, char device, blocking read, poll/select)
working and tested on hardware. ioctl in progress.
