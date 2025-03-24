![Ubuntu](https://github.com/WasatchPhotonics/Wasatch.VCPP/raw/master/screenshots/ubuntu.png)

# Linux / POSIX instructions

Wasatch.VCPP was intentionally designed to use libusb-win32 on Windows to 
maximize compatibility with existing ENLIGHTEN and Wasatch.NET installations.  

However, obviously that won't work on Linux and other POSIX environments (MacOS 
etc), and happily there are modern libusb versions on most platforms which make 
sharing the basic source tree straightforward.

# Dependencies

- libusb-1.0
- libusb-1.0-dev (for libusb.h)

Example:

    $ sudo apt install libusb-1.0-0-dev

## udev

To access Wasatch spectrometers from "userland" on Linux (without running with 
sudo or root privs), you need to "grant user access" to our devices by installing
the following file:

    $ cd Wasatch.VCPP
    $ sudo cp udev/10-wasatch.rules /etc/udev/rules.d

After installing the file, you need to restart or "HUP" udev (simplest method is
just to reboot the computer).

# Building

    $ cd Wasatch.VCPP
    $ make
    $ cd demo-linux
    $ make
    $ ./demo
