.PHONY: clean test

mcc: main.c
	gcc -O2 -Wall -std=c89 --pedantic-errors -o $@ $<

test:
	./mcc input.c
	gcc input.s
	./a.out

clean:
	rm -f mcc a.out *.s
