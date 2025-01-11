CC = gcc
CFLAGS = -Wall
TARGET = server
OBJS = main.o lso.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c lso.h
	$(CC) $(CFLAGS) -c main.c

lso.o: lso.c lso.h strutturedati.h
	$(CC) $(CFLAGS) -c lso.c

clean:
	rm -f $(OBJS) $(TARGET)
