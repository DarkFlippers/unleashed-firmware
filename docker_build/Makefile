IMG=local/unleashed_firmware
CUSTOM_FLIPPER_NAME=myflipper
PORT=/dev/$(shell ls -l /dev/serial/by-id/ | grep 'Flipper' | grep -o 'tty.*')

# Build Docker image
img: .image_build
.image_build: Dockerfile
	docker build --tag ${IMG} .
	touch .image_build
	docker images

check_port:
	@echo "Checking if flipper device is connected somewhere in /dev ...."
	[ -c $(PORT) ] && echo "Flipper Connected!"

# Build and flash flipper
flash: img check_port
	docker run --device $(PORT):$(PORT) --env CUSTOM_FLIPPER_NAME="${CUSTOM_FLIPPER_NAME}" ${IMG} ./fbt COMPACT=1 DEBUG=0 FORCE=1 flash_usb_full

bash: img
	docker run --device $(PORT):$(PORT) -it ${IMG} bash

clean:
	rm -f .image_build
	if docker images | grep -q "${IMG}" ; then docker image rm ${IMG}; fi
	docker images

help:
	@echo "make [all|build|dev|clean|help]"
	@echo "    img  : Create docker image"
	@echo "    flash: Build and flash flipper"
	@echo "    bash : Launch shell in unleashed_firmware contianer"
	@echo "    clean: Remove docker image"
	@echo "    help : Print this help menu"
