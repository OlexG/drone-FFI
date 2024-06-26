CC = gcc
CFLAGS = -Iinclude
CRYPTO_SRC = $(wildcard crypto/*.c)
SRC = $(CRYPTO_SRC) generate.c
EXCLUDE_FILES = src/main.c src/stick.c src/wifi.c src/drone.c
SRC := $(filter-out $(EXCLUDE_FILES), $(SRC))
OBJ = $(SRC:.c=.o)
TARGET = output_executable

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

debug:
	@echo "Source files: $(SRC)"
