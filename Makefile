.PHONY: acc clean run test

all: acc

acc: main.o parse.o lexer.o
	gcc -o $@ $^

main.o: main.c parse.h lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<

parse.o: parse.c parse.h lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<

lexer.o: lexer.c lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<

clean:
	rm -f acc a.out *.o *.s *_in.c test_lex
	@$(MAKE) --no-print-directory -C tests $@

run: acc
	./acc input.c
	gcc input.s
	./a.out

test: acc
	@$(MAKE) --no-print-directory -C tests $@

