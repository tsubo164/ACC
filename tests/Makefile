ACC = ../mcc
CC = cc
RM = rm -f

files   := test_0000 test_0001
targets := $(files)

.PHONY: all clean $(targets)
all: $(targets)

$(targets): %: %.c
	@echo -n '  $< ==> '
	@$(ACC) $<
	@$(CC) -o $@ $@.s
	@./$@ && echo "\033[0;32mOK\033[0;39m"

clean:
	$(RM) $(targets) *.s
