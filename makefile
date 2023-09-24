.PHONY: pixelbox clean

include makecfg

SRCS := $(shell find ./src/ -name '*.c')
OBJS := $(SRCS:./src/%.c=./bin/%.o)
FLAGS +=  

pixelbox : $(OBJS)
	$(CC) $^ -o $@ $(LFLAGS) -lm -lpthread -lraylib -rdynamic $(FLAGS)

./bin/%.o : ./src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

clean :
	rm ./bin/*.o
