CC      = cc
OPT     = -O2
#OPT     = -O0 -g
CFLAGS  = $(OPT) -Wall -ansi --pedantic-errors -c
LDFLAGS = 
RM      = rm -f

SRCS    := ast esc_seq gen_x64 lexer main message parse preprocessor \
					 semantics string_table symbol type

.PHONY: all run run_cc tree pp test test2 test3 test_all clean clean2 clean3

#-------------------------------------------------------------------------------
# stage 1
ACC  := acc
OBJS := $(addsuffix .o, $(SRCS))
DEPS := $(addsuffix .d, $(SRCS))

all: $(ACC)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(ACC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean: clean3 clean2
	$(RM) $(ACC) a.out *.o *.s *.d
	$(MAKE) -C tests $@

test: $(ACC)
	$(MAKE) -C tests $@
	@echo stage1

#-------------------------------------------------------------------------------
# stage 2
ACC2 := acc2
OBJS2 := $(addsuffix .2.o, $(SRCS))
ASMS2 := $(addsuffix .2.s, $(SRCS))
SRCS2 := $(addsuffix .2.c, $(SRCS))

$(ACC2): $(OBJS2)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS2): %.o: %.s
	$(CC) -c -o $@ $<

$(ASMS2): %.2.s: %.2.c
	./$(ACC) $^

$(SRCS2): %.2.c: %.c
	cp $< $@

test2: $(ACC2)
	mkdir -p stage2
	cp tests/*.c stage2/
	cp tests/*.h stage2/
	cp tests/Makefile stage2/
	$(MAKE) -C stage2 test ACC=../$<
	@echo stage2

clean2:
	$(RM) $(ACC2) *.2.c *.2.s *.2.o
	$(RM) -r stage2

#-------------------------------------------------------------------------------
# stage 3
ACC3 := acc3
OBJS3 := $(addsuffix .3.o, $(SRCS))
ASMS3 := $(addsuffix .3.s, $(SRCS))
SRCS3 := $(addsuffix .3.c, $(SRCS))

$(ACC3): $(OBJS3)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS3): %.o: %.s
	$(CC) -c -o $@ $<

$(ASMS3): %.3.s: %.3.c
	./$(ACC2) $^

$(SRCS3): %.3.c: %.c
	cp $< $@

test3: $(ACC3)
	mkdir -p stage3
	cp tests/*.c stage3/
	cp tests/*.h stage3/
	cp tests/Makefile stage3/
	$(MAKE) -C stage3 test ACC=../$<
	@echo stage3
	diff $(ACC2) $(ACC3)

clean3:
	$(RM) $(ACC3) *.3.c *.3.s *.3.o
	$(RM) -r stage3

#-------------------------------------------------------------------------------
# test all
test_all: test test2 test3


#-------------------------------------------------------------------------------
# debug
run: $(ACC)
	./$(ACC) input.c
	$(CC) input.s
	./a.out

run_cc:
	$(CC) -Wall -ansi --pedantic-errors input.c
	./a.out

tree: $(ACC)
	./$(ACC) --print-tree input.c

pp: $(ACC)
	./$(ACC) --print-preprocess input.c

test_cc:
	@echo "\033[0;31m*** testing with cc ***\033[0;39m"
	$(MAKE) --no-print-directory -C tests test ACC='cc -S'

$(DEPS): %.d: %.c
	$(CC) -I$(incdir) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
