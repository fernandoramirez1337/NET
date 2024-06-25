#ifndef TYPEDEF_HPP
#define TYPEDEF_HPP

#include <vector>

#define MAXLINE 1024
#define PAD 46
#define WIDTH 40
#define HEIGHT 20

using board = std::vector<std::vector<char>>;

struct point {
  int x, y;
};

struct spaceship {
  point body;
  bool game_over;
  int points;
  char ch;
};

#endif // TYPEDEF_HPP