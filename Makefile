# FIXME use cmake

# NOTE: This one Makefile works with Microsoft nmake and GNU make.
# They use different conditional syntax, but each can be nested and inverted within the other.

all: check

ifdef MAKEDIR:
!ifdef MAKEDIR

#
# Microsoft nmake on Windows with desktop CLR, Visual C++.
#

CFLAGS=-MD -Gy -Z7 -GX
CPPFLAGS=-MD -Gy -Z7 -GX

RM_F = del 2>nul /f
#ILDASM = ildasm /nobar /out:$@
#ILASM = ilasm /quiet
#RUN_EACH=for %%a in (
#RUN_EACH_END=) do @$Q$(MONO)$Q %%a
O=obj

!else
else

#
# GNU/Posix make on Unix with mono, gcc, clang, etc.
#
RM_F = rm -f

#ILDASM = ikdasm >$@
#ILASM = ilasm
#MONO ?= mono
#RUN_EACH=for a in
#RUN_EACH_END=;do $(MONO) $${a}; done
O=o

endif
!endif :

check:

clean:

exe:

ifdef MAKEDIR:
!ifdef MAKEDIR

!if !exist (./config.mk)
!if [.\config.cmd]
!endif
!endif
!if exist (./config.mk)
!include ./config.mk
!endif

#!message AMD64=$(AMD64)
#!message 386=$(386)

!if !defined (AMD64) && !defined (386) && !defined (ARM)
AMD64=1
386=0
ARM=0
!endif

!if $(AMD64)
win=winamd64.exe
386=0
ARM=0
!elseif $(386)
win=winx86.exe
AMD64=0
ARM=0
!elseif $(ARM)
win=winarm.exe
AMD64=0
386=0
!endif

!ifndef win
win=win.exe
!endif

all: $(win)

config:
	.\config.cmd

check:

run: $(win)
	.\$(win) hello.wasm

debug: $(win)
!if $(AMD64)
	\bin\amd64\cdb .\$(win) hello.wasm
!elseif $(386)
	\bin\x86\cdb .\$(win) hello.wasm
!endif

clean:
	$(RM_F) $(win) w3rt.o w3rt.obj w3.obj *.ilk win32 win32.exe win64 win64.exe win win.exe winarm.exe winx86.exe winamd64.exe *.pdb lin *.i

# TODO clang cross
#
#mac: w3.cpp
#	g++ -g w3.cpp -o $@
#

# TODO /Qspectre

OBJS=   w3Fd.$O \
        w3Handle.$O \
        w3InstructionEncoding.$O \
        w3InstructionNames.$O \
        w3MemoryMappedFile.$O \
        w3Module.$O \
        w3rt.$O \
        w3Unpack.$O \
        isinf.$O \
        isnan.$O \
        s_floor.$O \
        s_floorf.$O \
        s_round.$O \
        s_roundf.$O \
        s_trunc.$O \
        s_truncf.$O \

$(win): w3.cpp $(OBJS)
	@-del $(@R).pdb $(@R).ilk
	@rem TODO /GX on old, /EHsc on new
	rem cl -MD -Gy -Z7 /O2s $(Wall) $(Qspectre) -W4 -GX $** /link /out:$@ /incremental:no /opt:ref,icf
	cl -MD -Gy -Z7 $(Wall) $(Qspectre) -W4 /GX $** /link /out:$@ /incremental:no /opt:ref /pdb:$(@B).pdb

!else
else

UNAME_S = $(shell uname -s)

ifeq ($(UNAME_S), Cygwin)
Cygwin=1
NativeTarget=cyg
else
Cygwin=0
Linux=0
endif

ifeq ($(UNAME_S), Linux)
Linux=1
NativeTarget=lin
else
Cygwin=0
endif

# TODO Darwin, Linux, etc.

# FIXME winarm64 etc.
all: $(NativeTarget) win32.exe win64.exe

run: $(NativeTarget)
	./$(NativeTarget) /s/mono/mcs/class/lib/build-macos/mscorlib.dll

debug: mac
	lldb -- ./$(NativeTarget) /s/mono/mcs/class/lib/build-macos/mscorlib.dll

clean:
	$(RM_F) w3rt.o w3rt.obj mac win32 win32.exe win64 win64.exe win win.exe cyg cyg.exe *.ilk lin win.exe winarm.exe winx86.exe winamd64.exe

OBJS=   w3Fd.$O \
        w3Handle.$O \
        w3InstructionEncoding.$O \
        w3InstructionNames.$O \
        w3MemoryMappedFile.$O \
        w3Module.$O \
        w3rt.$O \

mac: w3.cpp $(OBJS)
	g++ -g w3.cpp $(OBJS) -o $@ -Bsymbolic -bind_at_load

cyg: w3.cpp $(OBJS)
	g++ -g w3.cpp $(OBJS) -o $@ -Bsymbolic -znow -zrelro

lin: w3.cpp $(OBJS)
	g++ -Wall -g w3.cpp $(OBJS) -o $@ -Bsymbolic -znow -zrelro

win32.exe: w3.cpp $(OBJS)
	i686-w64-mingw32-g++ -g w3.cpp $(OBJS) -o $@ -Bsymbolic

win64.exe: w3.cpp $(OBJS)
	x86_64-w64-mingw32-g++ -g w3.cpp $(OBJS) -o $@ -Bsymbolic

test:

endif
!endif :
