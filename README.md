# Huion KD100 Linux Driver
A simple driver for the Huion KD100 mini Keydial written in C to give the device some usability while waiting for Huion to fix their Linux drivers. Each button can be configured to either act as a key/multiple keys or to execute a program

Installation
------------
Download the latest version, unzip it, and run KD100

Usage
-----
```
sudo ./KD100 -c config_file -d
```
**-c**  Specify a config file (default.cfg is used normally)

**-d**  Enable debug output (can be used twice to output the full packet of data recieved from the device)

> **_KEEP IN MIND_**  Very few distros have been tested so far. If you have a problem on your distro, please report it and I will try to get to it as soon as possible

Notes
-----
- Only works on X11 based desktops
- You do not need to run this with sudo if you set a udev rule for the device. Create/edit a rule file in /etc/udev/rules.d/ and add the following, then save and reboot or reload your udev rules
```
SUBSYSTEM=="usb",ATTRS{idVendor}=="256c",ATTRS{idProduct}=="006d",MODE="0666",GROUP="plugdev"
```
- Sometimes the driver won't find the device and might require you to unplug it and plug it back in to fix it
- Technically speaking, this can support other devices, especially if they send the same type of byte information, otherwise the code should be easy enough to edit and add support for other usb devices
- New config files must have the same format and line count as the default file

Known Issues
------------
- Setting shortcuts like "ctrl+c" will close the driver if it ran from a terminal and is active
- The driver cannot trigger keyboard shortcuts from combining multiple buttons on the device
> **_NOTE:_**  Because of how the data is packaged, there currently is no work around for this

Building From Source
--------------------
Clone the repo and run compile.sh

--------------------
Special thanks to the creator of [xdotool](https://github.com/jordansissel/xdotool). Even though this project is not dependant on the tool, it utilizes some of the xdo scripts from that project 
