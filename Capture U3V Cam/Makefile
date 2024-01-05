BINNAME= capture_u3v_cam

CFLAGS= -I/usr/share/xeneth/Include 
LNKFLAGS= -lstdc++
LNKFLAGS+= -lxeneth  -lusb-1.0

# Sources
SRCS= capture_u3v_cam.cpp
OBJS= $(shell echo $(SRCS) | sed "s/\.cpp/.o/g" |sed "s/\.c/.o/g")

all: $(SRCS) $(BINNAME)

$(BINNAME): $(OBJS)
	gcc $(OBJS) -o $(BINNAME) $(LNKFLAGS)

.c.o:
	gcc $(CFLAGS) -o $@ -c $?

.cpp.o:
	g++ $(CFLAGS) -o $@ -c $?

clean:
	rm -f $(OBJS) $(BINNAME) 

