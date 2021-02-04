#!/bin/bash

set -x -e

STM32_Programmer_CLI -c port=swd -ob RDP=0xBB

STM32_Programmer_CLI -c port=swd -ob displ

STM32_Programmer_CLI -c port=swd --readunprotect

STM32_Programmer_CLI -c port=swd -ob displ
