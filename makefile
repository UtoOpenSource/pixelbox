.PHONY: pixelbox clean

include makecfg

SRCS += 
OBJS := $(SRCS:./src/%=./bin/%.o)
DEPS := $(OBJS:.o=.d)
FLAGS += -MMD -MP -fsanitize=address -fsanitize=undefined 

pixelbox : $(OBJS)
	$(CXX) $^ -o $@ $(LFLAGS) -lm -lpthread -lraylib -rdynamic $(FLAGS)

./bin/%.c.o : ./src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

./bin/%.cpp.o : ./src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

# archiver building...
./tools/archiver : ./tools/archiver.c
	$(CC) $^ -I./tools/ -o ./tools/archiver -lm -lraylib -Wall $(FLAGS)

clean :
	rm ./bin/*.o

-include $(DEPS)


