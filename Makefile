# Makefile - Simulador TLI (Situacion-Problema 3)
# Uso:  make          -> compila
#       make run      -> compila y ejecuta
#       make debug    -> compila con -g para depurar con gdb
#       make clean    -> borra objetos y ejecutable

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic -O2
LDLIBS  = -lm
TARGET  = orion_tli
OBJS    = main.o artemis.o incisos.o mision.o validaciones.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

main.o: main.c artemis.h validaciones.h
artemis.o: artemis.c artemis.h
incisos.o: incisos.c artemis.h validaciones.h
mision.o: mision.c artemis.h validaciones.h
validaciones.o: validaciones.c validaciones.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -O0
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: all debug run clean
