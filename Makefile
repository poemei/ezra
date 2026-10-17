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

ABI_DIR ?= ../../ABI
ABI_INCLUDE_DIR ?= $(ABI_DIR)/includes
ABI_SRC_DIR ?= $(ABI_DIR)/src

CPPFLAGS += -Iinclude -I$(ABI_INCLUDE_DIR) -Iinclude/stnlabz_abi

BUILD_DIR ?= build
TARGET ?= bin/ezra

CORE_SRCS := $(sort $(wildcard src/*.c))
ABI_SRCS := $(sort $(filter-out $(ABI_SRC_DIR)/main.c $(ABI_SRC_DIR)/loader_win.c $(ABI_SRC_DIR)/module_discovery_win.c,$(wildcard $(ABI_SRC_DIR)/*.c)))
CORE_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(CORE_SRCS))
ABI_OBJS := $(patsubst $(ABI_SRC_DIR)/%.c,$(BUILD_DIR)/abi/%.o,$(ABI_SRCS))
OBJS := $(CORE_OBJS) $(ABI_OBJS)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(TARGET)

.PHONY: check-abi
check-abi:
	@test -n "$(strip $(ABI_SRCS))" || { \
		echo "ERROR: Shared ABI C sources not found in $(ABI_SRC_DIR)" >&2; \
		echo "Set ABI_DIR or ABI_SRC_DIR to the actual external ABI location." >&2; \
		exit 1; \
	}

$(TARGET): $(OBJS) | check-abi
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
