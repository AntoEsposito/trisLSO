CC = gcc
CFLAGS = -Wall -Wextra -O3
TARGET = server
OBJS = main.o funzioni.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c funzioni.h
	$(CC) $(CFLAGS) -c main.c

funzioni.o: funzioni.c funzioni.h strutturedati.h
	$(CC) $(CFLAGS) -c funzioni.c

clean:
	rm -f $(OBJS) $(TARGET)
