CC=gcc
CFLAGS= -c -g -Wall -std=c99 `pkg-config --cflags gtk+-3.0` 
LDFLAGS= `pkg-config --libs gtk+-3.0`
DEBUGFLAGS= -g -DDEBUG
SOURCES= src/chip8.c src/platform_common.c src/platform_linux.c
OBJECTS= $(SOURCES:.c=.o)
EXECUTABLE= c8

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

debug:
	$(CC) $(OBJECTS) $(DEBUGFLAGS) -o $(EXECUTABLE) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@
