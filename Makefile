# ST-1 / Ezra Core
# STN-LABZ-owned Core implementation. Mission-specific modules are separate.
#
# Default: static Core library. No executable entry point is present yet.
# Linux / POSIX C; OpenSSL headers are required by core_hotload.c.
#
# Build: make
# Clean: make clean

CC ?= cc
AR ?= ar
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
ARFLAGS = rcs

CPPFLAGS += -Iinclude -Iinclude/stnlabz_abi

BUILD_DIR ?= build
TARGET ?= $(BUILD_DIR)/libstnlabz_core.a

CORE_SRCS := $(sort $(wildcard src/*.c) $(wildcard src/stnlabz_abi/*.c))
CORE_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(CORE_SRCS))
CORE_DEPS := $(CORE_OBJS:.o=.d)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(CORE_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(CORE_DEPS)
