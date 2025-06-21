To testing display LCD we should use xserver to display it
   1st: Install xserver package
	sudo apt install xserver-xorg xinit xserver-xorg-input-libinput
   2nd: Start xserver
	startx(If you're facing permission issue, please run with sudo permission)
   3rd: Create xstart.service unit file when starting system
	please refer to file content and move it into /etc/systemd/system/startx.service
   4th: Reload deamon
	sudo systemctl -u deamon-reexce
	sudo systemctl -u deamon-reload
	sudo systemctl enable startx.service
   5th: Reboot system and wait result


To testing CYW device we should use blueZ stack for this purpose;
	1st: Install BlueZ stack and bluetooth utils tools:
		 sudo apt update && sudo apt install bluez
		 sudo systemctl start bluetooth
		 sudo systemctl enable bluetooth
	2nd : Copy Firmware CYW43012 to /etc/firmware/brcm/
	3rd : Power on and wake device
		echo 1 > /sys/bus/platform/drivers/bluesleep/bluetooth/power_state #Power On device
		echo 1 > /sys/bus/platform/drivers/bluesleep/bluetooth/wake_state #Wake up device
	4th : Start initializing bluetooth device by hciattach tools:
		sudo hciattach /dev/ttyS5 bcm43xx 15200 flow
	5th : Scan and configue device
