.PHONY: all release debug clean
DEBUG_DIR = build
RELEASE_DIR = build-release
clean:
	rm -rf $(DEBUG_DIR) $(RELEASE_DIR)
release:
	cmake -S . -B $(RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(RELEASE_DIR) --config Release
debug:
	cmake -S . -B $(DEBUG_DIR) -DCMAKE_TOOLCHAIN_FILE="" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(DEBUG_DIR) --config Debug
all: release

