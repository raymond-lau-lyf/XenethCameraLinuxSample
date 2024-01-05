BINNAME= xeneth-demo

CFLAGS= -I/usr/share/xeneth/Include -I/usr/local/include/wx-2.8-x22 -I/usr/local/lib/wx/include/gtk2-ansi-release-2.8/
CFLAGS+= -DGTK_NO_CHECK_CASTS -D__WXGTK__ -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -DNO_GCC_PRAGMA -fabi-version=11

LNKFLAGS= -lxeneth -lwx_gtk2_xrc-2.8-x22 -lwx_gtk2_qa-2.8-x22 -lwx_gtk2_html-2.8-x22 \
	-lwx_gtk2_adv-2.8-x22 -lwx_gtk2_core-2.8-x22 -lwx_base_xml-2.8-x22 \
	-lwx_base_net-2.8-x22 -lwx_base-2.8-x22 

LNKFLAGS+= -ldl -lm -lstdc++ -lusb-1.0

# Sources
SRCS= app_main.cpp frm_main.cpp dlg_camera.cpp
OBJS= $(shell echo $(SRCS) | sed "s/\.cpp/.o/g" |sed "s/\.c/.o/g")

all: $(SRCS) $(BINNAME)

$(BINNAME): $(OBJS)
	gcc $(OBJS) -o $(BINNAME) $(LNKFLAGS)
	ln $(BINNAME) main

.c.o:
	gcc $(CFLAGS) -o $@ -c $?

.cpp.o:
	g++ $(CFLAGS) -o $@ -c $?

clean:
	rm -f $(OBJS) $(BINNAME) main

