CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra
INCLUDES = -Isrc
LIBS = $(shell pkg-config --libs raylib 2>/dev/null || echo "-lraylib -lm -lpthread -ldl")

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = metro_grid

# Auto-detect Emscripten compiler and Raylib Web library
EMCC ?= $(shell which em++ 2>/dev/null || which emcc 2>/dev/null || echo "/home/sachin/emsdk/upstream/emscripten/em++")
RAYLIB_WEB_LIB ?= $(shell find /home/sachin/raylib-src -name "libraylib.web.a" 2>/dev/null | head -n 1)
ifeq ($(RAYLIB_WEB_LIB),)
    RAYLIB_WEB_LIB = /home/sachin/raylib-src/src/libraylib.web.a
endif
RAYLIB_WEB_INC ?= -I/home/sachin/raylib-src/src

ZIP_NAME = metro_grid_web.zip

all: $(TARGET) coaster_grid

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

coaster_grid: $(TARGET)
	ln -sf $(TARGET) coaster_grid

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

web:
	@echo "==> [1/3] Compiling Metro Grid to WebAssembly (Emscripten)..."
	@mkdir -p build_web
	$(EMCC) -O3 -std=c++17 -DPLATFORM_WEB $(INCLUDES) $(RAYLIB_WEB_INC) $(SRCS) $(RAYLIB_WEB_LIB) \
		-sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 --shell-file src/itch_shell.html -o build_web/index.html
	@echo "==> [2/3] Packing directly into itch.io uploadable ZIP ($(ZIP_NAME))..."
	@rm -f $(ZIP_NAME) ../$(ZIP_NAME) ../build/$(ZIP_NAME)
	zip -9 -j $(ZIP_NAME) build_web/index.html build_web/index.js build_web/index.wasm
	@mkdir -p ../build
	@cp $(ZIP_NAME) ../build/$(ZIP_NAME)
	@cp $(ZIP_NAME) ../$(ZIP_NAME)
	@echo "==> [3/3] Verification & Integrity Check..."
	@unzip -t $(ZIP_NAME)
	@echo ""
	@echo "========================================================================="
	@echo " SUCCESS: itch.io Web Build ZIP is Ready for Direct Upload!"
	@echo " File location (upload this directly in itch.io 'Uploads' section):"
	@echo "   1) $(abspath ../$(ZIP_NAME))"
	@echo "   2) $(abspath $(ZIP_NAME))"
	@echo "   3) $(abspath ../build/$(ZIP_NAME))"
	@echo ""
	@echo " itch.io Settings:"
	@echo "   * Kind of project: HTML"
	@echo "   * [x] This file will be played in the browser (CHECK THIS BOX)"
	@echo "   * Viewport dimensions: 1280 x 720"
	@echo "   * [x] Automatically start on page load"
	@echo "   * [x] Fullscreen button"
	@echo "========================================================================="

clean:
	rm -f $(OBJS) $(TARGET) coaster_grid
	rm -rf build_web $(ZIP_NAME) ../$(ZIP_NAME) ../build/$(ZIP_NAME)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run web
