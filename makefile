# make makefile
# 
# Tools used:
#  Compile::Resource Compiler
#  Compile::GNU C
#  Make: make

CFLAGS=-Wall -Zomf -c -O2
DEBUGFLAGS=-g

HEADERS = caoj2i03.h caoj2i04.h csoj2i0a.h csoj2i0h.h csoj2i0i.h csoj2i0j.h \
          csoj2i0k.h csoj2i0l.h cspcsta3.h cspcsta4.h nfm.h nfmlib.h perfutil.h
OBJS = nfm.obj nfmlib.obj csoj2i0a.obj csoj2i0k.obj csoj2i0j.obj csoj2i0h.obj \
       caoj2i03.obj caoj2i04.obj csoj2i0i.obj cspcsta3.obj cspcsta4.obj
LIBS = nfmlib.lib csoj2i0a.lib csoj2i0k.lib csoj2i0j.lib csoj2i0h.lib \
        caoj2i03.lib caoj2i04.lib csoj2i0i.lib csoj2i0l.lib cspcsta4.lib \
        cspcsta3.lib
ALL_IPF = 
INC=-I/@unixroot/usr/include -I/@unixroot/usr/include/os2tk45

all : nfm.exe 

nfm.exe: nfm.obj nfm.res nfm.def $(HEADERS) $(LIBS)
	gcc -Zomf -Zmap -L. $(LIBS) nfm.obj nfm.def nfm.res -o $@
	wrc nfm.res

nfm.obj: nfm.c nfm.h nfm.def $(HEADERS) 
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) nfm.c -o nfm.obj

nfm.res: nfm.rc nfm.h
	wrc -r nfm.rc

#
nfmlib.obj : nfmlib.c nfmlib.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) nfmlib.c -o nfmlib.obj
	
nfmlib.dll : nfmlib.obj nfmlib.def nfmlib.res
	gcc -Zdll -Zomf -Zmap $(INC) nfmlib.obj nfmlib.def -o nfmlib.dll
	wrc nfmlib.res nfmlib.dll

nfmlib.lib : nfmlib.obj nfmlib.def nfmlib.dll
       $(shell emximp -o nfmlib.lib nfmlib.def)

nfmlib.res: nfmlib.rc nfmlib.h
	wrc -r nfmlib.rc
#
csoj2i0a.obj : csoj2i0a.c csoj2i0a.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) csoj2i0a.c -o csoj2i0a.obj
	
csoj2i0a.dll : csoj2i0a.obj csoj2i0a.def csoj2i0a.res
	gcc -Zdll -Zomf -Zmap -lmmpm2 $(INC) csoj2i0a.obj csoj2i0a.def -o csoj2i0a.dll
	wrc csoj2i0a.res csoj2i0a.dll

csoj2i0a.lib : csoj2i0a.obj csoj2i0a.def csoj2i0a.dll
       $(shell emximp -o csoj2i0a.lib csoj2i0a.def)

csoj2i0a.res: csoj2i0a.rc csoj2i0a.h
	wrc -r csoj2i0a.rc
#
csoj2i0k.obj : csoj2i0k.c csoj2i0k.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) csoj2i0k.c -o csoj2i0k.obj
	
csoj2i0k.dll : csoj2i0k.obj csoj2i0k.def
	gcc -Zdll -Zomf -Zmap $(INC) csoj2i0k.obj csoj2i0k.def -o csoj2i0k.dll

csoj2i0k.lib : csoj2i0k.obj csoj2i0k.def csoj2i0k.dll
       $(shell emximp -o csoj2i0k.lib csoj2i0k.def)

#
csoj2i0j.obj : csoj2i0j.c csoj2i0j.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) csoj2i0j.c -o csoj2i0j.obj
	
csoj2i0j.dll : csoj2i0j.obj csoj2i0j.def
	gcc -Zdll -Zomf -Zmap $(INC) csoj2i0j.obj csoj2i0j.def -o csoj2i0j.dll

csoj2i0j.lib : csoj2i0j.obj csoj2i0j.def csoj2i0j.dll
       $(shell emximp -o csoj2i0j.lib csoj2i0j.def)

csoj2i0j.res: csoj2i0j.rc csoj2i0j.h
	wrc -r csoj2i0j.rc

#
csoj2i0h.obj : csoj2i0h.c csoj2i0h.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) csoj2i0h.c -o csoj2i0h.obj
	
csoj2i0h.dll : csoj2i0h.obj csoj2i0h.def
	gcc -Zdll -Zomf -Zmap $(INC) csoj2i0h.obj csoj2i0h.def -o csoj2i0h.dll

csoj2i0h.lib : csoj2i0h.obj csoj2i0h.def csoj2i0h.dll
       $(shell emximp -o csoj2i0h.lib csoj2i0h.def)

csoj2i0h.res: csoj2i0h.rc csoj2i0h.h
	wrc -r csoj2i0h.rc

#
caoj2i03.obj : caoj2i03.c caoj2i03.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) caoj2i03.c -o caoj2i03.obj
	
caoj2i03.dll : caoj2i03.obj caoj2i03.def
	gcc -Zdll -Zomf -Zmap $(INC) caoj2i03.obj caoj2i03.def -o caoj2i03.dll

caoj2i03.lib : caoj2i03.obj caoj2i03.def caoj2i03.dll
       $(shell emximp -o caoj2i03.lib caoj2i03.def)

#
caoj2i04.obj : caoj2i04.c caoj2i04.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) caoj2i04.c -o caoj2i04.obj
	
caoj2i04.dll : caoj2i04.obj caoj2i04.def
	gcc -Zdll -Zomf -Zmap $(INC) caoj2i04.obj caoj2i04.def -o caoj2i04.dll

caoj2i04.lib : caoj2i04.obj caoj2i04.def caoj2i04.dll
       $(shell emximp -o caoj2i04.lib caoj2i04.def)

#
csoj2i0i.obj : csoj2i0i.c csoj2i0i.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) csoj2i0i.c -o csoj2i0i.obj
	
csoj2i0i.dll : csoj2i0i.obj csoj2i0i.def
	gcc -Zdll -Zomf -Zmap $(INC) csoj2i0i.obj csoj2i0i.def -o csoj2i0i.dll

csoj2i0i.lib : csoj2i0i.obj csoj2i0i.def csoj2i0i.dll
       $(shell emximp -o csoj2i0i.lib csoj2i0i.def)

#
csoj2i0l.obj : csoj2i0l.c csoj2i0l.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) csoj2i0l.c -o csoj2i0l.obj
	
csoj2i0l.dll : csoj2i0l.obj csoj2i0l.def
	gcc -Zdll -Zomf -Zmap $(INC) csoj2i0l.obj csoj2i0l.def -o csoj2i0l.dll

csoj2i0l.lib : csoj2i0l.obj csoj2i0l.def csoj2i0l.dll
       $(shell emximp -o csoj2i0l.lib csoj2i0l.def)

#
cspcsta4.obj : cspcsta4.c cspcsta4.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) cspcsta4.c -o cspcsta4.obj
	
cspcsta4.dll : cspcsta4.obj cspcsta4.def
	gcc -Zdll -Zomf -Zmap $(INC) cspcsta4.obj cspcsta4.def -o cspcsta4.dll

cspcsta4.lib : cspcsta4.obj cspcsta4.def cspcsta4.dll
       $(shell emximp -o cspcsta4.lib cspcsta4.def)

cspcsta4.res: cspcsta4.rc cspcsta4.h
	wrc -r cspcsta4.rc

#
cspcsta3.obj : cspcsta3.c cspcsta3.h
	gcc $(CFLAGS) $(DEBUGFLAGS) $(INC) cspcsta3.c -o cspcsta3.obj
	
cspcsta3.dll : cspcsta3.obj cspcsta3.def
	gcc -Zdll -Zomf -Zmap $(INC) cspcsta3.obj cspcsta3.def -o cspcsta3.dll

cspcsta3.lib : cspcsta3.obj cspcsta3.def cspcsta3.dll
       $(shell emximp -o cspcsta3.lib cspcsta3.def)


.PHONY : clean

clean :
	rm -rf *exe *res *obj *lib *.hlp *map *dll

# MAKEFILE NOTES
#
# $@ is the name of the target being generated.
#
# .PHONY - Read about .PHONY at: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
#
