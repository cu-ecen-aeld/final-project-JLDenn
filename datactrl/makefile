#I (Justin Denning) wrote most of this file for ECEN5813 last semester, but modified it to fit 
#this assignment (assignment 2) for ECEN5713 spring 2025

EXEC     = datactrl
CC       = $(CROSS_COMPILE)gcc

CFLAGS   = -Wall -Werror -std=c11

LDFLAGS  = 

SRC      = $(wildcard *.c)
OBJ      = $(SRC:.c=.o)



all: $(EXEC)

local: CFLAGS += -DLOCAL
local: $(EXEC)


${EXEC}: $(OBJ)
		$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
		$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean local

clean:
		@rm -rf *.o ${EXEC}

