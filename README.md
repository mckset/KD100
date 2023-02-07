# Huion KD100 Linux Driver
A simple driver for the Huion KD100 mini Keydial written in C to give the device some usability while waiting for Huion to fix their Linux drivers. Each button can be configured to either act as a key/multiple keys or to execute a program/command

Pre-Installation
------------
Arch Linux:
```
sudo pacman -S libusb-1.0 xdotool
```
Ubuntu/Debian:
```
sudo apt-get install libusb-1.0 xdotool
```
> **_NOTE:_**  Some distros label libusb as "libusb-1.0-0" and others might require the separate "libusb-1.0-dev" package

Installation
------------
You can either download the latest release or run the following:
```
git clone https://github.com/mckset/KD100.git
cd KD100
make
```

Usage
-----
```
sudo ./KD100 -c config_file -d
```
**-c**  Specify a config file (default.cfg is used normally)

**-d**  Enable debug output (can be used twice to output the full packet of data recieved from the device)

Configuring
----------
Edit or copy **default.cfg** to add your own keys/commands 
> **_NOTE:_**  New config files must have the same format and line count as the default file

Caveats
-------
- This only works on X11 based desktops (because of xdotool)
- You do not need to run this with sudo if you set a udev rule for the device. Create/edit a rule file in /etc/udev/rules.d/ and add the following, then save and reboot or reload your udev rules
```
SUBSYSTEM=="usb",ATTRS{idVendor}=="256c",ATTRS{idProduct}=="006d",MODE="0666",GROUP="plugdev"
```
- Sometimes the driver won't find the device and might require you to unplug it and plug it back in to fix it
- Technically speaking, this can support other devices, especially if they send the same type of byte information, otherwise the code should be easy enough to edit and add support for other usb devices. If you want to see the information sent by different devices, change the vid and pid in the program and run it with two debug flags

Tested Distros
--------------
- Arch linux
- Manjaro
- Ubuntu
- Kali Linux

Known Issues
------------
- Setting shortcuts like "ctrl+c" will close the driver if it ran from a terminal and it's active
- The driver cannot trigger keyboard shortcuts from combining multiple buttons on the device
> Because of how the data is packaged, there currently is no work around for this

