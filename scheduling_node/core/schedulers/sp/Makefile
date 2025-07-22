RISCV_PREFIX ?= riscv32-unknown-elf
GCC = $(RISCV_PREFIX)-gcc
OBJCOPY = $(RISCV_PREFIX)-objcopy

LSCRIPT = ../lscript
BOOTSTRAP = ../bootstrap.s
SREC2TEXT = ../srec2text.py

C_SRC = notmain.c
ELF = notmain.elf
SREC = notmain.srec
TXT = notmain.txt

all: $(TXT)

$(ELF): $(C_SRC) $(BOOTSTRAP) $(LSCRIPT)
	$(GCC) -O3 -march=rv32ima -mabi=ilp32 -T $(LSCRIPT) $(BOOTSTRAP) $(C_SRC) -o $(ELF) -nostdlib

$(SREC): $(ELF)
	$(OBJCOPY) -O srec --gap-fill 0 $(ELF) $(SREC)

$(TXT): $(SREC) $(SREC2TEXT)
	python3 $(SREC2TEXT) $(SREC) > $(TXT)

clean:
	rm -f $(ELF) $(SREC) $(TXT)

.PHONY: all clean