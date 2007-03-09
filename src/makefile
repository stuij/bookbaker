# --- Project details ---
PROJ    := book
OBJS    := $(PROJ).o libgbfs.o font.o

# --- Compiling defines ---
CC      := arm-eabi-gcc
LD      := arm-eabi-gcc
OBJCOPY := arm-eabi-objcopy

MODEL   := -mthumb-interwork -mthumb
EXT     := pre.gba
SPECS   := -specs=gba_mb.specs

CFLAGS  := $(MODEL) $(INCLUDES) -O2 -Wall
LDFLAGS := $(SPECS) $(MODEL)

# --- Build rules ---
# first rule; make process starts here
build: $(PROJ).$(EXT)

# translate step: .elf -> (.mb).gba
$(PROJ).$(EXT) : $(PROJ).elf
	$(OBJCOPY) -v -O binary $< $@
	gbafix $@
	padbin 256 $@

# link step: .o -> .elf
$(PROJ).elf : $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

# compile step .c -> .o
$(OBJS) : %.o : %.c
	$(CC) -c $< $(CFLAGS) -o $@

# --- Clean rules ---
.PHONY : clean
clean : 
	rm -fv $(COBJS)
	rm -fv $(PROJ).$(EXT) 
	rm -fv $(PROJ).elf