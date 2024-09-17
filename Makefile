FLAGS = -Wall -Wextra -fsanitize=address

BUILD = ./build

$(BUILD)/ultra.o: build ./src/ultra.c ./src/ultra.h
	$(CC) $(FLAGS) -c ./src/ultra.c -o $(BUILD)/ultra.o

./examples/example: $(BUILD)/ultra.o ./examples/hello.c
	$(CC) $(FLAGS) ./examples/hello.c $(BUILD)/ultra.o -o ./examples/example

build:
	mkdir build

clean:
	rm -rf ./build
	rm -rf ./examples/example
