CORE_DIR		= $(PROJECT_ROOT)/core

CFLAGS			+= -I$(CORE_DIR)
ASM_SOURCES		+= $(wildcard $(CORE_DIR)/*.s)
C_SOURCES		+= $(wildcard $(CORE_DIR)/*.c)
CPP_SOURCES		+= $(wildcard $(CORE_DIR)/*.cpp)
