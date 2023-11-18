#define SDL_MAIN_HANDLED

#include <chrono>
#include <iostream>

#include "clientSocket.h"

bool quit = false, muted = true, start = false, isTyping = false;
std::string inputText = "";
using namespace std;

int main(int argc, char* args[]) {
    bool typingTrigger = false;
    int mF = -1, mT = -1;
    SDL_Event e;  // Event handler
    ClientSocket clientSocket = ClientSocket("127.0.0.1", 5500);
}