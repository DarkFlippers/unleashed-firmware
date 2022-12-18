ARG distrib=debian:11
FROM $distrib AS base

# Configure base system packages
RUN apt-get update && apt-get install -y curl wget vim git python3 python3-pip build-essential

FROM base AS builder
# Setup builder user
ARG user=builder
ARG branch=builder
ARG commit=origin/dev
ENV PATH = "${PATH}:/home/${user}/.local/bin"

RUN useradd -ms /bin/bash ${user}
RUN mkdir /dist
ADD . /src
WORKDIR /src
RUN chown -R ${user}:${user} /src /dist
USER ${user}

# Add python requierements
RUN python3 -m pip install pip --upgrade; python3 -m pip install -r scripts/requirements.txt

# Checkout commit to new build branch
# reset code from commit
# remove untracked files
# reinit submodules from commit
# reset submodule code from commit
# remove untracked sumobules files
RUN git checkout --force -B $branch $commit && \
    git reset --hard && \
    git clean -ffdx && \
    git submodule update --init --recursive --force && \
    git submodule foreach git reset --hard && \
    git submodule foreach git clean -ffdx

# build and compress firmware
RUN ./fbt; ./fbt COMPACT=1 DEBUG=0 updater_package
RUN tar czvf ./dist/flipper-z-f7-update-$(git rev-parse --short $commit).tgz -C dist/f7-C/ f7-update-local
