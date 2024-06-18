#include "../include/server_interface.hxx"
#include <ncurses.h>
#include <algorithm>

bool server_interface::check_collision(snake& snk) {
  const point& head = snk.body.front();

  if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT)
  return true;

  for (size_t i = 1; i < snk.body.size(); ++i) 
  if (snk.body[i].x == head.x && snk.body[i].y == head.y) 
  return true;

  for (auto& snakee: snakes)
  for (size_t i = 1; i < snakee.second.body.size(); ++i) 
  if (snakee.second.body[i].x == head.x && snakee.second.body[i].y == head.y) {
    snakee.second.points++;
    return true;
  }
  return false;
}

void draw_border(int h, int w, int x = 0, int y = 0) {
  for (int i = 0; i < w + 2; ++i) {
    mvprintw(0+x, i+y, "x");
    mvprintw(h + 1+x, i+y, "x");
  }
  for (int i = 0; i < h + 2; ++i) {
    mvprintw(i+x, 0+y, "x");
    mvprintw(i+x, w + 1+y, "x");
  }
}

void server_interface::draw_scores() {
    int row = 0;

    mvprintw(1, WIDTH + 9, "LEADERBOARD");
    mvprintw(2, WIDTH + 8, "SNAKE | SCORE");

    // Create a vector to store snakes and their points
    std::vector<std::pair<char, int>> snake_points;

    // Fill the vector with snake characters and points
    for (const auto& snakee : snakes) {
        char snake_char = snakee.first;
        int points = snakee.second.points;
        snake_points.emplace_back(snake_char, points);
    }

    // Sort the vector by points in descending order
    std::sort(snake_points.begin(), snake_points.end(),
              [](const std::pair<char, int>& a, const std::pair<char, int>& b) {
                  return b.second < a.second;  // Sort by points descending
              });

    // Print the sorted scores
    for (const auto& snake : snake_points) {
        char snake_char = snake.first;
        int points = snake.second;
        mvprintw(row + 3, WIDTH + 10, "%c      %d", snake_char, points);
        ++row;
    }
}

void server_interface::draw_snakes() {
  for (const auto& snake : snakes)
  if (!snake.second.game_over)
  for (const auto& segment : snake.second.body)
  mvprintw(segment.y + 1, segment.x + 1, "%c", snake.first);
}

void server_interface::init_snake(snake& snk, char ch){
  snk.body.clear();
  for (int i = 0; i < 5; ++i) 
  snk.body.push_back({WIDTH / 2, HEIGHT / 2 - i});
  snk.ch = ch;
  snk.direction = {0, 1};
  snk.game_over = false;
  snk.points = 0;
}

server_interface::server_interface() {}

server_interface::~server_interface() {}

void server_interface::update_screen() {
  for (auto& snakee: snakes) {
    point new_head =  {snakee.second.body.front().x + snakee.second.direction.x, snakee.second.body.front().y + snakee.second.direction.y};
    if (check_collision(snakee.second)) {
      snakee.second.game_over = true;
      snakee.second.body.clear();
    }
    snakee.second.body.insert(snakee.second.body.begin(), new_head);
    snakee.second.body.pop_back();
  }
}

void server_interface::start_screen() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  timeout(100);
} 

void server_interface::draw_screen() {
  clear();
  draw_border(HEIGHT,WIDTH);
  draw_border(HEIGHT/2,WIDTH/2,0,WIDTH+4);
  draw_snakes();
  draw_scores();
  refresh();
}

board server_interface::draw_board() {
  board new_board(HEIGHT, std::vector<char>(WIDTH, ' '));

  for (const auto& snakee : snakes) {
    if (!snakee.second.game_over) {
      for (const auto& segment : snakee.second.body) {
        new_board[segment.y][segment.x] = snakee.first;
      }
    }
  }

  return new_board;
}