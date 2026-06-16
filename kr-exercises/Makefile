CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
CPPFLAGS = -MMD -MP
TARGET = app
OBJS = function_pointer.o math.o
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean
