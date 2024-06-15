CC = gcc
CFLAGS = -Wall
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)
TARGET = logos

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) -lm

clean:
	rm -f $(TARGET)
