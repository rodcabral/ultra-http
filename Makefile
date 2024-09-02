FLAGS = -Wall -Wextra

BUILD = ./build

$(BUILD)/ultra.o: build ultra.c ultra.h
	$(CC) $(FLAGS) -c ultra.c -o $(BUILD)/ultra.o

example: $(BUILD)/ultra.o ./examples/hello.c
	$(CC) $(CFLAGS) ./examples/hello.c $(BUILD)/ultra.o -o example

build:
	mkdir build

clean:
	rm -rf *.o
	rm -rf ./build
	rm example
