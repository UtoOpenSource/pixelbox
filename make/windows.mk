# see config.mk

# append build path (a bit hacky)
BIN_DIR := $(addprefix $(BUILD_DIR),/mingw)

OBJS := $(SRCS:./src/%=$(BIN_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
FLAGS += -MMD -MP  

# mingw toolchain
WCC  := x86_64-w64-mingw32-gcc 
WCXX := x86_64-w64-mingw32-g++

$(TARGET): pixelbox.exe

# windows specific
WLFLAGS := -static -static-libgcc -lm -lgdi32 -lwinmm -lpthread

libraylib.a :
	echo "you must build static raylib for windows and move libraylib.a in the root of this repository manually!"
	echo "recomended command is 'make CC=x86_64-w64-mingw32-gcc USE_EXTERNAL_GLFW=FALSE'"
	false;

pixelbox.exe : $(OBJS) libraylib.a
	$(WCCX) $^ libraylib.a -o $@  $(LFLAGS) $(FLAGS) $(WLFLAGS)

$(BIN_DIR)/%.c.o : ./src/%.c
	mkdir -p $(dir $@)
	$(WCC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

$(BIN_DIR)/%.cpp.o : ./src/%.cpp
	mkdir -p $(dir $@)
	$(WCXX) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

-include $(DEPS)

# build raylib with this command (make shure it's static) : 
# make CC=x86_64-w64-mingw32-gcc USE_EXTERNAL_GLFW=FALSE
