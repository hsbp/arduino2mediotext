CPPFLAGS=-O2 -Wall -march=native
CFLAGS=-O2 -Wall -march=native
REMOTE_OBJS=Remote.o runner_unix.o
SERIAL_OBJS=Remote.o runner_serial.o
EXECS=test_all_pixel_unix game_of_life_unix scroller_unix remote_test \
	  test_all_pixel_serial game_of_life_serial scroller_serial

all: $(EXECS)

%_serial: $(SERIAL_OBJS) %.o
	$(CC) $(CFLAGS) $^ -o $@

%_unix: $(REMOTE_OBJS) %.o
	$(CC) $(CFLAGS) $^ -o $@

remote_test: Remote.o remote_main.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXECS) *.o

.PHONY: all clean
