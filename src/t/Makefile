# Path to the ringbuffer source directory

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -O2
LDFLAGS :=

# Sources (note: ringbuf sources are in VSRC)
SRCS := ../ringbuf.c ringbuf_test.c
OBJS := $(SRCS:.c=.o)

TARGET := test_ringbuf

test: all 
	./test_ringbuf
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Build local .o files for sources
%.o: %.c ringbuf.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
