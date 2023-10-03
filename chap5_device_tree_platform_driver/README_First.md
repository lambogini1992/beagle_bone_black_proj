1. Follow the link below to set-up beagle bone board:
    https://embetronicx.com/tutorials/linux/device-drivers/setup-beaglebone-board-linux-device-driver-tutorial/

2. Apply the patch change for the source code.

3. Build the dtb overlay again to get CUSTOM_GPIOS.dtbo file

4. Copy it in /lib/firmware folder on beagle board.

5. Add the line in uEnv.txt on BB board.
    uboot_overlay_addr0==lib/firmware/CUSTOM_GPIOS.dtbo

6. Reboot BB board.

7. Go to folder /proc/device-tree and check custom_gpio node is append.

8. Build module kernel and get custom_gpio.ko

9. Copy custom_gpio.ko to BB board.

10. Install custom_gpio.ko

11. Check /dev/custom_gpio is append. that mean we can load system.
