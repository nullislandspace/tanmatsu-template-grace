SHELL := /usr/bin/env bash

# App installation settings
APP_SLUG_NAME ?= tld.username.gracetemplate
APP_INSTALL_BASE_PATH ?= /int/apps/
APP_INSTALL_PATH = $(APP_INSTALL_BASE_PATH)$(APP_SLUG_NAME)

# ESP-IDF tools path (needed for the cross-compiler)
IDF_TOOLS_PATH ?= $(shell cat .IDF_TOOLS_PATH 2>/dev/null)

export IDF_TOOLS_PATH

BUILD ?= build

MAKEFLAGS += --silent

####

.PHONY: all
all: build

.PHONY: build
build:
	@echo "=== Building app.so ==="
	mkdir -p $(BUILD)
	cd $(BUILD) && cmake .. && make
	@echo "=== Build complete: $(BUILD)/app.so ==="

# Badgelink
.PHONY: badgelink
badgelink:
	rm -rf badgelink
	git clone https://github.com/badgeteam/esp32-component-badgelink.git badgelink
	cd badgelink/tools; ./install.sh

.PHONY: install
install: build
	@echo "=== Installing to device ==="
	@echo "Creating directory $(APP_INSTALL_PATH)..."
	cd badgelink/tools; ./badgelink.sh fs mkdir $(APP_INSTALL_PATH) || true
	@echo "Uploading metadata.json..."
	cd badgelink/tools; ./badgelink.sh fs upload $(APP_INSTALL_PATH)/metadata.json ../../metadata/metadata.json
	@echo "Uploading icon16.png..."
	cd badgelink/tools; ./badgelink.sh fs upload $(APP_INSTALL_PATH)/icon16.png ../../metadata/icon16.png
	@echo "Uploading icon32.png..."
	cd badgelink/tools; ./badgelink.sh fs upload $(APP_INSTALL_PATH)/icon32.png ../../metadata/icon32.png
	@echo "Uploading icon64.png..."
	cd badgelink/tools; ./badgelink.sh fs upload $(APP_INSTALL_PATH)/icon64.png ../../metadata/icon64.png
	@echo "Uploading app.so..."
	cd badgelink/tools; ./badgelink.sh fs upload $(APP_INSTALL_PATH)/app.so ../../$(BUILD)/app.so
	@echo "=== Installation complete ==="

# Cleaning

.PHONY: clean
clean:
	rm -rf $(BUILD)

.PHONY: fullclean
fullclean: clean

# Formatting

.PHONY: format
format:
	find main/ -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' | xargs clang-format -i
