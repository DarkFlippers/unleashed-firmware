#!/bin/bash

set -x -e

COPRO_DIR="lib/STM32CubeWB/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x"


STM32_Programmer_CLI -c port=swd -fwupgrade $COPRO_DIR/stm32wb5x_FUS_fw_1_0_2.bin 0x080EC000 || true
STM32_Programmer_CLI -c port=swd

STM32_Programmer_CLI -c port=swd -fwupgrade $COPRO_DIR/stm32wb5x_FUS_fw.bin 0x080EC000 || true
STM32_Programmer_CLI -c port=swd

STM32_Programmer_CLI -c port=swd -fwdelete

STM32_Programmer_CLI -c port=swd -fwupgrade $COPRO_DIR/stm32wb5x_BLE_Stack_full_fw.bin 0x080CB000 firstinstall=0

STM32_Programmer_CLI -c port=swd -ob nSWBOOT0=1 nBOOT0=1

