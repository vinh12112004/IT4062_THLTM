CC = gcc
CFLAGS = -Wall -g
TARGET = hw1
SRCS = hw1.c account.c history.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET).exe $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /f $(OBJS) $(TARGET).exe
