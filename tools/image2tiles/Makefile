CC ?= gcc

CFLAGS = -Wall -Wextra -Werror -Wno-sign-compare -std=c11 -O2 -DPNG_SKIP_SETJMP_CHECK
CFLAGS += $(shell pkg-config --cflags libpng)

LIBS = -lpng -lz
LDFLAGS += $(shell pkg-config --libs-only-L libpng)

SRCS = deduplicateTiles.c main.c options.c ../gbagfx/convert_png.c ../gbagfx/gfx.c ../gbagfx/util.c
HEADERS = deduplicateTiles.h options.h ../gbagfx/convert_png.h ../gbagfx/global.h ../gbagfx/gfx.h ../gbagfx/util.h

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

.PHONY: all clean

all: image2tiles$(EXE)
	@:

image2tiles-debug$(EXE): Makefile $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -DDEBUG -ggdb $(SRCS) -o $@ $(LDFLAGS) $(LIBS)

image2tiles$(EXE): Makefile $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS) $(LIBS)

clean:
	$(RM) image2tiles image2tiles.exe
