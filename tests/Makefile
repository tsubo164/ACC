ACC = ../acc
CC = cc
RM = rm -f

SRCS   := \
		array enum expr func global goto for local macro pointer string \
		struct switch type union while
TARGETS := $(SRCS)

.PHONY: all clean test $(TARGETS)
all: $(TARGETS)

test: all
	@echo "\033[0;32mOK\033[0;39m"

$(TARGETS): %: %.c test.o test.h gcc_func.o
	$(ACC) -S $@.c
	$(CC) -c -o $@.o $@.s
	$(CC) -o $@.out $@.o test.o gcc_func.o
	./$@.out

test.o: test.c test.h
	$(ACC) -S test.c
	$(CC) -c -o test.o test.s

gcc_func.o: gcc_func.c gcc_func.h
	gcc -Wall --pedantic-errors -c gcc_func.c

clean:
	$(RM) $(TARGETS) *.s *.out *.o
