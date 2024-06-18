#ifndef TYPEDEF_HXX
#define TYPEDEF_HXX

#include <vector>

#define MAXLINE 1024
#define PAD 46
#define WIDTH 40
#define HEIGHT 20

using board = std::vector<std::vector<char>>;

struct point {
  int x, y;
};

struct snake {
  std::vector<point> body;
  point direction;
  bool game_over;
  int points;
  char ch;
};

#endif // TYPEDEF_HXX