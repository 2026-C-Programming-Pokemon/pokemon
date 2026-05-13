CC      = cc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -Illm
LDFLAGS =
LDLIBS  = -lcurl

SRC     = pokemon.c llm/llm.c
OBJ     = $(SRC:.c=.o)
BIN     = pokemon

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	./$(BIN)
