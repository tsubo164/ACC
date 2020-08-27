CC = gcc
OPT = -O2
CFLAGS = $(OPT) -Wall -ansi --pedantic-errors -c

.PHONY: all clean run run_cc test

all: acc

acc: main.o parse.o lexer.o symbol.o
	$(CC) -o $@ $^

main.o: main.c parse.h lexer.h symbol.h
	$(CC) $(CFLAGS) $<

parse.o: parse.c parse.h lexer.h symbol.h
	$(CC) $(CFLAGS) $<

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) $<

symbol.o: symbol.c symbol.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f acc a.out *.o *.s
	@$(MAKE) --no-print-directory -C tests $@

run: acc
	./acc input.c
	$(CC) input.s
	./a.out

run_cc:
	$(CC) -Wall -ansi --pedantic-errors input.c
	./a.out

test: acc
	@$(MAKE) --no-print-directory -C tests $@

