GIT_COMMIT		:= $(shell git rev-parse --short HEAD || echo 'unknown')
GIT_BRANCH		:= $(shell echo $${WORKFLOW_BRANCH_OR_TAG-$$(git rev-parse --abbrev-ref HEAD || echo 'unknown')})
GIT_BRANCH_NUM	:= $(shell git rev-list --count HEAD || echo 'nan')
BUILD_DATE		:= $(shell date '+%d-%m-%Y' || echo 'unknown')
VERSION			:= $(shell git describe --tags --abbrev=0 --exact-match || echo 'unknown')

CFLAGS += \
	-DGIT_COMMIT=\"$(GIT_COMMIT)\" \
	-DGIT_BRANCH=\"$(GIT_BRANCH)\" \
	-DGIT_BRANCH_NUM=\"$(GIT_BRANCH_NUM)\" \
	-DBUILD_DATE=\"$(BUILD_DATE)\" \
	-DVERSION=\"$(VERSION)\" \
	-DTARGET=$(HARDWARE_TARGET)