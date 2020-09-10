CFLAGS = -Wall -Werror -Wpedantic `pkg-config --cflags dbus-1`
LFLAGS = `pkg-config --libs dbus-1`
CC = gcc
MEM = valgrind -s --leak-check=full
leakCheck: CFLAGS += -fsanitize=address
leakCheck: LFLAGS += -fsanitize=address
leakCheck: CC = clang

.PHONY: leakCheck

audioSync: audioSync.o
	$(CC) audioSync.o -o audioSync $(LFLAGS)

leakCheck: audioSync
	ASAN_OPTIONS=detect_leaks=1 ./audioSync

audioSync.o: audioSync.c
	$(CC) audioSync.c -c -o audioSync.o $(CFLAGS)

clean:
	rm -f *.o audioSync audioSyncTest
