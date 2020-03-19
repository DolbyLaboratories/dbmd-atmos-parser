#-*-makefile-*-

EXECUTABLE = dbmd_atmos_parse
SRCDIR = ../../src
OUTDIR = ./bin
DIR = $(OUTDIR)
objects = $(OUTDIR)/main.o $(OUTDIR)/dbmd_atmos_parse.o 
CC = gcc
CFLAGS = -c -D_FILE_OFFSET_BITS=64
LD = $(CC)
LDFLAGS = 

cleanbuild: all
		@echo Cleaning object files
		rm -rf $(OUTDIR)/*.o
		@echo Build of $(EXECUTABLE) successfully completed,

all: $(DIR) $(OUTDIR)/$(EXECUTABLE)

$(OUTDIR)/$(EXECUTABLE) : $(objects) 
		@echo Linking binary into $(EXECUTABLE) at $(OUTDIR)
		$(CC) $(LDFLAGS) $(objects) -o $(OUTDIR)/$(EXECUTABLE)

$(OUTDIR)/main.o : $(SRCDIR)/main.c $(SRCDIR)/dbmd_atmos_parse.h $(SRCDIR)/dbmd_text.h
		@echo Compiling main.c
		$(CC) $(CFLAGS) $(SRCDIR)/main.c -o $(OUTDIR)/main.o 

$(OUTDIR)/dbmd_atmos_parse.o : $(SRCDIR)/dbmd_atmos_parse.c $(SRCDIR)/dbmd_atmos_parse.h 
		@echo Compiling dbmd_atmos_parse.c
		$(CC) $(CFLAGS) $(SRCDIR)/dbmd_atmos_parse.c -o $(OUTDIR)/dbmd_atmos_parse.o 

$(DIR):
		@echo Creating build path $(OUTDIR)
		@$(SHELL) -ec 'mkdir -p $(OUTDIR)'

