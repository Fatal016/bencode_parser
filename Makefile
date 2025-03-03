CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -O3 -g

CC_SOURCES = \
Core/Src/bencode.c \
Core/Src/main.c

HEADERS = \
Core/Inc/bencode.h \
Core/Inc/main.h

EXECUTABLE = bencode.bin

OBJECTS = $(CC_SOURCES:.c=.o)

all: $(EXECUTABLE)
	rm -f $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ -lssl -lcrypto -lcurl

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
