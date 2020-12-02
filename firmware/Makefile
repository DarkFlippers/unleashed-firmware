PROJECT_ROOT	= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)
PROJECT			= firmware

CFLAGS += -Werror
CPPFLAGS += -Werror

include 		$(PROJECT_ROOT)/make/base.mk
include			$(PROJECT_ROOT)/assets/assets.mk
include			$(PROJECT_ROOT)/core/core.mk
include			$(PROJECT_ROOT)/make/git.mk
include 		$(PROJECT_ROOT)/applications/applications.mk
include			$(PROJECT_ROOT)/lib/lib.mk

TARGET			?= f2

# TODO temporary for migrating f3.1 to f4
ifeq ($(TARGET), f4)
TARGET_DIR = targets/f3
else
TARGET_DIR		= targets/$(TARGET)
endif

include			$(TARGET_DIR)/target.mk
CFLAGS			+= -Itargets/api-hal-include

include			$(PROJECT_ROOT)/make/git.mk
include			$(PROJECT_ROOT)/make/toolchain.mk
include			$(PROJECT_ROOT)/make/rules.mk
