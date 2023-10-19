# see config.mk

# append build path (a bit hacky)
BIN_DIR := $(addprefix $(BUILD_DIR),/linux)

OBJS := $(SRCS:./src/%=$(BIN_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
FLAGS += -MMD -MP  

CC := gcc
CXX := g++

$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@ $(LFLAGS) -lm -lpthread -lraylib -rdynamic $(FLAGS)

$(BIN_DIR)/%.c.o: ./src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

$(BIN_DIR)/%.cpp.o : ./src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

-include $(DEPS)
