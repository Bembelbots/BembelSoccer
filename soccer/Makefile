DOCKER_COMPOSE ?= docker-compose
DOCKER ?= docker
USER_ID ?= $(shell id -u)
SRC_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR ?= build/v6

CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Debug
EXEC_FLAGS ?= -u$(USER_ID)

# disable pseudo-tty in jenkins builds
EXEC_FLAGS += $(shell tty -s || echo -T)
NINJA := $(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild ninja -C /src/soccer/$(BUILD_DIR)

all: compile

start: .build.env
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
	$(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild cmake -Wno-dev -GNinja $(CMAKE_FLAGS) -B /src/soccer/$(BUILD_DIR) /src/soccer/

compile_commands.json: $(BUILD_DIR)/compile_commands.json
	cp $(BUILD_DIR)/compile_commands.json .
	sed -i -e "s#/src/soccer/#$(SRC_DIR)#g" compile_commands.json

flatbuffers: | start $(BUILD_DIR)/build.ninja
	$(NINJA) jrlmonitor_settings bembelbuf_schemas

debug: | start $(BUILD_DIR)/build.ninja
	$(NINJA) playingfield_debug whistledebug behavior_graph

compile: | start $(BUILD_DIR)/build.ninja compile_commands.json
	$(NINJA)

$(BUILD_DIR)/compile_commands.json: $(BUILD_DIR)/build.ninja

cmake: $(BUILD_DIR)/build.ninja

devcontainer: .build.env

clean:
	rm -rf .build.env $(BUILD_DIR)

shell: start
	$(DOCKER_COMPOSE) exec $(EXEC_FLAGS) bembelbuild /bin/bash -l || true

.PHONY: all shutdown pull clean shell

.NOTPARALLEL: start
