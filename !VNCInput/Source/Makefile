
# Details on the target file
TARGET        = ^.!RunImage
TARGET_BINARY = $(TARGET)
# Objects
OBJS = Buttons.o d3des.o Main.o Sockets.o VNC.o Window.o \
       socket.o hton-ntoh.o wimp.o

# Libraries
LIBS = C:o.stubs


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

ASMFLAGS = -processor ARM7

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
