#### input / output
LIBOUT = libxsp.so
SRCS= xsp.c
OBJS= $(shell echo $(SRCS) | sed "s/\.cpp/\.o/" | sed "s/\.c/\.o/")

#### command
CC= gcc
LD= gcc

# flags
CFLAGS= -c -fPIC
LFLAGS= -shared -Wl,-soname,$(LIBOUT)

#### rules
all: $(SRCS) $(LIBOUT) 

.cpp.o .c.o:
	$(CC) $(CFLAGS) -o $@ -c $?

$(LIBOUT): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o $(LIBOUT)

clean:	
	rm $(OBJS) $(LIBOUT) 

install:
	cp libxsp.so /usr/lib/libxsp.so
