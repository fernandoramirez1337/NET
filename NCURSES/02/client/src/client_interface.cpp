#include "../include/client_interface.hpp"

#include <ncurses.h>

void clientInterface::drawBorder(int h, int w, int x, int y) {

  for (int i = 0; i < w + 2; ++i) {
    mvprintw(0+x, i+y, "x");
    mvprintw(h + 1+x, i+y, "x");
  }
  for (int i = 0; i < h + 2; ++i) {
    mvprintw(i+x, 0+y, "x");
    mvprintw(i+x, w + 1+y, "x");
  }
}

int clientInterface::getChar() {
  return getch();
}

void clientInterface::drawChar(int x, int y, char c) {
  mvprintw(x, y, "%c", c); 
}

void clientInterface::startScreen() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
}

void clientInterface::clearScreen() { 
  clear();
}

void clientInterface::refreshScreen() {
  refresh();
}

void clientInterface::printText(std::string& message) {
  printw(message.c_str());
}

void clientInterface::printText(const char * message) {
  printw(message);
}