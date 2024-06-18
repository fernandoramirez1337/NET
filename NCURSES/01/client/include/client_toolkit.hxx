#ifndef CLIENT_TOOLKIT_HXX
#define CLIENT_TOOLKIT_HXX

#include <cstdint>
#include <string>

#ifndef MAXLINE
  #define MAXLINE 1024
#endif
#ifndef PAD
  #define PAD 46
#endif

class client_toolkit {
public:
  enum from_server_flag {
    DATA = 'm',
    WIN = 'w',
    LOSE = 'l',
    YES = 'y',
    NO = 'n',
  };
  enum to_server_flag {
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

  void handle_data(const std::string&);
  void handle_win(const std::string&);
  void handle_lose(const std::string&);
  void handle_yes();
  void handle_no();

  std::string generate_data(const int, const char);
  std::string generate_init(const char);
  std::string generate_reset();

  int pad_(std::string&);
  int unpad_(std::string&);
};

#endif // CLIENT_TOOLKIT_HXX
