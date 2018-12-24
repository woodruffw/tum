CC := clang
RM := rm -rf

CFLAGS := -Wall -Wextra -Wno-unused-result -O2 -std=c99 -DDEBUG -D_POSIX_C_SOURCE=200809 -fsanitize=undefined

.PHONY: all
all: tmachine tasm

tmachine: src/machine.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

tasm: src/asm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) tmachine tasm

.PHONY: fmt
fmt:
	clang-format -i -style=file src/*
