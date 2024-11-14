CC = gcc

TARGET = bash

SRC = bash_parse.c bash_split.c bash_main.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all