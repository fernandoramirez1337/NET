#ifndef TICTACTOE_HPP
#define TICTACTOE_HPP

#include <string>

class TicTacToe {
private:
    char board[3][3];
    char currentPlayer;

public:
    TicTacToe(char startingPlayer);

    std::string makeMove(const std::string& move);
    std::string getBoardAsString() const;
    bool checkWin() const;
    bool isBoardFull() const;
    void reset(char startingPlayer);
    char getCurrentPlayer() const;
};

#endif
