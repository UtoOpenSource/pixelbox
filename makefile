.PHONY: pixelbox clean

include makecfg

SRCS += 
OBJS := $(SRCS:./src/%.c=./bin/%.o)
DEPS := $(OBJS:.o=.d)
FLAGS += -MMD -MP

pixelbox : $(OBJS)
	$(CC) $^ -o $@ $(LFLAGS) -lm -lpthread -lraylib -rdynamic $(FLAGS)

./bin/%.o : ./src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -Wall -Wextra $(FLAGS) $(INCS)

# archiver building...
./tools/archiver : ./tools/archiver.c
	$(CC) $^ -I./tools/ -o ./tools/archiver -lm -lraylib -Wall -O2

clean :
	rm ./bin/*.o

-include $(DEPS)


