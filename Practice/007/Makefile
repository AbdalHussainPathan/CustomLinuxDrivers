obj-m += wait_event_interruptible.o

# Target Directories
ARM_KERN_DIR  = /home/vboxuser/BBB_Workspace/LDD/source/linux-5.10.168-ti-rt-r76
HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build

# ARM Cross-Compiler Config
ARM_ARCH = arm
ARM_CROSS = arm-linux-gnueabihf-

# Default rule (prints help usage)
all:
	@echo "Please specify a target:"
	@echo "  make host  - Build for the Ubuntu Host PC"
	@echo "  make arm   - Build for the BeagleBone Black (ARM)"
	@echo "  make clean - Remove build artifacts"

# Ubuntu Host PC Build
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

# ARM Target Build
arm:
	make ARCH=$(ARM_ARCH) CROSS_COMPILE=$(ARM_CROSS) -C $(ARM_KERN_DIR) M=$(PWD) modules

# Clean Artifacts
clean:
	make -C $(HOST_KERN_DIR) M=$(PWD) clean
	@if [ -d "$(ARM_KERN_DIR)" ]; then \
		make ARCH=$(ARM_ARCH) CROSS_COMPILE=$(ARM_CROSS) -C $(ARM_KERN_DIR) M=$(PWD) clean; \
	fi

