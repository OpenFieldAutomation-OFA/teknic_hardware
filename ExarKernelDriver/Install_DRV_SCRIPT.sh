#!/bin/bash
# This script MUST be executed inside of root terminal: run ($ sudo su) to get there
# This script MUST be executed from the directory it resides in (ExarKernelDriver)
# i.e. uname@machinename:.../ExarKernelDriver$

# make sure we're running in the ExarKernelDriver directory
if [ "$($PWD | sed 's/^.*\///g')" = "ExarKernelDriver" ] ; then
    echo "Please navigate root terminal to the ExarKernelDriver directory and re-run this script."
    exit
fi

# unload the xr module if it is already loaded in the kernel
# this allows us to use our modified module
# if the module is already loaded this method doesn't affect anything
modprobe -r xr_usb_serial_common

# Run the driver makefile
make

# Copy diver module into modules
cp xr_usb_serial_common.ko /lib/modules/"$(uname -r)"/kernel/drivers/usb/serial

# Refresh modules
depmod

# make module load at boot time by adding "xr_usb_serial_common" on its own line to /etc/modules
echo "xr_usb_serial_common" | tee -a /etc/modules

# Load the module into kernel
insmod /lib/modules/"$(uname -r)"/kernel/drivers/usb/serial/xr_usb_serial_common.ko

# Prompt to connect
read -n 1 -s -r -p "Please insert the SC4-HUB if you haven't already and press any key to continue..."
echo

# Check if SC4 Device Connected
BUS_ID=""
if grep -q v2890p0213 /sys/bus/usb/devices/*/modalias; then
    # If device is connected, check what driver is bound to it
    # this gets the driver numbers just choose 1

    BUS_ID=$(grep v2890p0213 /sys/bus/usb/devices/*/modalias | head -1 | sed 's/\/sys\/bus\/usb\/devices\///g;s/\/.*//g')
    if [ -d "/sys/bus/usb/drivers/cdc_acm/$BUS_ID" ] ; then
        # need to unbind from acm
        echo -n "$BUS_ID" > /sys/bus/usb/drivers/cdc_acm/unbind      
    elif [ -d "/sys/bus/usb/drivers/cdc_xr_usb_serial/$BUS_ID" ]; then
        echo "The SC4-HUB is already bound to the XR driver. No need to rebind."
    elif [ -d "/sys/bus/usb/devices/$BUS_ID/driver" ]; then
        echo "$BUS_ID appears to be bound to another unrecognized driver."
        echo "Please unbind from this driver then re-run this script."
    fi
    if  [ ! -d "/sys/bus/usb/drivers/cdc_xr_usb_serial/$BUS_ID" ] ; then
    	# bind hub to the loaded xr driver
    	echo -n "$BUS_ID" > /sys/bus/usb/drivers/cdc_xr_usb_serial/bind
    fi
fi

cd ..

if [ ! -d "/sys/bus/usb/drivers/cdc_xr_usb_serial/$BUS_ID" ]; then
    # one last check
    echo "Something went wrong. It does not appear as though the SC4-HUB is bound to the XR driver."
else
    echo "Teknic driver install appears to have succeeded."
fi
