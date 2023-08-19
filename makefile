.PHONY: kisspixel clean

SRCS := $(shell find ./src/ -name '*.c')
OBJS := $(SRCS:./src/%.c=./bin/%.o)
FLAGS := -Og -fsanitize=address -fsanitize=undefined -g

kisspixel : $(OBJS)
	$(CC) $^ -o $@ -lm -lpthread -lraylib $(FLAGS)

./bin/%.o : ./src/%.c
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS)

clean :
	rm ./bin/*.o
