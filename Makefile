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
	rm -f mcc a.out *.o *.s *_in.c test_lex

run:
	./mcc input.c
	gcc -c input.s
	gcc -c func.c
	gcc input.o func.o
	./a.out

#run:
#	./mcc input.c
#	gcc input.s
#	./a.out

test: test_lex
	@./test_lex

test_lex: test_lex.o lexer.o
	gcc -o $@ $^

test_lex.o: test_lex.c lexer.h
	gcc -O2 -Wall -std=c89 --pedantic-errors -c $<
