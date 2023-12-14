#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <chrono>
#include <iostream>

#include "board.h"
#include "common.h"
#include "clientSocket.h"

bool quit = false, muted = true, start = false, isTyping = false;
std::string inputText = "";
using namespace std;

int main(int argc, char* args[]) {
    bool typingTrigger = false;
    int mF = -1, mT = -1;
    SDL_Event e;  // Event handler
    ClientSocket clientSocket = ClientSocket("127.0.0.1", 5500);
    Board board = Board(&clientSocket);

    std::cout << "Current FEN (start): " << board.getFEN() << '\n';
    std::cout << "Current Zobrist (start): " << board.getZobrist() << '\n';
    //	board.eval(true);

    /*
            typedef std::chrono::duration<float> fsec;
            fsec diff;
            int nodes;
            std::cout << "Perftesting....";
            for (int i = 1; i <= 6; i++) {
                    auto beginTime = std::chrono::high_resolution_clock::now();
                    nodes = board.perft(i);
                    std::cout << "board.perft(" << i << "): " << nodes << "\n";
                    auto endTime = std::chrono::high_resolution_clock::now();
                    diff = endTime - beginTime;
                    std::cout << "Took: " << diff.count() << " seconds" << '\n';
                    std::cout << "Nodes/sec: " << nodes/diff.count() << '\n';
            }
    */

    while (!quit) {
        clientSocket.handleBufferRead();
        if (isTyping){
            if (!typingTrigger){
			    SDL_StartTextInput();
                typingTrigger = isTyping;
            }
        }
        else
            if (typingTrigger){
			    SDL_StopTextInput();
                typingTrigger = isTyping;
            }

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (start) {
                            mF = mT = -1;
                            board.updateDisplay(mF, mT);
                            board.botMove();
                        } else if (board.display.getMenu() == Display::Menu::offlineMenu)
                            board.startGame();
                        break;
                    case SDLK_m:
                        if (!isTyping){
                            std::cout << "\nm\n"; 
                            muted = muted ? 0 : 1;
                        }
                        break;
                    case SDLK_w:
                        if (!isTyping){
                            board.setWhiteIsBot(!board.getWhiteIsBot());
                        }
                        break;
                    case SDLK_b:
                        if (!isTyping){
                            board.setBlackIsBot(!board.getBlackIsBot());
                        }
                        break;
                    case SDLK_UP:
                        if (board.whiteBot.getLevel() < 9 &&
                            board.blackBot.getLevel() < 9) {
                            board.whiteBot.setLevel(board.whiteBot.getLevel() +
                                                    1);
                            board.blackBot.setLevel(board.blackBot.getLevel() +
                                                    1);
                        }
                        break;
                    case SDLK_DOWN:
                        if (board.whiteBot.getLevel() > 1 &&
                            board.blackBot.getLevel() > 1) {
                            board.whiteBot.setLevel(board.whiteBot.getLevel() -
                                                    1);
                            board.blackBot.setLevel(board.blackBot.getLevel() -
                                                    1);
                        }
                        break;
                    case SDLK_LEFT:
                        board.undoMove();
                        board.updateDisplay(mF, mT);
                        break;
                    case SDLK_BACKSPACE:
                        if (!isTyping){
                            board.restart();
                            board.updateDisplay(mF, mT);
                        }
                        else{
                            //Handle backspace
                            if (inputText.length() > 0){
                                //lop off character
                                inputText.pop_back();
                                //renderText = true;
                            }
                        }
                        break;
                    case SDLK_c:
                        //Handle copy
                        if (isTyping && SDL_GetModState() & KMOD_CTRL){
                             SDL_SetClipboardText( inputText.c_str() );
                            //renderText = true;
                        } 
                    case SDLK_v:
                    //Handle paste
                        if (isTyping && SDL_GetModState() & KMOD_CTRL){
                            inputText = SDL_GetClipboardText();
                            //renderText = true;
                        }
                }
            }
            else if (e.type == SDL_TEXTINPUT) {
                //Not copy or pasting
                if(isTyping && !( SDL_GetModState() & KMOD_CTRL && ( e.text.text[ 0 ] == 'c' || e.text.text[ 0 ] == 'C' || e.text.text[ 0 ] == 'v' || e.text.text[ 0 ] == 'V' ) ) )
                {
                  //Append character
                  inputText += e.text.text;
                  //renderText = true;
                }
            }
            
            board.handleInput(mF, mT, &e);
        }
            
        board.updateDisplay(mF, mT);

        if (start) {
          if (board.getGamemode() == Board::gamemode::offline){
            if ((board.getSide() && board.getWhiteIsBot()) ||
                (!board.getSide() && board.getBlackIsBot())) {
                board.botMove();
                board.updateDisplay(mF, mT);
            }
          }
          else{
            if (board.getSide() ^ board.getPlayerSide()) 
            {
                //Get input from network
                //Stop taking move input
                mF = mT = -1;
            }
          }
        } else
            mF = mT = -1;
    }

    return 0;
}