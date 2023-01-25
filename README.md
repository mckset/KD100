# KD100 Linux Driver
A simple driver for the Huion KD100 mini keypad written in C to give the device some usability while waiting for Huion to fix their Linux drivers

Usage
-----
```
sudo ./KD100 -c config_file -d
```
**-c**  Specify a config file (default.cfg is used normally)

**-d**  Enable debug output (can be used twice to output the full packet of data recieved from the device)


Notes
-----
- You do not need to run this as root if you set a udev rule for the device. Create/edit a file in /etc/udev/rules.d/ and add the following, then save and reboot or reload your udev rules
```
SUBSYSTEM=="usb",ATTRS{idVendor}=="256c",ATTRS{idProduct}=="006d",MODE="0666",GROUP="plugdev"
```
- The output executable is only there to prevent users from needing to download xdotool. Why is it not part of the main program? Every time I tried to add it, it crashed the program without giving a reason and I couldn't find a work around. 
- Technically speaking, this can support other devices, especially if they send the same type of byte information, but the code should be easy enough to edit if you wanted to use it with some other usb device
- New config files must contain the same format and line count as the default file

Known Issues
------------
- The driver cannot trigger keyboard shorcuts using key/buttons pressed from another device
- The driver cannot trigger keyboard shortcuts from combining multiple buttons on the device

Work Arounds
------------
Both issues might be able to be worked arounds by creating a script that can send key inputs pair with keys/buttons currently being held
