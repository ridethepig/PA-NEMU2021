include $(NEMU_HOME)/scripts/git.mk
include $(NEMU_HOME)/scripts/build.mk

include $(NEMU_HOME)/tools/difftest.mk

compile_git:
	$(call git_commit, "compile")
$(BINARY): compile_git

# Some convenient rules

override ARGS ?= --log=$(BUILD_DIR)/nemu-log.txt
override ARGS += $(ARGS_DIFF)

# Command to execute NEMU
IMG ?= /home/unix/Code/ics2021/am-kernels/tests/cpu-tests/build/bit-riscv64-nemu.bin
ELF ?= /home/unix/Code/ics2021/am-kernels/tests/cpu-tests/build/bit-riscv64-nemu.elf

NEMU_EXEC := $(BINARY) $(ARGS) -i $(IMG)
NEMU_EXEC_ELF := $(BINARY) $(ARGS) -e $(ELF)

run-env: $(BINARY) $(DIFF_REF_SO)

run: run-env
	$(call git_commit, "run")
	$(NEMU_EXEC)

run-elf: run-env
	$(NEMU_EXEC_ELF)

gdb: run-env
	$(call git_commit, "gdb")
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean-tools = $(dir $(shell find ./tools -maxdepth 2 -mindepth 2 -name "Makefile"))
$(clean-tools):
	-@$(MAKE) -s -C $@ clean
clean-tools: $(clean-tools)
clean-all: clean distclean clean-tools

.PHONY: run run-elf gdb run-env clean-tools clean-all $(clean-tools)
