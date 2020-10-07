#######################################
# Rust library
#######################################

#######################################
# Rust library
#######################################

RUST_LIB_SRC = $(realpath $(PROJECT_DIR)/../core-rs)
RUST_LIB_NAME = flipper_core

ifeq ($(ARCH), 'x86_64')
RUST_LIB_TARGET = x86_64-unknown-linux-gnu
else
RUST_LIB_TARGET = thumbv7em-none-eabihf
endif

RUST_LIB_FLAGS = --target=$(RUST_LIB_TARGET)

ifeq ($(DEBUG), 1)
    RUST_LIB_PATH = $(RUST_LIB_SRC)/target/$(RUST_LIB_TARGET)/debug
else
    RUST_LIB_FLAGS += --release
    RUST_LIB_PATH = $(RUST_LIB_SRC)/target/$(RUST_LIB_TARGET)/release
endif

RUST_LIB_CMD = cd $(RUST_LIB_SRC) && cargo build -p flipper-core $(RUST_LIB_FLAGS)

LD_FLAGS += -l$(RUST_LIB_NAME)
LD_FLAGS += -L$(RUST_LIB_PATH)

$(RUST_LIB_PATH)/lib$(RUST_LIB_NAME).a: rust_lib

rust_lib:
	$(RUST_LIB_CMD)

clean:
	-rm -fR $(BUILD_DIR)
	cd $(RUST_LIB_SRC) && cargo clean



