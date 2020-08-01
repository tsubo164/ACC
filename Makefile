.PHONY: clean run test

mcc: main.o parse.o lexer.o
	gcc -o $@ $^

main.o: main.c parse.h lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<

parse.o: parse.c parse.h lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<

lexer.o: lexer.c lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<

clean:
	rm -f mcc a.out *.o *.s *-in.c test-lex

run:
	./mcc input.c
	gcc input.s
	./a.out

test: test-lex
	@./test-lex

test-lex: test-lex.o lexer.o
	gcc -o $@ $^

test-lex.o: test-lex.c lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<
