CC = gcc
CFLAGS = -Wall -g

SRCS = test_main.c memory_manager.c bitmap.c
OBJS = $(SRCS:.c=.o)
TARGET = memory_manager_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
