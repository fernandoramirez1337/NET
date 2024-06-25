#ifndef SERVER_TOOLKIT_HPP
#define SERVER_TOOLKIT_HPP

#include <typedef.hpp>
#include <string>

class serverToolkit {
public:
  enum toClientFlag {
    DATA = 'm',
    WIN = 'w',
    LOSE = 'l',
    YES = 'y',
    NO = 'n',
  };
  enum fromClientFlag {
    data = 'M',
    init = 'I',
    reset = 'R',
  };
  enum directions {
    UP = '0',
    RIGHT = '1',
    DOWN = '2',
    LEFT = '3',
  };

  std::string generateData(board&);
  std::string generateWin(const char);
  std::string generateLose(const char);
  std::string generateYes();
  std::string generateNo();

  int pad(std::string&);
  int unpad(std::string&);
};

#endif // SERVER_TOOLKIT_HXX