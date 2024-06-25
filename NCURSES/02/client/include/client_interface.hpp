#ifndef CLIENT_INTERFACE_HPP
#define CLIENT_INTERFACE_HPP

#include <string>

class clientInterface {
public:
  void drawBorder(int h, int w, int x = 0, int y = 0);
  void drawChar(int x, int y, char c);
  void startScreen();
  void clearScreen();
  void refreshScreen();
  void printText(std::string&);
  void printText(const char *);

  int getChar();
};

#endif