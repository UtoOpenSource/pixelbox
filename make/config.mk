# DO NOT EDIT!
# DO NOT RUN!
# used by makefile in the root of the repo!

# preinit

# Disable built-in rules and variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

# all directories with sourcefiles to compile in pixelbox binary
DIRS ?= $(shell find ./src/ -type d -not -name '*old*')

# default c compiler flags for source files
FLAGS ?= -Og

# c compiler flags for linking all object files in target ./pixelbox[.exe]
LFLAGS ?= -g

# all files to compile
SRCS   ?= $(shell find $(DIRS) -name '*.c' -or -name '*.cpp') 

# include flags
INCS   += $(addprefix -I,$(DIRS))

# core target
TARGET := pixelbox
all: pixelbox

# build directory
BUILD_DIR :=./build

TARGET_WINDOWS := FALSE

# now you can build both :D
ifneq ($(TARGET_LINUX), FALSE)
include ./make/linux.mk
endif

ifneq ($(TARGET_WINDOWS), FALSE)
include ./make/windows.mk
endif
