CC = gcc
CFLAGS = -Wall
LDFLAGS = -lm

# Source file and executable name
SRC = server.c
EXE = resolver

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)

clean:
	rm -f $(EXE)

.PHONY: all clean
