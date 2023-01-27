# Huion KD100 Linux Driver
A simple driver for the Huion KD100 mini Keydial written in C to give the device some usability while waiting for Huion to fix their Linux drivers. Each button can be configured to either act as a key/multiple keys or to execute a program

Installation
------------
Download the latest version, unzip it, and run KD100

Requirements
------------
- xdotool
> **_NOTE:_**  Will be removed in a future update

Usage
-----
```
sudo ./KD100 -c config_file -d
```
**-c**  Specify a config file (default.cfg is used normally)

**-d**  Enable debug output (can be used twice to output the full packet of data recieved from the device)

Notes
-----
- Only works on X11 based desktops
- You do not need to run this with sudo if you set a udev rule for the device. Create/edit a rule file in /etc/udev/rules.d/ and add the following, then save and reboot or reload your udev rules
```
SUBSYSTEM=="usb",ATTRS{idVendor}=="256c",ATTRS{idProduct}=="006d",MODE="0666",GROUP="plugdev"
```
- The output executable was meant to be a replacement for xdotool but ironically relies on it. It will be removed in a future update
- Technically speaking, this can support other devices, especially if they send the same type of byte information, otherwise the code should be easy enough to edit and add support for other usb devices
- New config files must have the same format and line count as the default file

Known Issues
------------
- The driver cannot trigger keyboard shortcuts from combining multiple buttons on the device
> **_NOTE:_**  Because of how the data is packaged, there currently is no work around for this

Building From Source
--------------------
```
gcc -lusb-1.0 KD100.c -o KD100
gcc -lxdo output.c -o output
```
