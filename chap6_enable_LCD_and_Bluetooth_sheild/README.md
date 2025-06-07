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
