TOOLCHAIN = x86

# Sources
C_SOURCES += $(TARGET_DIR)/Src/main.c
C_SOURCES += $(TARGET_DIR)/Src/flipper_hal.c
C_SOURCES += $(TARGET_DIR)/Src/lo_os.c
C_SOURCES += $(TARGET_DIR)/Src/lo_hal.c

# CFLAGS += -DFURI_DEBUG
CFLAGS += -I$(TARGET_DIR)/Inc
CFLAGS += -Wall -fdata-sections -ffunction-sections -pthread
LDFLAGS += -pthread

# FatFs library
CFLAGS += -I$(TARGET_DIR)/fatfs
C_SOURCES += $(TARGET_DIR)/fatfs/syscall.c

# memory manager
C_SOURCES += $(TARGET_DIR)/Src/heap_4.c

CFLAGS += -I$(TARGET_DIR)/api-hal
C_SOURCES += $(wildcard $(TARGET_DIR)/api-hal/*.c)

run: all
	$(OBJ_DIR)/$(PROJECT).elf