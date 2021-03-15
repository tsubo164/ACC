ACC = ../acc
CC = cc
RM = rm -f

files   := \
		test_arith test_array test_enum test_global test_goto test_for \
		test_incdec test_local test_macro test_pointer test_string \
		test_struct test_switch test_type test_while
targets := $(files)

.PHONY: all clean test $(targets)
all: $(targets)

test: all

$(targets): %: %.c test.o test.h
	@printf '%-15s ... ' $@.c
	@$(ACC) $@.c
	@$(CC) -c -o $@.o $@.s
	@$(CC) -o $@.out $@.o test.o
	@./$@.out && echo "\033[0;32mOK\033[0;39m"

test.o: test.c test.h
	@echo 'compiling test.c ..'
	@$(ACC) test.c 
	@$(CC) -c -o test.o test.s

clean:
	$(RM) $(targets) *.s *.out *.o
