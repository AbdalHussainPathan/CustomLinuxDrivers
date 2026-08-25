Sample Char Device for open,read,write

Testing:
	Compile first (make host)
	sudo insmod main.ko
	sudo chmod 666 /dev/Example_Device
	sudo chown abdal:abdal /dev/Example_Device
	sudo echo 1> /dev/Example_Device
	sudo cat /dev/Example_Device

then 
	sudo dmesg | tail
	
output
	[  624.091880] write is called
	[  624.093513] close was success
	[  632.356347] Open is called
	[  632.356364] read is called
	[  632.356375] close was success

	
