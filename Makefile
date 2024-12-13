FLAGS = -Wall -Wextra -O3 -fsanitize=address

hello: ./examples/hello.c $(wildcard ./src/*.c) $(wildcard ./src/*.h)
	$(CC) $(FLAGS) ./examples/hello.c $(wildcard ./src/*.c) -o hello

examples: hello

clean:
	rm -f hello
