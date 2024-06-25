#include "../include/server_interface.hpp"
#include <ncurses.h>
#include <algorithm>
#include <ctime>

bool serverInterface::checkCollision(spaceship& snk) {
  const point& ss = snk.body;
  const char c = snk.ch;

  if (ss.x < 0 || ss.x >= WIDTH || ss.y < 0 || ss.y >= HEIGHT) {
    return true;
  }

  for (auto& sss: spaceships) {
    if (sss.second.body.x == ss.x && sss.second.body.y == ss.y && sss.first != c) {
      sss.second.points++;
      return true;
    }
  }

  for (int y = 0; y < 4; y++) {
    if (ss.y == 1 + y) {
      int x = (ss.x - 1 + effect) % WIDTH;
      if (terrainUp[y][x] != ' ') {
        return true;
      }
    }
  }

  for (int y = 0; y < 3; y++) {
    if (ss.y == 1 + y + HEIGHT - 4) {
      int x = (ss.x - 1 + effect) % WIDTH;
      if (terrainDown[y][x] != ' ') {
        return true;
      }
    }
  }

  return false;
}

void drawBorder(int h, int w, int x = 0, int y = 0) {
  for (int i = 0; i < w + 2; ++i) {
    mvprintw(0 + x, i + y, "x");
    mvprintw(h + 1 + x, i + y, "x");
  }
  for (int i = 0; i < h + 2; ++i) {
    mvprintw(i + x, 0 + y, "x");
    mvprintw(i + x, w + 1 + y, "x");
  }
}

void serverInterface::drawScores() {
  int row = 0;

  mvprintw(1, WIDTH + 9, "LEADERBOARD");
  mvprintw(2, WIDTH + 8, "SSHIP | SCORE");

  std::vector<std::pair<char, int>> spaceship_points;

  for (const auto& ss : spaceships) {
    char spaceship_char = ss.first;
    int points = ss.second.points;
    spaceship_points.emplace_back(spaceship_char, points);
  }

  std::sort(spaceship_points.begin(), spaceship_points.end(),[](const std::pair<char, int>& a, const std::pair<char, int>& b) {
    return b.second < a.second;
  });

  for (const auto& ss : spaceship_points) {
    char spaceship_char = ss.first;
    int points = ss.second;
    mvprintw(row + 3, WIDTH + 10, "%c      %d", spaceship_char, points);
    ++row;
  }
}

void serverInterface::drawSpaceships() {
  for (const auto& ss : spaceships)
  if (!ss.second.game_over)
  mvprintw(ss.second.body.y + 1, ss.second.body.x + 1, "%c", ss.first);
}

void serverInterface::initSpaceship(spaceship& snk, char ch){
  snk.body.x = WIDTH / 2; 
  snk.body.y = HEIGHT / 2;
  snk.ch = ch;
  snk.game_over = false;
  snk.points = 0;
}

serverInterface::serverInterface() : effect(0) {}

serverInterface::~serverInterface() {}

void serverInterface::updateScreen() {
  for (auto& ss: spaceships)
  if (!ss.second.game_over) {
    if (checkCollision(ss.second)) {
      std::lock_guard<std::mutex> lock(clientsMutex);
      ss.second.game_over = true;
      ss.second.body.x = 0, ss.second.body.y = 0;
      ss.second.points--;
    }
    ss.second.body.x--;
  }
}

void serverInterface::startScreen() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  timeout(100);
  initTerrain();
} 

void serverInterface::drawScreen() {
  clear();
  drawBorder(HEIGHT,WIDTH);
  drawBorder(HEIGHT/2,WIDTH/2,0,WIDTH+4);
  drawTerrain();
  drawSpaceships();
  drawScores();
  refresh();
}

board serverInterface::getBoard() {
  board new_board(HEIGHT, std::vector<char>(WIDTH, ' '));

  for (auto& ss : spaceships) {
    if (!ss.second.game_over) {
      int x = ss.second.body.x;
      int y = ss.second.body.y;

      if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
      new_board[y][x] = ss.first;           
    }
  }

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < WIDTH ; x++) {
      if (terrainUp[y][(x + effect) % WIDTH] != ' ')
      new_board[y][x] = terrainUp[y][(x + effect) % WIDTH];
    }
  }
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < WIDTH ; x++) {
      if (terrainDown[y][(x + effect) % WIDTH] != ' ')
      new_board[y + HEIGHT - 3][x] = terrainDown[y][(x + effect) % WIDTH];
    }
  }
  return new_board;
}

void serverInterface::initTerrain() {
  terrainUp = {
    {'\\',' ',' ',' ',' ',' ',' ','/','\\',' ',' ',' ','/','v','\\',' ',' ',' ',' ','/','\\',' ',' ',' ',' ',' ',' ','/','\\',' ',' ',' ','/','v','\\',' ',' ',' ',' ','/'},
    {' ','\\',' ',' ',' ',' ','/',' ',' ','\\',' ','/',' ',' ',' ','\\',' ',' ','/',' ',' ','\\',' ',' ',' ',' ','/',' ',' ','\\',' ','/',' ',' ',' ','\\',' ',' ','/',' '},
    {' ',' ','\\',' ',' ','/',' ',' ',' ',' ','v',' ',' ',' ',' ',' ','\\','/',' ',' ',' ',' ','\\',' ',' ','/',' ',' ',' ',' ','v',' ',' ',' ',' ',' ','\\','/',' ',' '},
    {' ',' ',' ','\\','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\\','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}};
  terrainDown = {
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '/', '-', '-', '-', '\\', ' ', ' ', ' ', ' ',' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '/', '-', '-', '-', '\\', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', '/', '-', '-', '\\', ' ', ' ', ' ', '/', ' ', ' ', ' ', ' ', ' ', '\\', '-', '\\', ' ',' ', ' ', ' ', '/', '-', '-', '\\', ' ', ' ', ' ', '/', ' ', ' ', ' ', ' ', ' ', '\\', '-', '\\', ' '},
    {'-', '-', '/', ' ', ' ', ' ', ' ', '\\', '-', '/', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\\','-', '-', '/', ' ', ' ', ' ', ' ', '\\', '-', '/', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\\'}};
}

void serverInterface::drawTerrain() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < WIDTH ; x++) {
      int newXUp = (x + effect) % WIDTH;
      if (terrainUp[y][newXUp] != ' ')
      mvprintw(2 + y, 1 + x, "%c", terrainUp[y][newXUp]);
    }
  }
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < WIDTH ; x++) {
      int newXDown = (x + effect) % WIDTH;
      if (terrainDown[y][newXDown] != ' ')
      mvprintw(1 + y + HEIGHT - 3, 1 + x, "%c", terrainDown[y][newXDown]);
    }
  }
  effect++;
}