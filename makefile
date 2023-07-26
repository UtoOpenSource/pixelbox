.PHONY: kisspixel clean

SRCS := $(shell find ./src/ -name '*.c')
OBJS := $(SRCS:./src/%.c=./bin/%.o)

kisspixel : $(OBJS)
	$(CC) $^ -o $@ -lm -lpthread

./bin/%.o : ./src/%.c
	$(CC) -c $< -o $@ -O1 -g -Wall -Wextra

clean :
	rm ./bin/*.o
