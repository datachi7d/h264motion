
BUILD_DIR:=./build
DEBUG?=1

ifeq ($(DEBUG),0)
  override CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE=Release
else
  override CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE=Debug
endif


all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) --no-print-directory
	@echo "Build complete in directory $(BUILD_DIR)"

install: all
	$(MAKE) -C $(BUILD_DIR) --no-print-directory install

$(BUILD_DIR)/Makefile:
	mkdir -p $(BUILD_DIR)
	( cd $(BUILD_DIR) && cmake .. $(CMAKE_OPTIONS) )
