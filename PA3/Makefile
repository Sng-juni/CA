CC=gcc
CCFLAGS=

#add C source files here
SRCS=main.c

TARGET=mips-sim

OBJS := $(patsubst %.c,%.o,$(SRCS))

all: $(TARGET)

%.o:%.C
			$(CC) $(CCFLAGS) $< -c -o $@

$(TARGET): $(OBJS)
		   $(CC) $(CCFLAGS) $^ -o  $@ -lm

.PHONY=clean

clean:
			rm -f $(OBJS) $(TARGET)