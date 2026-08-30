Sample Char Device for open,read,write

Testing:
	Compile first (make host)
	sudo insmod main.ko
	sudo chmod 666 /dev/Example_Device
	sudo chown abdal:abdal /dev/Example_Device
	sudo echo 1> /dev/Example_Device
	sudo cat /dev/Example_Device
	gcc -o ./bin/Test_app Test_ioctl.c
then 
	sudo dmesg | tail
	
output
	[  624.091880] write is called
	[  624.093513] close was success
	[  632.356347] Open is called
	[  632.356364] read is called
	[  632.356375] close was success
output for ioctl
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/004$ sudo ./Test_app
	Enter the value to be Written
	34
	Value Written is 34
	Read Value from Driver 34
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/004$ sudo dmesg | tail
	[  990.240616] ex_ioctl: arg is: 140720524518464
	[  990.240618] ex_ioctl: IOCTL CMD WR_ONLY is called
	[  990.240619] ex_ioctl: Written value is 34
	[  990.240621] ex_ioctl: IOCTL is called
	[  990.240621] ex_ioctl: cmd is: 2147770722
	[  990.240623] ex_ioctl: arg is: 140720524518460
	[  990.240624] ex_ioctl: IOCTL CMD RD_ONLY is called
	[  990.240625] ex_ioctl: value is Read 34
	[  990.240636] ex_release: close was success
	[  996.951939] workqueue: blk_mq_requeue_work hogged CPU for >10000us 7 times, consider switching to WQ_UNBOU
	
