.PHONY: kisspixel clean

SRCS := $(shell find ./src/ -name '*.c')
OBJS := $(SRCS:./src/%.c=./bin/%.o)

kisspixel : $(OBJS)
	$(CC) $^ -o $@ -lm -lpthread -lsqlite3 -lraylib -fsanitize=undefined

./bin/%.o : ./src/%.c
	$(CC) -c $< -o $@ -O1 -g -Wall -Wextra -fsanitize=undefined

clean :
	rm ./bin/*.o
