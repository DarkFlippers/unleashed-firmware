#!/bin/bash

dfu-util -D `dirname "$0"`/build/target_prod.bin -a 0 -s 0x08008000
