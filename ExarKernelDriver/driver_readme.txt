

                     Installing SC4-Hub USB Driver on Linux
                                   2021-06-11


                                       0
                             Background Information
Your system might have a built-in cdc_acm driver that will take control of the
SC4-Hub. That driver is adequate for establishing communications with the board,
but it is NOT fully compatible with sFoundation. For full support, install the
Exar USB Serial driver packaged here.

This exact procedure has been verified on the following distributions. The 
general concepts should apply to nearly all Linux systems.
 - Ubuntu 20.04 (x64)
 - Fedora Workstation 33 (x64)
 - Debian 10.9 (x64)
 - Raspberry Pi OS (buster) (arm)
 - Ubuntu 18.04 on BeagleBone Black (arm)
 - elementary OS 5.1 (Hera)

You will need root permission to follow this procedure.

                                       1
                               Install Procedure
 1. Make sure the kernel headers are installed on your system.
    
    Debian/Ubuntu/Raspberry Pi: 
    Headers are typically stored in /usr/src/linux-headers-{kernelVersion}.
    See if they exist using ls.
    $ ls /usr/src/linux-headers-$(uname -r)

    If they don't exist, install them using apt.
    $ sudo apt install linux-headers-$(uname -r)

    For Raspberry Pi, simply install the raspberrypi-kernel-headers
    package.
    $ sudo apt install raspberrypi-kernel-headers

    Fedora:
    Headers are typically stored in /usr/src/kernels/{kernelVersion}. See
    if they exist using ls.
    $ ls /usr/src/kernels/$(uname -r)

    In Fedora, you can install the kernel-devel package with yum.
    $ sudo yum install kernel-devel-$(uname -r)

 2. If you're reading this, you've probably already extracted the tarball. In 
    case you didn't, extract it, then change to the ExarKernelDriver directory.
    $ tar -xvf Teknic_SC4Hub_USB_Driver.tar
    $ cd ExarKernelDriver

 3. Make sure Install_DRV_SCRIPT.sh is an executable
    $ chmod +x Install_DRV_SCRIPT.sh

 4. Enter Root Terminal and Run the Driver Install script
    $ sudo su
    $ ./Install_DRV_SCRIPT.sh

 > Note if the script fails or if you'd prefer, open up the script and read through it and execute the lines one by one
 > the script is commented to explain the purpose of all the instructions
