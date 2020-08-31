FROM ubuntu:18.04

RUN apt-get update && \
    apt install -y --no-install-recommends \
        make \
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        libnewlib-arm-none-eabi \
        autoconf \
        automake \
        libtool \
        curl \
        wget \
        g++ \
        unzip \
        build-essential \
        python \
        python-pip \
        libstdc++-arm-none-eabi-newlib \
        libclang-10-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --profile=minimal --target thumbv7em-none-eabi thumbv7em-none-eabihf

COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
