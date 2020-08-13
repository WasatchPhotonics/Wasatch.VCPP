![Ubuntu](https://github.com/WasatchPhotonics/Wasatch.VCPP/raw/master/screenshots/ubuntu.png)

# Linux / POSIX instructions

Wasatch.VCPP was intentionally designed to use libusb-win32 on Windows to 
maximize compatibility with existing ENLIGHTEN and Wasatch.NET installations.  

However, obviously that won't work on Linux and other POSIX environments (MacOS 
etc), and happily there are modern libusb versions on most platforms which make 
sharing the basic source tree straightforward.

# Dependencies

- libusb-1.0

# Building

    $ cd Wasatch.VCPP
    $ make
    $ cd demo-linux
    $ make
    $ ./demo
