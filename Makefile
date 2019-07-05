# lasersight metamod plugin Makefile for linux and win32 (mingw)

LFLAGS = -shared -static
ARCH = $(shell uname -m)
LS_COMMIT = $(firstword $(shell git rev-parse --short=6 HEAD) unknown)
CFLAGS =  -O1 -DVVERSION=\"$(LS_COMMIT)\" -fexpensive-optimizations -ffast-math -fpermissive \
	  -funroll-loops -fomit-frame-pointer -Wall -fno-exceptions -fno-rtti -fPIC \
	  -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -Dstrcmpi=strcasecmp

# force i686 postfix on x86_64 architecture
ifeq ($(ARCH), x86_64)
	DLLNAME = lasersight_mm_i686.so
else
	DLLNAME = lasersight_mm_$(ARCH).so
endif

# architecture depended flags
ifeq ($(ARCH), x86_64)
	LFLAGS += -m32
	CFLAGS += -march=i686 -m32
endif

INCLUDEDIRS = -Isrc/. \
	      -Isrc/metamod \
	      -Isrc/engine \
	      -Isrc/common \
	      -Isrc/pm_shared \
	      -Isrc/dlls \
	      -Isrc/libini

OBJ =	src/meta_api.o		\
        src/plugin.o		\
				\
        src/dllapi.o		\
        src/engine_api.o	\
				\
        src/cplayer.o		\
        src/cmessagehandlers.o	\
				\
        src/lasersight_ini.o	\
        src/libini.o

DOCC = $(CXX) $(CFLAGS) $(INCLUDEDIRS) -o $@ -c $<
DOO = $(CXX) -o $@ $(OBJ) $(LFLAGS)

$(DLLNAME) : $(OBJ)
	$(DOO)

clean:
	rm -f $(OBJ) $(DLLNAME)

./%.o: ./%.cpp
	$(DOCC)
