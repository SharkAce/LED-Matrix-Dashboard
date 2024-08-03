NAME=matrix-dashboard

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/,%,$(dir $(MKFILE_PATH)))

EXEC_REL=$(NAME)
EXEC_ABS=$(CURRENT_DIR)/$(NAME)
EXEC_TARGET=/usr/bin/$(NAME)

SERVICE_DIR=systemd
SERVICE_NAME=$(NAME).service
SERVICE_FILE=$(SERVICE_DIR)/$(SERVICE_NAME)
SERVICE_FILE_TARGET=/etc/systemd/system/$(SERVICE_NAME)

RGB_LIB_DISTRIBUTION=matrix
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
FLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread -O3 -lcurl -std=c++17

.PHONY: all install uninstall test

all: $(EXEC_REL)

$(EXEC_REL): main.cpp $(RGB_LIBRARY)
	 $(CXX) $(CXXFLAGS) main.cpp -o $@ $(FLAGS)

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

install: $(EXEC_REL)
	@if ! [ "$(shell id -u)" = 0 ]; then\
		echo "This command requires root privileges.";\
		exit 1;\
	fi
	ln -sf $(EXEC_ABS) $(EXEC_TARGET)
	cp -f $(SERVICE_FILE) $(SERVICE_FILE_TARGET)
	systemctl enable systemd-networkd-wait-online.service
	systemctl enable $(SERVICE_NAME)
	@echo
	@echo "Service installed. You can now start it with:"
	@echo "systemctl start $(SERVICE_NAME)"
	@echo "You can also run it in the current shell with:"
	@echo "$(NAME)"

uninstall:
	@id -u > /dev/null 2>&1 || (echo "This command requires root privileges."; exit 1)
	systemctl stop $(SERVICE_NAME)
	systemctl disable $(SERVICE_NAME)
	rm -f $(EXEC_TARGET)
	rm -f $(SERVICE_TARGET)
	@echo Uninstall complete.
