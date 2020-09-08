CC ?= gcc
RM ?= rm -f

CFLAGS-y  := -Wall -Wextra -Wshadow -Werror -I. $(CFLAGS)

CFLAGS-$(DEBUG)   += -DDEBUG=1 -ggdb -g3 -O0 -fanalyzer -fsanitize=undefined -fsanitize=thread -DLOG_STDOUT
CFLAGS-$(RELEASE) += -O3 -DNDEBUG=1

LDFLAGS-y := $(LDFLAGS)

TARGET  := jobs
SOURCES := $(TARGET).c main.c
OBJECTS := $(SOURCES:.c=.o)
CFLAGS  := $(CFLAGS-y)
LDFLAGS := $(LDFLAGS-y)

.PHONY: all clean

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $(TARGET)
	$(RM) *.o
	mkdir -p ./bin/
	mv $(TARGET) bin/$(TARGET)

all: build

clean:
	$(RM) $(TARGET) $(OBJECTS)

