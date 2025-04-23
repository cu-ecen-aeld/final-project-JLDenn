
EXEC     = btctl
CC       = $(CROSS_COMPILE)gcc

CFLAGS   = -Wall -Werror -std=gnu11	-Ibluez

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
		@rm -rf *.o bluez/lib/*.o ${EXEC}

