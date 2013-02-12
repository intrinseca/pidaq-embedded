# 	Makefile for compiling the Getting Started project

#-------------------------------------------------------------------------------
#		User-modifiable options
#-------------------------------------------------------------------------------

# Trace level used for compilation
# (can be overriden by adding TRACE_LEVEL=#number to the command-line)
# TRACE_LEVEL_DEBUG      5
# TRACE_LEVEL_INFO       4
# TRACE_LEVEL_WARNING    3
# TRACE_LEVEL_ERROR      2
# TRACE_LEVEL_FATAL      1
# TRACE_LEVEL_NO_TRACE   0
TRACE_LEVEL = 4

# Optimization level
OPTIMIZATION = -O0

# Output file basename
OUTPUT = main

# Output directories
BIN = .
OBJ = obj

# library dirs
LIBRARYSRC = ./lib/src

STARTUPFILE = ./lib/startup_stm32f10x_md.s

#-------------------------------------------------------------------------------
#		Tools
#-------------------------------------------------------------------------------

# Tool suffix when cross-compiling
CROSS_COMPILE = arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
LD = $(CROSS_COMPILE)ld
AS = $(CROSS_COMPILE)as

#-------------------------------------------------------------------------------
#		Files
#-------------------------------------------------------------------------------

# include folders
INCLUDES = -I./
INCLUDES += -I./lib/
INCLUDES += -I./lib/inc/
INCLUDES += -I./lib/inc/../../

# Objects built from C source files
C_OBJECTS = $(OBJ)/main.o
C_OBJECTS += $(OBJ)/system_stm32f10x.o
C_OBJECTS += $(OBJ)/stm32f10x_gpio.o
C_OBJECTS += $(OBJ)/stm32f10x_rcc.o
C_OBJECTS += $(OBJ)/stm32f10x_it.o

# Objects built from Assembly source files
ASM_OBJECTS = $(OBJ)/startup_stm32f10x_md.o

LINKER_SCRIPT = ./lib/stm32_flash.ld
#LINKER_SCRIPT = ./lib/stm32_ram.ld

# Append OBJ and BIN directories to output filename
OUTPUT := $(BIN)/$(OUTPUT)

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

# Flags
CFLAGS = -Wall -fno-common -c -g -mcpu=cortex-m3 -mthumb
CFLAGS += -g $(OPTIMIZATION) $(INCLUDES) -DTRACE_LEVEL=$(TRACE_LEVEL)
ASFLAGS = -g -mapcs-32
LDFLAGS = -g -v -nostartfiles 
#LDFLAGS += -L'D:\OpenOCD_Olimex\yagarto\arm-none-eabi\lib\'
#LDFLAGS += -L'D:/OpenOCD_Olimex/yagarto/lib/gcc/arm-none-eabi/4.5.1/'
OBJCOPYFLAGS = -O binary
OBJDUMPFLAGS = -x --syms -S

all: $(BIN) $(OBJ) $(OUTPUT).out

$(BIN) $(OBJ):
	mkdir $@

$(OUTPUT).out: $(C_OBJECTS) $(ASM_OBJECTS) $(LINKER_SCRIPT)
	@ echo "..linking"
	$(LD) $(LDFLAGS) -Map $(OUTPUT).map -T$(LINKER_SCRIPT) -o $(OUTPUT).out $(C_OBJECTS) $(ASM_OBJECTS) libgcc.a
	$(OBJCOPY) $(OBJCOPYFLAGS) $(OUTPUT).out $(OUTPUT).bin
	$(OBJDUMP) $(OBJDUMPFLAGS) $(OUTPUT).out > $(OUTPUT).list 
	@ echo "...completed."
	
$(C_OBJECTS): main.c system_stm32f10x.c
	@ echo ".compiling"
	$(CC) $(CFLAGS) -o $(OBJ)/main.o main.c
	$(CC) $(CFLAGS) -o $(OBJ)/system_stm32f10x.o system_stm32f10x.c
	$(CC) $(CFLAGS) -o $(OBJ)/stm32f10x_it.o stm32f10x_it.c
	@ echo ".compiling libraries"
	$(CC) $(CFLAGS) -o $(OBJ)/stm32f10x_gpio.o $(LIBRARYSRC)/stm32f10x_gpio.c 
	$(CC) $(CFLAGS) -o $(OBJ)/stm32f10x_rcc.o $(LIBRARYSRC)/stm32f10x_rcc.c 
	
$(ASM_OBJECTS): $(STARTUPFILE)
	@ echo ".assembling"
	$(AS) $(ASFLAGS) -o $(OBJ)/startup_stm32f10x_md.o $(STARTUPFILE)

clean:
	-rm -f $(OBJ)/*.o $(BIN)/*.out $(BIN)/*.bin $(BIN)/*.dmp $(BIN)/*.map