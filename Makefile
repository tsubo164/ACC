CC      = gcc
OPT     = -O2
CFLAGS  = $(OPT) -Wall -ansi --pedantic-errors -c
LDFLAGS = 
RM      = rm -f

target_name := acc
files       := ast gen_x86 lexer main message parse semantics symbol type

target  := $(target_name)
sources := $(addsuffix .c, $(files))
objects := $(addsuffix .o, $(files))
depends := $(addsuffix .d, $(files))

.PHONY: all clean run run_cc test

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

test: $(target)
	@$(MAKE) --no-print-directory -C tests $@

$(depends): %.d: %.c
	@echo '  dependency $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(depends)
endif
