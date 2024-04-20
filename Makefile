CC = gcc

LIBS := -I./include -lshell32 -lXInput -lopengl32 -lgdi32 -lm -ggdb -lShcore -lWinmm
EXT = .exe

ifeq ($(CC),x86_64-w64-mingw32-gcc)
	STATIC = --static
endif

ifneq (,$(filter $(CC),winegcc x86_64-w64-mingw32-gcc))
    detected_OS := Windows
	LIB_EXT = .dll
else
	ifeq '$(findstring ;,$(PATH))' ';'
		detected_OS := Windows
	else
		detected_OS := $(shell uname 2>/dev/null || echo Unknown)
		detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
		detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
		detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
	endif
endif

ifeq ($(detected_OS),Windows)
	LIBS := -I./include -lshell32 -lXInput -lopengl32 -lgdi32 -lm -ggdb -lShcore -lWinmm
	EXT = .exe
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -I./include -framework AudioToolbox -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo -w $(STATIC)
	EXT = 
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lXrandr -lX11 -lGL -lm $(STATIC)
	EXT = 
endif

all:
	make RSGL.o
	$(CC) source/main.c RSGL.o  -O3 $(LIBS) -I./ -Wall -o RSGLDoom$(EXT)

RSGL.o:
	gcc -c source/RSGL.c -I./include

clean:
	rm -f RSGLDoom RSGLDoom$(EXTT)

debug:
	make clean
	make RSGL.o

	$(CC) source/main.c -w RSGL.o $(LIBS) -I./ -Wall -D RSGL_DEBUG -o RSGLDoom
	./RSGLDoom$(EXT)