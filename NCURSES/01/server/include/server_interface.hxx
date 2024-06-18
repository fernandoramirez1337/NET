#ifndef SERVER_INTERFACE_HXX
#define SERVER_INTERFACE_HXX

#include "typedef.hxx"
#include <string>
#include <map>

class server_interface{
public:
  server_interface();
  ~server_interface();
  void update_screen();
  void start_screen();
  void draw_screen();

  void init_snake(snake&,char);
  std::map<char,snake> snakes;
  void draw_snakes();
  void draw_scores();
  bool check_collision(snake& );
  board draw_board();
};

#endif // SERVER_INTERFACE_HXX