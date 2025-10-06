CC = gcc
CFLAGS = -Wall -g
TARGET = resolver.exe
SRC = resolver.c
LIBS = -lws2_32

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	del $(TARGET)