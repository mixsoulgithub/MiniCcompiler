miniCcompiler:main.c
	gcc -o $@ $<

test: miniCcompiler
	./test.sh

clean:
	rm -f miniCcompiler *.o *~ *.s tmp* *.out

.PHONY: clean test