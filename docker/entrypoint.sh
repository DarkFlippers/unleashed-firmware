#!/bin/bash

# A hack for GitHub Actions to not install Rust twice
if [ "$HOME" != "/root" ]; then
    ln -sf /root/.rustup "$HOME/.rustup"
    ln -sf /root/.cargo "$HOME/.cargo"
fi

PATH="$HOME/.cargo/bin:${PATH}"

if [ -z "$1" ]; then
    bash
else
    echo "Running $1"
    set -ex
    bash -c "$1"
fi
