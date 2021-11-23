CC      = cc
OPT     = -O2
#OPT     = -O0 -g
CFLAGS  = $(OPT) -Wall -ansi --pedantic-errors -c
LDFLAGS = 
RM      = rm -f

SRCS    := ast gen_x86 lexer main message parse preprocessor \
					 semantics string_table symbol type

ACC  := acc
OBJS := $(addsuffix .o, $(SRCS))
DEPS := $(addsuffix .d, $(SRCS))

.PHONY: all clean run run_cc tree test self

all: $(ACC)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(ACC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(ACC) a.out *.o *.s *.d
	$(MAKE) --no-print-directory -C tests $@

run: $(ACC)
	./$(ACC) input.c
	$(CC) input.s
	./a.out

run_cc:
	$(CC) -Wall -ansi --pedantic-errors input.c
	./a.out

#target for testing single file for self compile
SELF =
self: $(SELF).c
	./$(ACC) $(SELF).c
	$(CC) -c $(SELF).s
	$(CC) -o $(ACC) $(OBJS) $(LDFLAGS)

tree: $(ACC)
	./$(ACC) --print-tree input.c

pp: $(ACC)
	./$(ACC) --print-preprocess input.c

test: $(ACC)
	$(MAKE) --no-print-directory -C tests $@

test_cc:
	@echo "\033[0;31m*** testing with cc ***\033[0;39m"
	$(MAKE) --no-print-directory -C tests test ACC='cc -S'

$(DEPS): %.d: %.c
	$(CC) -I$(incdir) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
