CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
ROOT ?= ../..
PREFIX ?= /opt/ezra
MODULE_USER ?= $(shell id -un)
MODULE_GROUP ?= $(shell id -gn)
MODULEDIR ?= $(PREFIX)/modules/sentinel
CPPFLAGS += -I$(ROOT)/include -I$(ROOT)/../ABI/includes
.PHONY: all clean install
all: build/sentinel.so
build/sentinel.so: src/sentinel_module.c
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -shared -o $@ $< -pthread
install: all
	install -d -o "$(MODULE_USER)" -g "$(MODULE_GROUP)" -m 0755 "$(MODULEDIR)"
	install -o "$(MODULE_USER)" -g "$(MODULE_GROUP)" -m 0644 module.conf "$(MODULEDIR)/module.conf"
	@set -eu; tmp=$$(mktemp "$(MODULEDIR)/.sentinel.so.XXXXXX"); \
	  trap 'rm -f "$$tmp"' EXIT HUP INT TERM; \
	  install -o "$(MODULE_USER)" -g "$(MODULE_GROUP)" -m 0755 build/sentinel.so "$$tmp"; \
	  cmp -s build/sentinel.so "$$tmp"; \
	  mv -f "$$tmp" "$(MODULEDIR)/sentinel.so"; \
	  trap - EXIT HUP INT TERM; \
	  cmp -s build/sentinel.so "$(MODULEDIR)/sentinel.so"
clean:
	rm -rf build
