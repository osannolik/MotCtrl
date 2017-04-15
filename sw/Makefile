# Makefile for an stm32f4 project. It uses CMSIS and ST's HAL library.
#
# all: build the whole project
# clean: delete all build products
# fresh: clean and then all
#
# Author: osannolik

#### Project ####
PROJ_NAME = MotCtrl
OUTDIR = build
OBJDIR = $(OUTDIR)/obj
#OUTFILE = $(OUTDIR)/$(notdir $(shell PWD)).elf
OUTFILE = $(OUTDIR)/$(PROJ_NAME).elf
OUTHEX = $(OUTFILE:.elf=.hex)
OUTBIN = $(OUTFILE:.elf=.bin)
OUTLIST = $(OUTFILE:.elf=.lst)
OUTMAP = $(OUTFILE:.elf=.map)

#### Tools ####
TOOL_PREFIX = /usr/local/gcc-arm-none-eabi-4_9-2015q2
PRG_PREFIX = $(TOOL_PREFIX)/bin/arm-none-eabi-
LIB_PREFIX = $(TOOL_PREFIX)/arm-none-eabi/lib/armv7e-m/fpu
MKDIR = mkdir -p
# The libs in /fpu are for use with float-abi = hard

CC = $(PRG_PREFIX)gcc
LD = $(PRG_PREFIX)gcc
CP = $(PRG_PREFIX)objcopy
OD = $(PRG_PREFIX)objdump
SIZE = $(PRG_PREFIX)size

#### Files and folders ####
APPSRCDIR = src
HALSRCDIR = HAL/src
DEVICESRCDIR = CMSIS/stm32f4xx/src
DSPSRCDIR = CMSIS/DSP/src

APPINCDIR = inc
HALINCDIR = HAL/inc
DEVICEINCDIR = CMSIS/stm32f4xx/inc
CMSISINCDIR = CMSIS/inc

SRC = $(wildcard $(APPSRCDIR)/*.c)
SRC += $(wildcard $(HALSRCDIR)/*.c)
SRC += $(wildcard $(DEVICESRCDIR)/*.c)
SRC += $(wildcard $(DSPSRCDIR)/*.c)

OBJ = $(SRC:.c=.o)
OBJFILES = $(addprefix $(OBJDIR)/,$(OBJ))

DEP = $(OBJFILES:.o=.d)

INC = $(wildcard $(APPINCDIR)/*.h)
INC += $(wildcard $(APPINCDIR)/*.h)
INC += $(wildcard $(APPINCDIR)/*.h)
INC += $(wildcard $(APPINCDIR)/*.h)

INCDIRS = ./$(APPINCDIR)
INCDIRS += ./$(HALINCDIR)
INCDIRS += ./$(DEVICEINCDIR)
INCDIRS += ./$(CMSISINCDIR)

#### Compiler and linker flags ####
CFLAGS = $(addprefix -I,$(INCDIRS))
#CFLAGS += -D__FPU_USED
CFLAGS += -Wall -Wdouble-promotion -MMD -MP -g
CFLAGS += -Og
CFLAGS += -fsingle-precision-constant -fno-common -fno-builtin #-ffunction-sections -fdata-sections
CFLAGS += -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb-interwork
#  add -mfix-cortex-m3-ldrd for cortex-m3

LFLAGS = -nostartfiles -gc-sections -Tstm32.ld -L$(LIB_PREFIX) -lm -lc -Xlinker -Map=$(OUTMAP)
#LFLAGS += -nostdlib

#### Rules ####
.PHONY: clean all

all: $(OUTFILE)

fresh: clean all

$(OBJDIR)/%.o: %.c
	@echo "CC $<"
	@$(MKDIR) $(@D)
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OUTFILE): $(OBJFILES)
	@echo "Linking $@"
	@$(CC) $(OBJFILES) $(CFLAGS) $(LFLAGS) -o $@
	@echo "Generating $(OUTLIST)"
	@$(OD) -S $@ > $(OUTLIST)
	@echo "Copy to $(OUTHEX)"
	@$(CP) -O ihex $@ $(OUTHEX)
	@echo "Copy to $(OUTBIN)"
	@$(CP) -O binary $@ $(OUTBIN)
	@echo Done! Size of binary:
	@$(SIZE) $@

clean:
	@echo Cleaning...
	@rm -R -f $(OUTDIR)

-include $(DEP)
