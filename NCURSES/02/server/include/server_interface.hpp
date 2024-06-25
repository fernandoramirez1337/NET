#ifndef SERVER_INTERFACE_HPP
#define SERVER_INTERFACE_HPP

#include "typedef.hpp"
#include <mutex>  // mutex
#include <map>    // map

class serverInterface{
public:
  int effect;
  serverInterface();
  ~serverInterface();

  void drawSpaceships();
  void updateScreen();
  void startScreen();
  void drawScreen();
  void drawScores();
  void drawTerrain();
  void initTerrain();
  board getBoard();

  void initSpaceship(spaceship&,char);
  std::map<char,spaceship> spaceships;
  bool checkCollision(spaceship&);
  std::mutex clientsMutex;
  board terrainUp, terrainDown;

};

#endif // SERVER_INTERFACE_HPP