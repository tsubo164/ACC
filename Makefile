.PHONY: clean

mcc: main.c
	gcc -O2 -Wall -std=c89 --pedantic-errors -o $@ $<

run:
	./mcc input.mc
	gcc input.s

clean:
	rm -f mcc a.out *.s
