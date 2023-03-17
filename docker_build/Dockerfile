FROM ubuntu:20.04

# Update/Dependency
RUN apt-get update; \
	apt-get install -y \
		git \
		curl \
		wget

# Create non-root user: "builder"
RUN useradd -m builder && \
    adduser builder dialout
USER builder
WORKDIR /home/builder 

# Clone flipper firmware from GitHub
RUN git clone --recurse-submodules https://github.com/DarkFlippers/unleashed-firmware.git
WORKDIR unleashed-firmware

USER root
RUN apt-get install -y vim
USER builder
