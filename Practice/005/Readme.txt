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
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo insmod Drv_Dev.ko
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo cat /dev/Device-5
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo echo "Hi What's Up" >> /dev/Devi
	ce-1
	bash: /dev/Device-1: Permission denied
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo dmesg | tail
	[127597.074898] ex_read: Current file pos is: 0
	[127597.074899] ex_read: Requested bytes to read: 131072
	[127597.074902] ex_read: Succesfully read bytes : 1024
	[127597.074903] ex_read: Updated file pos is: 1024
	[127597.075027] ex_read: read is called
	[127597.075029] ex_read: Current file pos is: 1024
	[127597.075030] ex_read: Requested bytes to read: 131072
	[127597.075032] ex_read: Succesfully read bytes : 0
	[127597.075033] ex_read: Updated file pos is: 1024
	[127597.075048] ex_release: close was success
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo echo "Hi What's Up" >> /dev/Device-5
	bash: /dev/Device-5: Permission denied
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo dmesg | tail
	[127597.074898] ex_read: Current file pos is: 0
	[127597.074899] ex_read: Requested bytes to read: 131072
	[127597.074902] ex_read: Succesfully read bytes : 1024
	[127597.074903] ex_read: Updated file pos is: 1024
	[127597.075027] ex_read: read is called
	[127597.075029] ex_read: Current file pos is: 1024
	[127597.075030] ex_read: Requested bytes to read: 131072
	[127597.075032] ex_read: Succesfully read bytes : 0
	[127597.075033] ex_read: Updated file pos is: 1024
	[127597.075048] ex_release: close was success
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo echo "Hi What's Up" >> /dev/Device-5^C
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ echo "Hi How you are?" | sudo tee /dev/Device-5
	Hi How you are?
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ sudo dmesg | tail
	[127597.075048] ex_release: close was success
	[127768.435240] ex_open: Open is called
	[127768.435252] ex_open: Minor Access=4
	[127768.435254] ex_open: open was Success
	[127768.435273] ex_write: write is called
	[127768.435275] ex_write: Current file pos is: 0
	[127768.435276] ex_write: Requested bytes to write: 16
	[127768.435279] ex_write: Succesfully write bytes : 16
	[127768.435280] ex_write: Updated file pos is: 16
	[127768.435284] ex_release: close was success
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/005$ 

	
