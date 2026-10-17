# ST-1 / Ezra: Linux executable
# Core is STN-LABZ-owned; spacecraft capabilities remain separate modules.
#
# Build: make
# Clean: make clean
# The current Core sources still require include/core_activation.h.

CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -ldl -lcrypto

CPPFLAGS += -Iinclude -Iinclude/stnlabz_abi

BUILD_DIR ?= build
TARGET ?= bin/ezra

CORE_SRCS := $(sort $(filter-out src/main.c,$(wildcard src/*.c)) $(wildcard src/stnlabz_abi/*.c))
SRCS := src/main.c $(CORE_SRCS)
OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
