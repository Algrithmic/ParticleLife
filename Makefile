# Program Variables
PROGNAME := ParticleLife
SOURCE_DIR := ./source
VENDOR_DIR := ./vendor
BUILD_DIR := ./build

# Compilation Variables
CC := gcc
CFLAGS := -Wall -Wextra -Ivendor/SDL3/include -Ivendor/GLAD/include
LDFLAGS := -lm -lcglm -lSDL3 -Lvendor/SDL3/libraries
RPATH := -Wl,-rpath,'$$ORIGIN/../vendor/SDL3/libraries'

# Create all corresponding .o files from .c filenames
SOURCES := $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

# Add GLAD source and object
GLAD_SRC := $(VENDOR_DIR)/GLAD/src/glad.c
GLAD_OBJ := $(BUILD_DIR)/glad.o

# Cleanup variables
RM := rm -rf

.PHONY: all clean rebuild

all: $(BUILD_DIR)/$(PROGNAME)

# link all the object files in the final program exe
$(BUILD_DIR)/$(PROGNAME): $(OBJECTS) $(GLAD_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) $(RPATH)

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
