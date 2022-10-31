#!/usr/bin/env bash

# Usage
# ./flash.sh <Device Port> <Path to Output Dir>
# ./flash.sh /dev/ttyUSB0 /path/to/output

if [ -z "$1" ]
  then
    echo "No device port provided"
    exit
fi

if [ -z "$2" ]
  then
    echo "No output directory provided"
    exit
fi

SDK_ROOT_DIR=$PWD/../../..
OUTPUT_DIR=$2

rm -rf log*
sudo ./Ameba_ImageTool -add device $1
sudo ./Ameba_ImageTool -set baudrate 1500000
sudo ./Ameba_ImageTool -set image $OUTPUT_DIR/flash_is.bin
sudo ./Ameba_ImageTool -show
read -p "Check if the settings are correct, then enter UART DOWNLOAD mode
1) Push the UART DOWNLOAD button and keep it pressed.
2) Re-power on the board or press the Reset button.
3) Release the UART DOWNLOAD button.
Once AmebaZ2 entered UART Download mode, press Enter to continue"
sudo ./Ameba_ImageTool -download
