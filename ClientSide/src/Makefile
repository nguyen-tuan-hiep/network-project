IDIR=../include
LDIR=../lib
CC=g++
CFLAGS=-g -std=c++11 -c -I$(IDIR)
LFLAGS=-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -L$(LDIR) -lws2_32
SOURCES=main.cpp display.cpp board.cpp move.cpp movegen.cpp legal.cpp ltexture.cpp square.cpp button.cpp piece.cpp eval.cpp bot.cpp zobrist.cpp bitboards.cpp clientSocket.cpp inputField.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=client 

all: $(SOURCES) $(EXECUTABLE) 

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	del /Q *.o *.exe $(EXECUTABLE)
