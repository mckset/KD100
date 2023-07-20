# Huion KD100 Linux Driver
A simple driver for the Huion KD100 mini Keydial written in C to give the device some usability while waiting for Huion to fix their Linux drivers. Each button can be configured to either act as a key/multiple keys or to execute a program/command

> **_NOTICE:_**  When updating from **v1.31** or below, make sure you updated your config file to follow the new format shown in the default config file

Pre-Installation
------------
Arch Linux/Manjaro:
```
sudo pacman -S libusb-1.0 xdotool
```
Ubuntu/Debian/Pop OS:
```
sudo apt-get install libusb-1.0-0-dev xdotool
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

> Running make as root will install the driver as a command and create a folder in ~/.config to store config files

Usage
-----
```
sudo ./KD100 [options]
```
**-a**  Assume that the first device that matches the vid and pid is the keydial (skips prompt to select a device)

**-c**  Specify a config file to use after the flag (./default.cfg or ~/.config/KD100/default.cfg is used normally)

**-d**  Enable debug output (can be used twice to output the full packet of data recieved from the device)

**-dry**  Display data sent from the keydial and ignore events

**-h**  Displays a help message

Configuring
----------
Edit or copy **default.cfg** to add your own keys/commands and use the '-c' flag to specify the location of the config file. New config files do not need to end in ".cfg".

Caveats
-------
- This only works on X11 based desktops (because it relies on xdotool) but can be patched for wayland desktops by altering the "handler" function
- You do not need to run this with sudo if you set a udev rule for the device. Create/edit a rule file in /etc/udev/rules.d/ and add the following, then save and reboot or reload your udev rules
```
SUBSYSTEM=="usb",ATTRS{idVendor}=="256c",ATTRS{idProduct}=="006d",MODE="0666",GROUP="plugdev"
```
- Technically speaking, this can support other devices, especially if they send the same type of byte information, otherwise the code should be easy enough to edit and add support for other usb devices. If you want to see the information sent by different devices, change the vid and pid in the program and run it with two debug flags

Tested Distros
--------------
- Arch linux
- Manjaro
- Ubuntu
- Pop OS

Known Issues
------------
- Setting shortcuts like "ctrl+c" will close the driver if it ran from a terminal and it's active
- The driver cannot trigger keyboard shortcuts from combining multiple buttons on the device
> Because of how the data is packaged, there currently is no work around for this

