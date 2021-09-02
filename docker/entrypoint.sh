#!/bin/bash

if [ -z "$1" ]; then
    bash
else
    echo "Running $1"
    set -ex
    bash -c "$1"
fi
