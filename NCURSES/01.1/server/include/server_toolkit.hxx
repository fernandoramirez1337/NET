#ifndef SERVER_TOOLKIT_HXX
#define SERVER_TOOLKIT_HXX

#include <typedef.hxx>
#include <string>

class server_toolkit {
public:
  enum to_client_flag {
    DATA = 'm',
    WIN = 'w',
    LOSE = 'l',
    YES = 'y',
    NO = 'n',
  };
  enum from_client_flag {
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

  std::string generate_data(board&);
  std::string generate_win(const char);
  std::string generate_lose(const char);
  std::string generate_yes();
  std::string generate_no();

  int pad_(std::string&);
  int unpad_(std::string&);
};

#endif // SERVER_TOOLKIT_HXX