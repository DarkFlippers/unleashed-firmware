PROJECT_ROOT	= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)
PROJECT			= firmware

include 		$(PROJECT_ROOT)/make/base.mk
include			$(PROJECT_ROOT)/assets/assets.mk
include			$(PROJECT_ROOT)/core/core.mk
include 		$(PROJECT_ROOT)/applications/applications.mk
include			$(PROJECT_ROOT)/lib/lib.mk

TARGET			?= f2
TARGET_DIR		= targets/$(TARGET)
include			$(TARGET_DIR)/target.mk

include			$(PROJECT_ROOT)/make/toolchain.mk
include			$(PROJECT_ROOT)/make/rules.mk
