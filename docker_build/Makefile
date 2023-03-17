IMG=local/unleashed_firmware
CUSTOM_FLIPPER_NAME=myflipper
PORT=/dev/$(shell ls -l /dev/serial/by-id/ | grep 'Flipper' | grep -o 'tty.*')

.PHONY: all
all: .image_build

build: .image_build
	docker run --device $(PORT):$(PORT) --env CUSTOM_FLIPPER_NAME="${CUSTOM_FLIPPER_NAME}" ${IMG} ./fbt COMPACT=1 DEBUG=0 FORCE=1 flash_usb_full

dev:
	docker run --device $(PORT):$(PORT) -it ${IMG} bash

# Build Docker image
.image_build: Dockerfile
	docker build --tag ${IMG} .
	touch .image_build
	docker images

clean:
	rm -f .image_build
	if docker images | grep -q "${IMG}" ; then docker image rm ${IMG}; fi
	docker images
