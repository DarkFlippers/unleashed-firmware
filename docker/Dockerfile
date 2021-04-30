FROM ubuntu:18.04

# hadolint ignore=DL3008
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        software-properties-common \
        gpg-agent \
        make \
        cmake \
        autoconf \
        automake \
        libtool \
        libusb-1.0 \
        libusb-1.0-0-dev \
        libgtk-3-dev pandoc \
        curl \
        wget \
        g++ \
        gcc \
        unzip \
        build-essential \
        python \
        python-pip \
        python3 \
        imagemagick \
        srecord \
        git \
        pkg-config \
        dfu-util \
        && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

SHELL ["/bin/bash", "-eo", "pipefail", "-c"]
RUN wget --progress=dot:giga -O - "https://apt.llvm.org/llvm-snapshot.gpg.key" | apt-key add - && add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-12 main"

# hadolint ignore=DL3008
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libclang-12-dev \
        clang-format-12 \
        && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# install arm-none-eabi

# hadolint ignore=DL3003
RUN wget --progress=dot:giga "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2" && \
    tar xjf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 && \
    rm gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 && \
    cd gcc-arm-none-eabi-10-2020-q4-major/bin/ && \
    for file in * ; do ln -s "${PWD}/${file}" "/usr/bin/${file}" ; done && \
    cd / && arm-none-eabi-gcc -v && arm-none-eabi-gdb -v
    
# install hex2dfu

# hadolint ignore=DL3003
RUN wget --progress=dot:giga "https://github.com/rusdacent/hex2dfu/archive/master.zip" --output-document=hex2dfu.zip && \
    unzip hex2dfu.zip && \
    rm hex2dfu.zip && \
    cd hex2dfu-master && gcc hex2dfu.c ED25519/*.c -o hex2dfu && \
    ln -s "${PWD}/hex2dfu" /usr/bin/hex2dfu

# install openocd

# hadolint ignore=DL3003
RUN git clone --depth 1 --branch v0.11.0 https://github.com/ntfreak/openocd.git && \
    cd openocd && ./bootstrap && ./configure && make install && \
    cd .. && rm -rf openocd && \
    openocd --version

COPY entrypoint.sh syntax_check.sh /

RUN chmod +x /syntax_check.sh

ENTRYPOINT ["/entrypoint.sh"]
