FROM ubuntu:18.04

ENV PATH /root/.cargo/bin:$PATH

RUN apt update && \
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
        libclang-10-dev \
        clang-format-10 \
        && apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --profile=minimal --target thumbv7em-none-eabi thumbv7em-none-eabihf && \
    rustup component add rustfmt --toolchain stable-x86_64-unknown-linux-gnu

COPY entrypoint.sh syntax_check.sh /

RUN chmod +x /syntax_check.sh

ENTRYPOINT ["/entrypoint.sh"]
