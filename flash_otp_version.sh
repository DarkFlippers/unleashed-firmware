#!/bin/bash

set -x -e

if [ "$#" -ne 1 ]; then
    echo "OTP file required"
    exit
fi

if [ ! -f $1 ]; then
    echo "Unable to open OTP file"
    exit
fi

STM32_Programmer_CLI -c port=swd -d $1 0x1FFF7000

STM32_Programmer_CLI -c port=swd -r8 0x1FFF7000 8
