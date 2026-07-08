
CROSS_COMPILE ?= riscv64-unknown-elf-
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
LD:=$(CROSS_COMPILE)ld
OBJCOPY:=$(CROSS_COMPILE)objcopy
OBJDUMP:=$(CROSS_COMPILE)objdump
READELF:=$(CROSS_COMPILE)readelf

TARGET_PROFILE ?= generic
ifneq ($(TARGET_PROFILE),generic)
target_dir:=targets/$(TARGET_PROFILE)
ifeq ($(wildcard $(target_dir)/target.mk),)
$(error unsupported target profile $(TARGET_PROFILE))
endif
include $(target_dir)/target.mk
endif

ifneq ($(MAKECMDGOALS), clean)
ifeq ($(PLAT),)
$(error Undefined platform)
endif
endif

log_dir:=log
build_dir:=build/$(PLAT)
plat_dir:=platform/$(PLAT)
TEST_REGISTER_SRC ?= test_register.c
CASE_ROOTS ?= test_cases/manual_test_cases test_cases/ai_test_cases manual_test_cases ai_test_cases
CASE_LINK_MODE ?= registered
CASE_C_SRCS ?=
CASE_ASM_SRCS ?=
PAGE_TABLE_BACKEND ?= static
PAGE_TABLE_MODE ?= sv39
SAVE_PREPROCESSED ?= 0
GENERATE_BIN ?= 1
GENERATE_ASM ?= 1
GENERATE_DUMP ?= 1
GENERATE_READELF ?= 1
ifeq ($(wildcard $(plat_dir)),)
$(error unsupported platform $(PLAT))
else
$(shell mkdir -p $(log_dir))
$(shell mkdir -p $(build_dir))
endif

prev_log_file:=$(build_dir)/prev_log.mk
-include $(prev_log_file)
LOG_LEVEL ?= LOG_INFO
GENERIC_FLAGS += -D LOG_LEVEL=$(LOG_LEVEL)
$(file > $(prev_log_file), PREV_LOG_LEVEL:=$(LOG_LEVEL))
ifneq ($(PREV_LOG_LEVEL), $(LOG_LEVEL))
pre_targets += clean_objs
endif

TARGET := $(build_dir)/rvh_test
ifeq ($(PAGE_TABLE_BACKEND),static)
page_table_src := src/page_tables.c
GENERIC_FLAGS += -D$(PAGE_TABLE_MODE)
else ifeq ($(PAGE_TABLE_BACKEND),dynamic)
page_table_src := src/dynamic_page_tables.c
else
$(error unsupported PAGE_TABLE_BACKEND $(PAGE_TABLE_BACKEND))
endif

core_c_srcs := src/main.c $(page_table_src) src/rvh_test.c src/instruction.c $(TEST_REGISTER_SRC)
test_c_srcs := $(sort $(foreach root,$(CASE_ROOTS),$(shell find $(root) -name '*.c' -print 2>/dev/null)))
test_asm_srcs := $(sort $(foreach root,$(CASE_ROOTS),$(shell find $(root) -name '*.S' -print 2>/dev/null)))
dynamic_case_c_srcs := $(sort $(shell grep -rl 'dynamic_page_tables.h' $(CASE_ROOTS) --include='*.c' 2>/dev/null))
static_case_c_srcs := $(filter-out $(dynamic_case_c_srcs),$(test_c_srcs))
registered_case_names := $(sort $(shell sed -n 's/^[[:space:]]*TEST_REGISTER([[:space:]]*\([A-Za-z_][A-Za-z0-9_]*\)[[:space:]]*)[[:space:]]*;[[:space:]]*$$/\1/p' $(TEST_REGISTER_SRC) 2>/dev/null))
registered_c_srcs := $(sort $(shell python3 scripts/list_registered_sources.py --repo-root . --register $(TEST_REGISTER_SRC) --roots $(CASE_ROOTS)))
registered_asm_srcs := $(sort $(foreach src,$(registered_c_srcs),$(wildcard $(dir $(src))*.S)))
ifeq ($(CASE_LINK_MODE),selected)
case_c_srcs := $(sort $(CASE_C_SRCS))
case_asm_srcs := $(sort $(CASE_ASM_SRCS))
else ifeq ($(CASE_LINK_MODE),registered)
case_c_srcs := $(registered_c_srcs)
case_asm_srcs := $(registered_asm_srcs)
else
ifeq ($(PAGE_TABLE_BACKEND),static)
case_c_srcs := $(static_case_c_srcs)
else
case_c_srcs := $(test_c_srcs)
endif
case_asm_srcs := $(test_asm_srcs)
endif
c_srcs := $(core_c_srcs) $(case_c_srcs)\
	$(addprefix $(plat_dir)/, $(notdir $(wildcard $(plat_dir)/*.c)))
asm_srcs := asm/boot.S asm/handlers.S $(case_asm_srcs) $(wildcard $(plat_dir)/*.S)
ld_file:=linker.ld
inc_dirs := ./inc $(plat_dir)/inc
inc_dirs := $(addprefix -I, $(inc_dirs))

objs:=
objs+=$(patsubst  %.c, $(build_dir)/%.o, $(c_srcs))
objs+=$(patsubst  %.S, $(build_dir)/%.o, $(asm_srcs))
ld_file_final:=$(build_dir)/$(ld_file)

deps:=$(patsubst  %.o, %.d, $(objs)) $(ld_file_final).d
dirs:=$(sort $(dir $(objs) $(deps)))

TEST_MABI ?= lp64
# For RISC-V GCC versions greater than 10, instructions such as csrwi are
# grouped separately under the zicsr extension and need to be specified
# explicitly by appending '_zicsr' to the -march parameter. Below, the
# variable GCCVERSION only gets defined if the GCC version is > 10.
# See https://github.com/josecm/riscv-hyp-tests/pull/8 for more info
ifeq ($(TEST_MARCH),)
GCCVERSION := $(shell $(CC) -dumpversion | awk -F. '$$1 > 10 { print $$1 }')
ifdef GCCVERSION
	TEST_MARCH := rv64imac_zicsr
else
	TEST_MARCH := rv64imac
endif
endif
GENERIC_FLAGS += -march=$(TEST_MARCH) -mabi=$(TEST_MABI) -g3 -mcmodel=medany -O3 -ffunction-sections -fdata-sections $(inc_dirs)

ASFLAGS = $(GENERIC_FLAGS)
CFLAGS = $(GENERIC_FLAGS)
LDFLAGS = -ffreestanding -nostartfiles -static -Wl,--gc-sections -Wl,--orphan-handling=warn $(GENERIC_FLAGS)

$(build_dir)/$(plat_dir)/syscalls.o: CFLAGS += -ffreestanding -fno-tree-vectorize -fno-tree-loop-distribute-patterns

ifeq ($(GENERATE_BIN),1)
final_target := $(TARGET).bin
else
final_target := $(TARGET).elf
endif

all: $(pre_targets) $(final_target)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).elf: $(objs) $(ld_file_final)
	$(CC) $(LDFLAGS) -T$(ld_file_final) $(objs) -o $@
	$(if $(filter 1,$(GENERATE_ASM)),$(OBJDUMP) -S $@ > $(TARGET).asm,)
	$(if $(filter 1,$(GENERATE_DUMP)),$(OBJDUMP) -D $@ > $(TARGET).dump,)
	$(if $(filter 1,$(GENERATE_READELF)),$(READELF) -a -W $@ > $(@).txt,)

$(build_dir)/%.o: %.[c,S] $(build_dir)/%.d
	$(if $(filter 1,$(SAVE_PREPROCESSED)),$(CC) $(CFLAGS) -E -c $< -o $@.c,)
	$(CC) $(CFLAGS) -c $< -o $@

$(build_dir)/%.d: %.[c,S,ld] 
	$(CC) $(GENERIC_FLAGS) -MM -MT "$(patsubst %.d, %.o, $@) $@" -MF $@ $<  

$(ld_file_final): $(ld_file)
	$(CC) $(CFLAGS) -E -x assembler-with-cpp $< | grep "^[^#;]" > $@
	
.SECONDEXPANSION:

$(objs) $(deps): | $$(@D)/

$(dirs):
	mkdir -p $@

ifneq ($(MAKECMDGOALS), clean)
-include $(deps)
endif

clean:
	rm -rf $(build_dir)
	rm -rf $(log_dir)

clean_objs:
	find $(build_dir) -type f \( -name '*.o' -o -name '*.o.c' -o -name '*.d' \) -delete

.PHONY: all clean
