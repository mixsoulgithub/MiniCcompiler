SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

miniCcompiler:$(SRC)
	gcc -o $@ $^

$(SRC):mini.h

test: miniCcompiler
	./test.sh

clean:
	rm -f miniCcompiler *.o *~ *.s tmp* *.out

.PHONY: clean test