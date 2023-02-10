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
sudo ./KD100 [options]
```
**-a**  Assume that the first device that matches the vid and pid is the keydial (skips prompt to select a device)

**-c**  Specify a config file to use after the flag (./default.cfg or ~/.config/KD100/default.cfg is used normally)

**-d**  Enable debug output (can be used twice to output the full packet of data recieved from the device)

**-h**  Displays a help message

Configuring
----------
Edit or copy **default.cfg** to add your own keys/commands and use the '-c' flag to specify the location of the config file
> **_NOTE:_**  New config files must have the same format and line count as the default file

Caveats
-------
- This only works on X11 based desktops (because of xdotool)
- You do not need to run this with sudo if you set a udev rule for the device. Create/edit a rule file in /etc/udev/rules.d/ and add the following, then save and reboot or reload your udev rules
```
SUBSYSTEM=="usb",ATTRS{idVendor}=="256c",ATTRS{idProduct}=="006d",MODE="0666",GROUP="plugdev"
```
- If the driver is ran as a user and the '-a' flag is not used, you will need to select the device to use during startup
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

