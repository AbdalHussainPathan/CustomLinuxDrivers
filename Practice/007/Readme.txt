Sample Char Device for open,read,write

Testing:
	Compile first (make host)
	sudo insmod main.ko
	sudo chmod 666 /dev/Example_Device
	sudo chown abdal:abdal /dev/Example_Device

abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ make host
make -C /lib/modules/7.0.0-30-generic/build M=/home/abdal/BBB_Workspace/LDD/custom_drivers/Practice/007 modules
make[1]: Entering directory '/usr/src/linux-headers-7.0.0-30-generic'
make[2]: Entering directory '/home/abdal/BBB_Workspace/LDD/custom_drivers/Practice/007'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
warning: pahole version differs from the one used to build the kernel
  The kernel was built with: 125
  You are using:             0
  CC [M]  wait_event_timeout.o
  MODPOST Module.symvers
  CC [M]  wait_event_timeout.mod.o
  LD [M]  wait_event_timeout.ko
  BTF [M] wait_event_timeout.ko
Skipping BTF generation for wait_event_timeout.ko due to unavailability of vmlinux
make[2]: Leaving directory '/home/abdal/BBB_Workspace/LDD/custom_drivers/Practice/007'
make[1]: Leaving directory '/usr/src/linux-headers-7.0.0-30-generic'
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo insmod wait_event_timeout.ko
[sudo] password for abdal: 
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  196.299566] workqueue: vmstat_shepherd hogged CPU for >10000us 4 times, consider switching to WQ_UNBOUND
[  213.216966] workqueue: vmstat_shepherd hogged CPU for >10000us 5 times, consider switching to WQ_UNBOUND
[  234.005904] workqueue: e1000_watchdog [e1000] hogged CPU for >10000us 67 times, consider switching to WQ_UNBOUND
[  253.263580] workqueue: vmstat_shepherd hogged CPU for >10000us 7 times, consider switching to WQ_UNBOUND
[  344.116245] workqueue: vmstat_shepherd hogged CPU for >10000us 11 times, consider switching to WQ_UNBOUND
[  422.533506] Created ThreadFunction
[  422.533555] Device Driver Inserted!
[  422.533623] Waiting for event!
[  424.541339] Timeout!
[  424.541345] Waiting for event!
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo rmmod wait_event_timeout.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail^C
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ ^C
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ ^C
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ make host
make -C /lib/modules/7.0.0-30-generic/build M=/home/abdal/BBB_Workspace/LDD/custom_drivers/Practice/007 modules
make[1]: Entering directory '/usr/src/linux-headers-7.0.0-30-generic'
make[2]: Entering directory '/home/abdal/BBB_Workspace/LDD/custom_drivers/Practice/007'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
  You are using:           gcc-13 (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
warning: pahole version differs from the one used to build the kernel
  The kernel was built with: 125
  You are using:             0
  CC [M]  wait_event_timeout.o
  CC [M]  wait_event_cmd.o
  CC [M]  wait_event_interruptible.o
  MODPOST Module.symvers
  LD [M]  wait_event_timeout.ko
  BTF [M] wait_event_timeout.ko
Skipping BTF generation for wait_event_timeout.ko due to unavailability of vmlinux
  CC [M]  wait_event_cmd.mod.o
  LD [M]  wait_event_cmd.ko
  BTF [M] wait_event_cmd.ko
Skipping BTF generation for wait_event_cmd.ko due to unavailability of vmlinux
  CC [M]  wait_event_interruptible.mod.o
  LD [M]  wait_event_interruptible.ko
  BTF [M] wait_event_interruptible.ko
Skipping BTF generation for wait_event_interruptible.ko due to unavailability of vmlinux
make[2]: Leaving directory '/home/abdal/BBB_Workspace/LDD/custom_drivers/Practice/007'
make[1]: Leaving directory '/usr/src/linux-headers-7.0.0-30-generic'
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo insmod wait_event_timeout.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  441.686078] Thread exiting cleanly
[  441.686888] Removed Device Driver
[  455.386218] workqueue: e1000_watchdog [e1000] hogged CPU for >10000us 131 times, consider switching to WQ_UNBOUND
[  678.137051] Created ThreadFunction
[  678.137124] Device Driver Inserted!
[  678.137229] Waiting for event!
[  680.159798] Timeout!
[  680.159823] Waiting for event!
[  682.203932] Timeout!
[  682.203950] Waiting for event!
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo cat /dev/ex_device
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  694.485914] Waiting for event!
[  696.533901] Timeout!
[  696.533920] Waiting for event!
[  697.311558] Open is called
[  697.311575] Read is Called
[  697.311641] close is called
[  697.311719] Thread Function called from ex_read function
[  697.311725] Waiting for event!
[  699.347763] Timeout!
[  699.347784] Waiting for event!
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo rmmod wait_event_timeout.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo insmod wait_event_cmd.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  711.630234] Timeout!
[  711.630254] Waiting for event!
[  713.675513] Timeout!
[  713.675655] Waiting for event!
[  714.814216] Thread exiting cleanly
[  714.815945] Removed Device Driver
[  733.114835] Created ThreadFunction
[  733.114891] Device Driver Inserted!
[  733.114959] Waiting for event!
[  733.114964] Func1 called
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo cat /dev/ex_device
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  733.114891] Device Driver Inserted!
[  733.114959] Waiting for event!
[  733.114964] Func1 called
[  760.470083] Open is called
[  760.470099] Read is Called
[  760.470186] close is called
[  760.470263] Func2 called
[  760.470269] Thread Function called from ex_read function
[  760.470271] Waiting for event!
[  760.470272] Func1 called
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo rmmod wait_event_cmd.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo insmod wait_event_interruptible.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  760.470263] Func2 called
[  760.470269] Thread Function called from ex_read function
[  760.470271] Waiting for event!
[  760.470272] Func1 called
[  779.089698] Func2 called
[  779.090214] Removed Device Driver
[  783.000732] workqueue: vmstat_shepherd hogged CPU for >10000us 19 times, consider switching to WQ_UNBOUND
[  794.612006] Created ThreadFunction
[  794.612141] Device Driver Inserted!
[  794.612505] Waiting for event!
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo cat /dev/ex_device
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo rmmod wait_event_interruptible.ko
abdal@abdal-VirtualBox:~/BBB_Workspace/LDD/custom_drivers/Practice/007$ sudo dmesg | tail
[  783.000732] workqueue: vmstat_shepherd hogged CPU for >10000us 19 times, consider switching to WQ_UNBOUND
[  794.612006] Created ThreadFunction
[  794.612141] Device Driver Inserted!
[  794.612505] Waiting for event!
[  809.143728] Open is called
[  809.143743] Read is Called
[  809.143815] close is called
[  809.144302] Thread Function called from ex_read function
[  809.144306] Waiting for event!
[  820.996794] Removed Device Driver