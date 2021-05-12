PROJECT_ROOT	= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)
PROJECT			= firmware

include 		$(PROJECT_ROOT)/make/base.mk
include			$(PROJECT_ROOT)/assets/assets.mk
include			$(PROJECT_ROOT)/core/core.mk
include 		$(PROJECT_ROOT)/applications/applications.mk
include			$(PROJECT_ROOT)/lib/lib.mk

CFLAGS += -Werror -Wno-address-of-packed-member
CPPFLAGS += -Werror

TARGET			?= f5

TARGET_DIR		= targets/$(TARGET)

include			$(TARGET_DIR)/target.mk
CFLAGS			+= -Itargets/api-hal-include

include			$(PROJECT_ROOT)/make/git.mk
include			$(PROJECT_ROOT)/make/toolchain.mk
include			$(PROJECT_ROOT)/make/rules.mk
