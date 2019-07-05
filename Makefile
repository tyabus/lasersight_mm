# lasersight metamod plugin Makefile for linux and win32 (mingw)

# CFLAGS = -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -Dstrcmpi=strcasecmp
LFLAGS = -shared -static
ARCH = $(shell uname -m)
LS_COMMIT = $(firstword $(shell git rev-parse --short=6 HEAD) unknown)
CFLAGS =  -O1 -DVVERSION=\"$(LS_COMMIT)\" -fexpensive-optimizations -ffast-math -fpermissive -fPIC \
	  -funroll-loops -fomit-frame-pointer -Wall -fno-exceptions -fno-rtti \
	  -Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -Dstrcmpi=strcasecmp

# force i686 postfix on x86_64 architecture
ifeq ($(ARCH), x86_64)
	DLLNAME = lasersights_mm_i686.so
else
	DLLNAME = lasersights_mm_$(ARCH).so
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

DOCC = $(CC) $(CFLAGS) $(INCLUDEDIRS) -o $@ -c $<
DOO = $(CC) -o $@ $(OBJ) $(LFLAGS)

$(DLLNAME) : $(OBJ)
	$(DOO)

clean:
	rm -f $(OBJ) $(DLLNAME)

./%.o: ./%.cpp
	$(DOCC)
