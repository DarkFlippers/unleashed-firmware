#!/bin/bash

st-flash --reset write `dirname "$0"`/build/target_prod.bin 0x08008000
