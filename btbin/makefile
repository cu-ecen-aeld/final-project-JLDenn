#I (Justin Denning) wrote most of this file for ECEN5813 last semester, but modified it to fit 
#this assignment (assignment 2) for ECEN5713 spring 2025

EXEC     = btctl
CC       = $(CROSS_COMPILE)gcc

CFLAGS   = -Wall -Werror -std=gnu11

LDFLAGS  = 

SRC      = $(wildcard *.c) $(wildcard bluez/lib/*.c)
OBJ      = $(SRC:.c=.o)



all: $(EXEC)


${EXEC}: $(OBJ)
		$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
		$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean local

clean:
		@rm -rf *.o ${EXEC}

