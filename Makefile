CC      = gcc
OPT     = -O2
CFLAGS  = $(OPT) -Wall -ansi --pedantic-errors -c
LDFLAGS = 
RM      = rm -f

target_name := acc
files       := ast gen_x86 lexer main message parse preprocessor \
               semantics string_table symbol type

target  := $(target_name)
sources := $(addsuffix .c, $(files))
objects := $(addsuffix .o, $(files))
depends := $(addsuffix .d, $(files))

.PHONY: all clean run run_cc tree test

all: $(target)

$(objects): %.o: %.c
	@echo '  compile $<'
	@$(CC) $(CFLAGS) -c -o $@ $<

$(target): $(objects)
	@echo '  link    $@'
	@$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(target) a.out *.o *.s *.d
	@$(MAKE) --no-print-directory -C tests $@

run: $(target)
	./$(target) input.c
	$(CC) input.s
	./a.out

run_cc:
	$(CC) -Wall -ansi --pedantic-errors input.c
	./a.out

#target for testing single file for self compile
SELF =
self: $(SELF).c
	@$(MAKE)
	@echo '  * self compile $(SELF).c'
	@./$(target) $(SELF).c
	@$(CC) -c $(SELF).s
	@$(MAKE)

tree: $(target)
	./$(target) --print-tree input.c

pp: $(target)
	./$(target) --print-preprocess input.c

test: $(target)
	@$(MAKE) --no-print-directory -C tests $@

test_cc: $(target)
	@echo "\033[0;31m*** testing with cc ***\033[0;39m"
	@$(MAKE) --no-print-directory -C tests test ACC='cc -S'

$(depends): %.d: %.c
	@echo '  dependency $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(depends)
endif
