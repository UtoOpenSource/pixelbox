.PHONY: pixelbox clean

include makecfg

SRCS += 
OBJS := $(SRCS:./src/%=./bin/%.o)
DEPS := $(OBJS:.o=.d)
FLAGS += -MMD -MP  

pixelbox : $(OBJS)
	$(CXX) $^ -o $@ $(LFLAGS) -lm -lpthread -lraylib -rdynamic $(FLAGS)

./bin/%.c.o : ./src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

./bin/%.cpp.o : ./src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

clean :
	rm ./bin/*.o

-include $(DEPS)


