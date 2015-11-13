CFLAGS=-O2 -Wall -march=native
REMOTE_OBJS=Remote.o runner_unix.o
EXECS=test_all_pixel_unix game_of_life_unix

all: $(EXECS)

%_unix: $(REMOTE_OBJS) %.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXECS)

.PHONY: all clean
