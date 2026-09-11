
.PHONY: all linux-x64 linux-arm64 macos-x64 macos-arm64

# Linux x86_64
linux-x64_configure:
	cd native && cmake --preset linux-x64-release

linux-x64_build:
	cd native && cmake --build --preset linux-x64-release

linux-x64: linux-x64_configure linux-x64_build

# Linux ARM64
linux-arm64_configure:
	cd native && cmake --preset linux-arm64-release

linux-arm64_build:
	cd native && cmake --build --preset linux-arm64-release

linux-arm64: linux-arm64_configure linux-arm64_build

# Linux ARM64
linux-cross-arm64_configure:
	cd native && cmake --preset linux-cross-arm64-release

linux-cross-arm64_build:
	cd native && cmake --build --preset linux-cross-arm64-release

linux-cross-arm64: linux-arm64_configure linux-cross-arm64_build

# Win x64
win-x64_configure:
	cd native && cmake --preset win-x64-release

win-x64_build:
	cd native && cmake --build --preset win-x64-release

win-x64: win-x64_configure win-x64_build

# Win arm64
win-arm64_configure:
	cd native && cmake --preset win-arm64-release

win-arm64_build:
	cd native && cmake --build --preset win-arm64-release

win-arm64: win-arm64_configure win-arm64_build

# macOS x86_64
macos-x64_configure:
	cmake -S native --preset macos-x64-release

macos-x64_build:
	cmake --build --preset macos-x64-release

macos-x64: macos-x64_configure macos-x64_build

# macOS ARM64
macos-arm64_configure:
	cmake -S native --preset macos-arm64-release

macos-arm64_build:
	cmake --build --preset macos-arm64-release

macos-arm64: macos-arm64_configure macos-arm64_build


# Docker
docker-linux-x64:
	cd docker/linux-x64 && docker compose run --rm linux-x64 \
		sh -c 'cd native && cmake --preset linux-x64-release && cmake --build --preset linux-x64-release'

docker-linux-arm64:
	cd docker/linux-arm64 && docker compose run --rm linux-arm64 \
		sh -c 'cd native && cmake --preset linux-arm64-release && cmake --build --preset linux-arm64-release'

docker-win-x64:
	cd docker/win-x64 && docker compose run --rm win-x64-builder \
		sh -c 'cd native && cmake --preset win-x64-release && cmake --build --preset win-x64-release'

# Win arm64
docker-win-arm64:
	cd docker/win-arm64 && docker compose run --rm win-arm64-builder \
		sh -c 'cd native && cmake --preset win-arm64-release && cmake --build --preset win-arm64-release'

archive:
	tar -czf dist/pulse-player-ffmpeg-test.tar.gz packages/pulse-player-ffmpeg packages/linux-x64 packages/linux-arm64 packages/win32-x64 packages/win32-arm64

# Build all
all: linux-x64
