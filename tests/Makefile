ACC = ../acc
CC = cc
RM = rm -f

files   := \
		test_0001 test_0002 test_0004 test_0005 test_0007 test_0008 \
		test_arith test_enum test_for test_incdec test_string test_switch \
		test_type test_while
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
