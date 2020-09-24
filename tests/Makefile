ACC = ../acc
CC = cc
RM = rm -f

files   := \
		test_0000 test_0001 test_0002 test_0003 test_0004 \
		test_0005 test_0006 test_0007
targets := $(files)

.PHONY: all clean $(targets)
all: $(targets)

test: all

$(targets): %: %.c
	@echo -n '  $< ==> '
	@$(ACC) $<
	@$(CC) -o $@.out $@.s
	@./$@.out && echo "\033[0;32mOK\033[0;39m"

clean:
	$(RM) $(targets) *.s *.out
