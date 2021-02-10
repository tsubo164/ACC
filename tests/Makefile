ACC = ../acc
CC = cc
RM = rm -f

files   := \
		test_0002 test_0007 \
		test_arith test_array test_enum test_global test_goto test_for \
		test_incdec test_pointer test_string test_struct test_switch test_type \
		test_while
targets := $(files)

.PHONY: all clean test $(targets)
all: $(targets)

test: all

$(targets): %: %.c test.o
	@printf '%-15s ... ' $<
	@$(ACC) $<
	@$(CC) -c -o $@.o $@.s
	@$(CC) -o $@.out $@.o test.o
	@./$@.out && echo "\033[0;32mOK\033[0;39m"

test.o: test.c
	@echo 'compiling test.c ..'
	@$(ACC) $<
	@$(CC) -c -o test.o test.s

clean:
	$(RM) $(targets) *.s *.out *.o
