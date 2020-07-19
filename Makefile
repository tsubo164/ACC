.PHONY: clean

mcc: main.c
	gcc -O2 -Wall -std=c89 --pedantic-errors -o $@ $<

clean:
	rm -f mcc a.out a.s
