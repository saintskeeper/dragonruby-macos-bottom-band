SDK ?= .sdk
ARCH ?= $(shell uname -m)

GAME_DIR := $(abspath .)
LIB := native/macos/bottom_band.dylib
SDK_HEADER := $(SDK)/include/dragonruby.h
SDK_BINARY := $(SDK)/dragonruby

.PHONY: build run clean check-sdk FORCE

build: $(LIB)

run: build
	@"$(SDK_BINARY)" "$(GAME_DIR)"

# Rebuild this tiny extension so SDK/ARCH changes cannot reuse a stale binary.
FORCE:

$(LIB): bottom_band.c FORCE | native/macos check-sdk
	clang -x c -std=c11 -fPIC -dynamiclib -arch "$(ARCH)" \
		-isysroot "$$(xcrun --show-sdk-path)" -I"$(SDK)/include" \
		"$<" -framework AppKit -framework Foundation -framework CoreGraphics \
		-o "$@"

native/macos:
	mkdir -p "$@"

check-sdk:
	@if [ "$$(uname -s)" != "Darwin" ]; then \
		echo "error: this example builds only on macOS (Darwin)." >&2; \
		exit 1; \
	fi
	@if [ ! -f "$(SDK_HEADER)" ]; then \
		echo "error: DragonRuby header not found: $(SDK_HEADER)" >&2; \
		echo "Set SDK=/absolute/path/to/dragonruby-macos-7.16 (requires Pro C extension support)." >&2; \
		exit 1; \
	fi
	@if [ ! -x "$(SDK_BINARY)" ]; then \
		echo "error: DragonRuby executable not found or not executable: $(SDK_BINARY)" >&2; \
		echo "Set SDK=/absolute/path/to/dragonruby-macos-7.16." >&2; \
		exit 1; \
	fi

clean:
	rm -f "$(LIB)"
