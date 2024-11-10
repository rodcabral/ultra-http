FLAGS = -Wall -Wextra -fsanitize=address

BUILD = ./build

$(BUILD)/ultra.o: build ./src/ultra.c ./src/ultra.h
	$(CC) $(FLAGS) -c ./src/ultra.c -o $(BUILD)/ultra.o

./examples/json: $(BUILD)/ultra.o ./examples/json-test.c ./src/ultra-json/ultra-json.c
	$(CC) $(FLAGS) ./examples/json-test.c ./src/ultra-json/ultra-json.c $(BUILD)/ultra.o -o ./examples/json

./examples/hello: $(BUILD)/ultra.o ./examples/hello.c
	$(CC) $(FLAGS) ./examples/hello.c $(BUILD)/ultra.o -o ./examples/hello

examples: ./examples/hello ./examples/json

build:
	mkdir build

clean:
	rm -rf ./build
	rm -f ./examples/hello ./examples/json
