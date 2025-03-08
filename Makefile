SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

miniCcompiler:$(SRC)
	gcc -o $@ $^

$(SRC):mini.h

test: miniCcompiler
	./test.sh

miniCcompiler_d: $(SRC)
	gcc -g -o $@ $^

debug:miniCcompiler_d
	gdb ./miniCcompiler_d

clean:
	rm -f miniCcompiler *.o *~ *.s tmp* *.out

.PHONY: clean test