FROM ubuntu:18.04

ENV PATH /root/.cargo/bin:$PATH

RUN apt update && \
    apt install -y --no-install-recommends \
        software-properties-common \
        gpg-agent \
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
        python3 \
        imagemagick \
        libstdc++-arm-none-eabi-newlib \
        git \
        && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key| apt-key add - && add-apt-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic main"

RUN apt update && \
    apt install -y --no-install-recommends \
        libclang-12-dev \
        clang-format-12 \
        && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --profile=minimal --target thumbv7em-none-eabi thumbv7em-none-eabihf && \
    rustup component add rustfmt --toolchain stable-x86_64-unknown-linux-gnu

# st-flash

RUN apt update && \
    apt install -y --no-install-recommends \
    gcc build-essential cmake libusb-1.0 libusb-1.0-0-dev libgtk-3-dev pandoc \
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    wget https://github.com/stlink-org/stlink/archive/v1.5.1.zip && \
    unzip v1.5.1.zip && \
    cd stlink-1.5.1 && make clean && make release && \
    cd build/Release && make install && ldconfig

# gdb

RUN apt update && \
    apt install -y --no-install-recommends \
    python-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    wget http://mirrors.kernel.org/ubuntu/pool/main/r/readline6/libreadline6_6.3-8ubuntu2_amd64.deb && \
    dpkg -i libreadline6_6.3-8ubuntu2_amd64.deb && \
    wget http://mirrors.kernel.org/ubuntu/pool/universe/g/gdb-arm-none-eabi/gdb-arm-none-eabi_7.10-1ubuntu3+9_amd64.deb && \
    dpkg -i gdb-arm-none-eabi_7.10-1ubuntu3+9_amd64.deb

# dfu-util

RUN apt update && \
    apt install -y --no-install-recommends \
        dfu-util \
    && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

COPY entrypoint.sh syntax_check.sh /

RUN chmod +x /syntax_check.sh

ENTRYPOINT ["/entrypoint.sh"]
