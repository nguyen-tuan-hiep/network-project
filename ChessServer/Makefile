
# CC = g++

# all: windows_server 
# windows_server: server_main.cpp
# 	$(CC) server_main.cpp -lws2_32 -o windows_server -g

# run_server: windows_server
# 	windows_server $(ip) $(port)

# clean:
# 	del /Q *.o *.exe

IDIR=../include
CC=g++
CFLAGS=-g -std=c++11 -c -I$(IDIR)
LFLAGS=-lws2_32
SOURCES=main.cpp account.cpp room.cpp ServerSocket.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server 

all: $(SOURCES) $(EXECUTABLE) 

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	del /Q *.o *.exe $(EXECUTABLE)


