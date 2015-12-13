CPPFLAGS=-O2 -Wall -march=native
CFLAGS=-O2 -Wall -march=native
LDFLAGS=-lSDL2
REMOTE_OBJS=Remote.o runner_unix.o
SERIAL_OBJS=Remote.o runner_serial.o
EXECS=test_all_pixel_unix game_of_life_unix scroller_unix remote_test \
	  test_all_pixel_serial game_of_life_serial scroller_serial \
	  snake_unix

all: $(EXECS)

%_serial: $(SERIAL_OBJS) %.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%_unix: $(REMOTE_OBJS) %.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

remote_test: Remote.o remote_main.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXECS) *.o

.PHONY: all clean
