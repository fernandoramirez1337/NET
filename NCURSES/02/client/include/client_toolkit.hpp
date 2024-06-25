#ifndef CLIENT_TOOLKIT_HPP
#define CLIENT_TOOLKIT_HPP

#include "typedef.hpp"
#include "client_interface.hpp"
#include <cstdint>
#include <string>

class clientToolkit {
public:
  enum fromServerFlag {
    DATA = 'm',
    WIN = 'w',
    LOSE = 'l',
    YES = 'y',
    NO = 'n',
  };
  enum toServerFlag {
    data = 'M',
    init = 'I',
    reset = 'R',
  };
  enum directions {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3,
  };

  void handleData(const std::string&);
  void handleWin(const std::string&);
  void handleLose(const std::string&);
  void handleYes();
  void handleNo();

  std::string generateData(const int, const char);
  std::string generateInit(const char);
  std::string generateReset();

  int unpad(std::string&);
  int pad(std::string&);

  int startUdp(const std::string&, const uint16_t, int&, struct sockaddr_in&);
  clientInterface interface;
};

#endif // CLIENT_TOOLKIT_HPP
