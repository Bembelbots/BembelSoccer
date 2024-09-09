-include config.mk

DOCKER ?= docker
DOCKER_COMPOSE ?= $(DOCKER)-compose
USER_ID ?= $(shell id -u)
SRC_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR ?= build/v6

CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Debug
EXEC_FLAGS ?= -u$(USER_ID)

# disable pseudo-tty in jenkins builds
EXEC_FLAGS += $(shell tty -s || echo -T)
CMAKE_BUILD := $(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild cmake --build --preset 

all: framework

${HOME}/.docker/config.json:
	@echo "Please login using your Bembelbots login:"
	$(DOCKER) login git.bembelbots.de

start: .build.env ${HOME}/.docker/config.json
	$(DOCKER_COMPOSE) up -d

shutdown: .build.env
	$(DOCKER_COMPOSE) down

update: .build.env
	$(DOCKER_COMPOSE) pull

.build.env: Makefile ./.devcontainer/setup
	./.devcontainer/setup

$(BUILD_DIR)/build.ninja:
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	$(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild cmake --preset v6

compile_commands.json: $(BUILD_DIR)/compile_commands.json
	cp $(BUILD_DIR)/compile_commands.json .
	sed -i -e "s#/src/soccer/#$(SRC_DIR)#g" compile_commands.json

flatbuffers: | start $(BUILD_DIR)/build.ninja
	$(CMAKE_BUILD) $@

debugtools: | start $(BUILD_DIR)/build.ninja
	$(CMAKE_BUILD) $@

framework: | start $(BUILD_DIR)/build.ninja compile_commands.json
	$(CMAKE_BUILD) $@

$(BUILD_DIR)/compile_commands.json: $(BUILD_DIR)/build.ninja

cmake: $(BUILD_DIR)/build.ninja

devcontainer: .build.env

clean:
	rm -rf .build.env $(BUILD_DIR)

shell: start
	$(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild /bin/bash -l || true

sim: start
	$(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild naosim || true

.PHONY: all shutdown pull clean shell

.NOTPARALLEL: start
