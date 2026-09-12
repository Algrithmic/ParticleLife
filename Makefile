# Program Variables
PROGNAME := ParticleLife
SOURCE_DIR := ./source
VENDOR_DIR := ./vendor
BUILD_DIR := ./build

# Compilation Variables
CC := gcc
CFLAGS := -Wall -Wextra -Ivendor/SDL3/include -Ivendor/GLAD/include -Ivendor/Nuklear/include
LDFLAGS := -lm -lcglm -lSDL3 -Lvendor/SDL3/libraries

# Platform detection: MSYS2/MinGW sets OS=Windows_NT, matching native Windows builds
ifeq ($(OS),Windows_NT)
	EXE_EXT := .exe
	# SDL3.dll must sit next to the exe (or be on PATH) since Windows has no rpath
	POST_BUILD := cp -u $(VENDOR_DIR)/SDL3/libraries/SDL3.dll $(BUILD_DIR)/

	# Always build with the MinGW-w64 compiler directly instead of trusting `gcc` on PATH.
	ifneq ($(wildcard /mingw64/bin/gcc.exe),)
		CC := /mingw64/bin/gcc.exe
	endif

	CC_MACHINE := $(shell $(CC) -dumpmachine 2>/dev/null)
	ifeq (,$(findstring mingw32,$(CC_MACHINE)))
	$(error `gcc` resolved to target '$(CC_MACHINE)', not MinGW-w64. Install \
	the toolchain from the MSYS2 MINGW64 shell: pacman -S mingw-w64-x86_64-gcc. \
	See "Building on Windows" in README.md)
	endif
else
	RPATH := -Wl,-rpath,'$$ORIGIN/../vendor/SDL3/libraries'
	POST_BUILD := true
endif

TARGET := $(BUILD_DIR)/$(PROGNAME)$(EXE_EXT)

# Create all corresponding .o files from .c filenames
SOURCES := $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

# Add GLAD source and object
GLAD_SRC := $(VENDOR_DIR)/GLAD/src/glad.c
GLAD_OBJ := $(BUILD_DIR)/glad.o

# Documentation variables
DOXYGEN := doxygen
DOCS_DIR := ./docs

# Cleanup variables
RM := rm -rf

.PHONY: all clean rebuild docs clean-docs

all: $(TARGET)

# link all the object files in the final program exe
$(TARGET): $(OBJECTS) $(GLAD_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) $(RPATH)
	@$(POST_BUILD)

# Compile each .c file into .o file
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile GLAD
$(GLAD_OBJ): $(GLAD_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory if it does not exist
$(BUILD_DIR):
	mkdir -p $@

# clean up .o files and exe
clean:
	$(RM) $(BUILD_DIR)

# Clean up .o files and exe, then build
rebuild: clean
	$(MAKE) all

# Generate HTML documentation from Doxygen comments -> docs/html/index.html
docs:
	$(DOXYGEN) Doxyfile
	mkdir -p $(DOCS_DIR)/html/assets
	cp -r assets/. $(DOCS_DIR)/html/assets/

# Remove generated documentation
clean-docs:
	$(RM) $(DOCS_DIR)
