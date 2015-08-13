
PROJECT=dac

SOURCES= \
	main.c

CFLAGS= \
	-c \
	-Wall \
	-Wextra \
	-Wstrict-prototypes \
	-Wunused-result \
	-O3 \
	-std=c99 \
	-fmessage-length=0

LDFLAGS= 

LDLIBS= \
	-lmcp2221

EXECUTABLE=$(PROJECT)

CC=gcc
OBJECTS=$(SOURCES:.c=.o)


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

.PHONY: clean all
