# ST-1 / Ezra executable (Linux)
# The shared STN-LABZ ABI is maintained outside this repository.
#
# Default layout:
#   ../stnlabz_abi/include/stnlabz_abi/*.h
#   ../stnlabz_abi/src/*.c
# Override ABI_DIR / ABI_INCLUDE_DIR / ABI_SRC_DIR for a different layout.

CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -ldl -lcrypto

ABI_DIR ?= ../stnlabz_abi
ABI_INCLUDE_DIR ?= $(ABI_DIR)/include
ABI_SRC_DIR ?= $(ABI_DIR)/src

CPPFLAGS += -Iinclude -I$(ABI_INCLUDE_DIR) -I$(ABI_INCLUDE_DIR)/stnlabz_abi

BUILD_DIR ?= build
TARGET ?= bin/ezra

CORE_SRCS := $(sort $(wildcard src/*.c))
ABI_SRCS := $(sort $(wildcard $(ABI_SRC_DIR)/*.c) $(wildcard $(ABI_SRC_DIR)/stnlabz_abi/*.c))
CORE_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(CORE_SRCS))
ABI_OBJS := $(patsubst $(ABI_SRC_DIR)/%.c,$(BUILD_DIR)/abi/%.o,$(ABI_SRCS))
OBJS := $(CORE_OBJS) $(ABI_OBJS)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(TARGET)

ifeq ($(strip $(ABI_SRCS)),)
$(error No external ABI C sources found in $(ABI_SRC_DIR); set ABI_DIR or ABI_SRC_DIR)
endif

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/abi/%.o: $(ABI_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
