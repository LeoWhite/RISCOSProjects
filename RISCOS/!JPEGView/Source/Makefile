
# Details on the target file
TARGET        = ^.!RunImage
TARGET_BINARY = $(TARGET)
# Objects
OBJS = Button.o Display.o heap.o JPEGdecode.o JPEGsrcman.o \
	Wimp.o Main.o

# Libraries
LIBS = C:o.stubs ^.^.^.Libraries.jpeg-6b.libjpeg


# What tools to use.
CC	= cc
ASM	= asm
LD	= link $(LDFlags)
RM	= rm -rf
MKDIR	= mkdir -p
CD	= cd
PERL    = perl -w

# C Flags
CFLAGS = -W+adf -D__riscos__ -DRISCOS -throwback

# Linker Flags
LDFLAGS = -rescan -aif

ASMFLAGS = -processor ARM6

# Debug flags
#CFLAGS += -DVNC_DEBUG -g 
#LDFLAGS += -Debug

all: $(TARGET_BINARY)
	echo target $(TARGET)
	
# Rule to build the final target binary	
$(TARGET_BINARY): $(OBJS)
	@echo '### Linking $(TARGET_BINARY)'
	@${LD} ${LDFLAGS} -Output "$(TARGET_BINARY)" $(OBJS) $(LIBS)

# Rule to compile the .c files
.c.o:
	@echo '### Compiling $(@F)'
	@${CC} $(INC_DIR) ${CFLAGS} -c $<

.a.o:
	@echo '### Assembling $(@F)'
	$(ASM) $(ASMFLAGS) -Output $@ $<

# Rule to clean the directory
clean:
	$(RM) $(OBJS) "$(TARGET_BINARY)"
