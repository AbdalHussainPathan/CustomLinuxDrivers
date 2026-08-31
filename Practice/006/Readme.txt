Sample Char Device for open,read,write

Testing:
	Compile first (make host)
	sudo insmod main.ko
	sudo chmod 666 /dev/Example_Device
	sudo chown abdal:abdal /dev/Example_Device

Terminal 1:
	sudo cat /dev/Example_Device
Terminal 2:
	sudo echo "Abdal"> /dev/Example_Device
	

then 
	sudo dmesg | tail
	
output
Terminal 1:(execution halts waits for buf to be filled then print out)
	abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ sudo insmod main.ko
[sudo] password for abdal: 
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ sudo cat /dev/Example_Device
Abdal
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ 

Terminal 2:	
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ sudo echo "Abdal" >> /dev/Example_Device
bash: /dev/Example_Device: Permission denied
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ sudo chmod 666 /dev/Example_Device
        sudo chown abdal:abdal /dev/Example_Device
[sudo] password for abdal: 
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ sudo echo "Abdal" >> /dev/Example_Device
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/006$ sudo dmesg | tail
[ 1391.870773] ex_read: Succesfully read bytes : 1024
[ 1391.870777] ex_read: Updated file pos is: 1024
[ 1391.871634] ex_read: read is called
[ 1391.871637] ex_read: Current file pos is: 1024
[ 1391.871639] ex_read: Requested bytes to read: 131072
[ 1391.872497] ex_release: close was success
[ 1391.872543] ex_read: Succesfully read bytes : 0
[ 1391.872547] ex_read: Updated file pos is: 1024
[ 1391.872580] ex_release: close was success
[ 1404.483525] workqueue: e1000_watchdog [e1000] hogged CPU for >10000us 515 times, consider switching to WQ_UNBOUND
