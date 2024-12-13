FLAGS = -Wall -Wextra -O3 -fsanitize=address

hello: ./examples/hello.c ./src/ultra.c ./src/ultra.h
	$(CC) $(FLAGS) ./examples/hello.c ./src/ultra.c -o hello

examples: hello

clean:
	rm -f hello
