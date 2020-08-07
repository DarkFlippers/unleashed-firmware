FROM ubuntu:18.04

RUN apt-get update && apt-get install -y --no-install-recommends \
make gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi \
autoconf automake libtool curl wget g++ unzip && \
apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN apt-get update && apt-get install -y --no-install-recommends \
python python-pip libstdc++-arm-none-eabi-newlib && \
apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
