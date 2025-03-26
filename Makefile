CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wno-unused-parameter -O3 -g

CC_SOURCES = \
Core/Src/bencode.c \
Core/Src/bencode_utils.c \
Core/Src/main.c

HEADERS = \
Core/Inc/bencode.h \
Core/Inc/bencode_utils.h \
Core/Inc/main.h \
Core/Inc/codes.h

EXECUTABLE = bencode.bin

OBJECTS = $(CC_SOURCES:.c=.o)

all: $(EXECUTABLE)
	rm -f $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ -lssl -lcrypto

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
