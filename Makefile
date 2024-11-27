FLAGS = -Wall -Wextra -fsanitize=address

./examples/hello: ./examples/hello.c ultra.h
	$(CC) $(FLAGS) ./examples/hello.c -o ./examples/hello

examples: ./examples/hello

clean:
	rm -f ./examples/hello
