NAME=matrix-dashboard

BUILD_DIR := build
SRC_DIR := src
INC_DIR := include
TP_INC_DIR := include/thirdparty
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/,%,$(dir $(MKFILE_PATH)))

EXEC_REL := $(BUILD_DIR)/$(NAME)
EXEC_ABS := $(CURRENT_DIR)/$(BUILD_DIR)/$(NAME)
EXEC_TARGET := /usr/bin/$(NAME)

SERVICE_DIR := systemd
SERVICE_NAME := $(NAME).service
SERVICE_FILE := $(SERVICE_DIR)/$(SERVICE_NAME)
SERVICE_FILE_TARGET := /etc/systemd/system/$(SERVICE_NAME)

CONFIG_DIR := config
CONFIG_FILE := $(CONFIG_DIR)/config.json
CONFIG_FILE_TARGET := /etc/$(NAME)/config.json
CONFIG_FILE_BACKUP := /etc/$(NAME)/config.json.bak

RGB_LIB_DISTRIBUTION := rpi-rgb-led-matrix
RGB_INCDIR := $(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR := $(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME := rgbmatrix
RGB_LIBRARY := $(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a

INCLUDES := $(RGB_INCDIR) $(INC_DIR) $(TP_INC_DIR) 
CXXFLAGS += -O3 -Wno-psabi -std=c++17
CXXFLAGS += $(addprefix -I,$(INCLUDES))
LDFLAGS += -L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread -lcurl

PCH_HEADER := $(INC_DIR)/pch.hpp
PCH_FILE := $(INC_DIR)/pch.hpp.gch

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.d,$(SOURCES))

.PHONY: all
all: $(EXEC_REL)

$(EXEC_REL): $(RGB_LIBRARY) $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(PCH_FILE): $(PCH_HEADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(PCH_FILE) $(RGB_LIBRARY)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

.PHONY: check-root install uninstall clean
check-root:
	@[ "$(shell id -u)" -eq 0 ] || { echo "This rule must be run as root"; exit 1; }

install: check-root $(EXEC_REL)
	ln -sf $(EXEC_ABS) $(EXEC_TARGET)
	cp -f $(SERVICE_FILE) $(SERVICE_FILE_TARGET)
	systemctl enable systemd-networkd-wait-online.service
	systemctl enable $(SERVICE_NAME)
	mkdir -p $(dir $(CONFIG_FILE_TARGET))
	[ -e "$(CONFIG_FILE_TARGET)" ] && mv $(CONFIG_FILE_TARGET) $(CONFIG_FILE_BACKUP)
	cp -f $(CONFIG_FILE) $(CONFIG_FILE_TARGET)
	@echo
	@echo "Service installed. You can now start it with:"
	@echo "systemctl start $(SERVICE_NAME)"
	@echo "You can also run it in the current shell with:"
	@echo "$(NAME)"

uninstall: check-root
	systemctl stop $(SERVICE_NAME)
	systemctl disable $(SERVICE_NAME)
	rm -f $(EXEC_TARGET)
	rm -f $(SERVICE_FILE_TARGET)
	@echo Uninstall complete.

clean:
	rm -f $(OBJECTS) $(DEPENDS) $(EXEC_REL) $(PCH_FILE)
