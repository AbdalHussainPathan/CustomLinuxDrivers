For DT:
    Build it: make src/arm/BB-NRF24-SPI1-00A0.dtbo.
    Copy it to /lib/firmware/ and /boot/dtbs/$(uname -r)/overlays/.
    In uEnv.txt, comment out addr5 and addr6, then add uboot_overlay_addr5=BB-NRF24-SPI1-00A0.dtbo.
    Reboot.