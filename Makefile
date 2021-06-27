TARGET = netassist
DEBUG = true

CC = gcc

ifeq ($(DEBUG), true)
	COMPILE_FLAGS = -g -Wall -std=gnu11
	LINK_FLAGS = -luv
else
	COMPILE_FLAGS = -O3 -std=gnu11
	LINK_FLAGS = -luv_a -lpthread -ldl -static
endif

$(TARGET): main.o my_options.o wrapper.o callback.o
	$(CC) $^ -o $@ $(LINK_FLAGS)
main.o: main.c my_options.h wrapper.h
	$(CC) -c $< $(COMPILE_FLAGS)
my_options.o: my_options.c my_options.h wrapper.h
	$(CC) -c $< $(COMPILE_FLAGS)
wrapper.o: wrapper.c wrapper.h
	$(CC) -c $< $(COMPILE_FLAGS)
callback.o: callback.c
	$(CC) -c $< $(COMPILE_FLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o