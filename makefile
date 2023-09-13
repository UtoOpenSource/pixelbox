.PHONY: kisspixel clean

include makecfg

SRCS := $(shell find ./src/ -name '*.c')
OBJS := $(SRCS:./src/%.c=./bin/%.o)
FLAGS +=  

kisspixel : $(OBJS)
	$(CC) $^ -o $@ -lm -lpthread -lraylib $(FLAGS)

./bin/%.o : ./src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

clean :
	rm ./bin/*.o
