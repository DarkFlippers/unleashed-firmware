FROM ubuntu:hirsute

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        ca-certificates \
        build-essential \
        python3 \
        git \
        clang-format-12 \
        dfu-util \
        openocd \
        libncurses5 \
        python-setuptools \
        libpython2.7-dev \
        libxml2-dev \
        libxslt1-dev \
        zlib1g-dev \
        wget \
        python3-protobuf \
        protobuf-compiler \
        python3-pip \
        libpython3-dev \
        ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN wget --progress=dot:giga "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07-$(uname -m)-linux.tar.bz2" && \
    tar xjf gcc-arm-none-eabi-10.3-2021.07-$(uname -m)-linux.tar.bz2 && \
    rm gcc-arm-none-eabi-10.3-2021.07-$(uname -m)-linux.tar.bz2 && \
    cd gcc-arm-none-eabi-10.3-2021.07/bin/ && \
    rm -rf ../share && \
    for file in * ; do ln -s "${PWD}/${file}" "/usr/bin/${file}" ; done && \
    cd / && arm-none-eabi-gcc -v && arm-none-eabi-gdb -v

RUN pip3 install heatshrink2==0.11.0 Pillow==9.1.1

RUN ln -s `which clang-format-12` /usr/local/bin/clang-format

COPY entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh"]
