# Makefile for Independent JPEG Group's software

# This makefile is suitable for Unix-like systems with ANSI-capable compilers.
# If you have a non-ANSI compiler, makefile.unix is a better starting point.

# Read installation instructions before saying "make" !!

# The name of your C compiler:
CC= cc 

# You may need to adjust these cc options:
CFLAGS= -Otime -throwback -depend !Depend -DDEFAULT_MAX_MEM=25000
# Generally, we recommend defining any configuration symbols in jconfig.h,
# NOT via -D switches here.

# Link-time cc options:
LDFLAGS= 

# To link any special libraries, add the necessary -l commands here.
LDLIBS= C:o.stubs 

# Put here the object file name for the correct system-dependent memory
# manager file.  For Unix this is usually jmemnobs.o, but you may want
# to use jmemansi.o or jmemname.o if you have limited swap space.
SYSDEPMEM= jmemansi.o

# miscellaneous OS-dependent stuff
# linker
LN= link
# file deletion command
RM= wipe
# library (.a) file creation command
AR= libfile -c
AR2= 

# End of configurable options.


# source files: JPEG library proper
LIBSOURCES= jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
        jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
        jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
        jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
        jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
        jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
        jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
        jquant2.c jutils.c jmemmgr.c
# memmgr back ends: compile only one of these into a working library
SYSDEPSOURCES= jmemansi.c jmemname.c jmemnobs.c jmemdos.c jmemmac.c
# source files: cjpeg/djpeg/jpegtran applications, also rdjpgcom/wrjpgcom
APPSOURCES= cjpeg.c djpeg.c jpegtran.c rdjpgcom.c wrjpgcom.c cdjpeg.c \
        rdcolmap.c rdswitch.c transupp.c rdppm.c wrppm.c rdgif.c wrgif.c \
        rdtarga.c wrtarga.c rdbmp.c wrbmp.c rdrle.c wrrle.c wrsprite.c
SOURCES= $(LIBSOURCES) $(SYSDEPSOURCES) $(APPSOURCES)
# files included by source files
INCLUDES= jchuff.h jdhuff.h jdct.h jerror.h jinclude.h jmemsys.h jmorecfg.h \
        jpegint.h jpeglib.h jversion.h cdjpeg.h cderror.h transupp.h

# library object files common to compression and decompression
COMOBJECTS= jcomapi.o jutils.o jerror.o jmemmgr.o $(SYSDEPMEM)

# compression library object files
CLIBOBJECTS= jcapimin.o jcapistd.o jctrans.o jcparam.o jdatadst.o jcinit.o \
        jcmaster.o jcmarker.o jcmainct.o jcprepct.o jccoefct.o jccolor.o \
        jcsample.o jchuff.o jcphuff.o jcdctmgr.o jfdctfst.o jfdctflt.o \
        jfdctint.o

# decompression library object files
DLIBOBJECTS= jdapimin.o jdapistd.o jdtrans.o jdatasrc.o jdmaster.o \
        jdinput.o jdmarker.o jdhuff.o jdphuff.o jdmainct.o jdcoefct.o \
        jdpostct.o jddctmgr.o jidctfst.o jidctflt.o jidctint.o jidctred.o \
        jdsample.o jdcolor.o jquant1.o jquant2.o jdmerge.o

# These objectfiles are included in libjpeg
LIBOBJECTS= $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS)

# object files for sample applications (excluding library files)
COBJECTS= cjpeg.o rdppm.o rdgif.o rdtarga.o rdrle.o rdbmp.o rdswitch.o \
        cdjpeg.o
DOBJECTS= djpeg.o wrppm.o wrgif.o wrtarga.o wrrle.o wrbmp.o rdcolmap.o \
        cdjpeg.o wrsprite.o
TROBJECTS= jpegtran.o rdswitch.o cdjpeg.o transupp.o

# object files for sample applications (excluding library files)
#TOBJECTS= jpegtran.o wrppm.o wrgif.o wrtarga.o wrrle.o wrbmp.o rdcolmap.o \
#        cdjpeg.o wrsprite.o  rdswitch.o transupp.o \
#        jcapimin.o jctrans.o jdatadst.o jcparam.o jcapistd.o \
#        jcmarker.o jcmaster.o jchuff.o jcinit.o jccolor.o jcsample.o \
#        jcprepct.o jcdctmgr.o jccoefct.o jfdctint.o jfdctfst.o jcmainct.o

all: libjpeg cjpeg djpeg jpegtran rdjpgcom wrjpgcom

libjpeg: $(LIBOBJECTS)
#	$(RM) libjpeg
	$(AR)  -o libjpeg  $(LIBOBJECTS)

cjpeg: $(COBJECTS) libjpeg
	$(LN) $(LDFLAGS) -o cjpeg $(COBJECTS) libjpeg $(LDLIBS)

djpeg: $(DOBJECTS) libjpeg
	$(LN) -o djpeg $(DOBJECTS) libjpeg $(LDLIBS)

jpegtran: $(TROBJECTS) libjpeg
	$(LN) $(LDFLAGS) -o jpegtran $(TROBJECTS) libjpeg $(LDLIBS)

rdjpgcom: rdjpgcom.o
	$(LN) $(LDFLAGS) -o rdjpgcom rdjpgcom.o $(LDLIBS)

wrjpgcom: wrjpgcom.o
	$(LN) $(LDFLAGS) -o wrjpgcom wrjpgcom.o $(LDLIBS)

clean:
	$(RM) o.* libjpeg cjpeg djpeg jpegtran rdjpgcom wrjpgcom
