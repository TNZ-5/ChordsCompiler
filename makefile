# Makefile for fb1.l and fb1.y

# Compiler
CC = cc

# Compiler flags
CFLAGS = -Wall

# Bison and Flex commands
BISON = bison
FLEX = flex

# Source files
BISON_SRC = chordProg.y
FLEX_SRC = chordProg.l
OBJS = chordProg.tab.c lex.yy.c

# Executable name
TARGET = chordCompiler

# Default target
all: $(TARGET)

# Rule to generate the lexer and parser
$(OBJS): $(BISON_SRC) $(FLEX_SRC)
	$(BISON) -d $(BISON_SRC)
	$(FLEX) $(FLEX_SRC)

# Rule to compile and link the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ chordProg.tab.c lex.yy.c -lfl

# Clean rule to remove generated files and the executable
clean:
	rm -f $(OBJS) $(TARGET) chordProg.tab.h