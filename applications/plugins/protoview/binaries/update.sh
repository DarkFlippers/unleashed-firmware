#!/bin/sh
BINPATH="/Users/antirez/hack/flipper/official/build/f7-firmware-D/.extapps/protoview.fap"
cp $BINPATH .
git commit -a -m 'Binary file updated.'
