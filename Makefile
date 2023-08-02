# Compiler
CC := gcc

# Compiler flags
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2` -lm

# Math library flag
MATH_LIB := -lm

# SDL library flags
SDL_FLAGS := -lSDL2

# SDL library flags including math library
# SDL_FLAGS := `pkg-config --libs sdl2` -lm


# Source files
SRCS := main.c

# Output binary name
TARGET := text_editor

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o textEditor main.c la.c $(LIBS)

clean:
	rm -f $(TARGET)

