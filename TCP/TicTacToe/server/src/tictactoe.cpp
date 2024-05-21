#include "../include/tictactoe.hpp"

#include <iostream>
#include <string>

TicTacToe::TicTacToe(char startingPlayer) : currentPlayer(startingPlayer){ reset(startingPlayer); }

std::string TicTacToe::makeMove(const std::string& move) {
    std::string output = "\n";
    if (move.size() != 2) {
        output += move;
        output += "Invalid move format. Please use format 'PM', where P is player (X or O) and M is move (1-9).";
        return output;
    }

    char player = move[0];
    char cell = move[1];

    if (player != currentPlayer) {
        output += "It's not ";
        output += player;
        output += "'s turn.";
        return output;
    }

    if (cell < '1' || cell > '9') {
        output += "Invalid move. Move must be a number between 1 and 9.";
        return output;
    }

    int row = (cell - '1') / 3;
    int col = (cell - '1') % 3;

    if (board[row][col] != ' ') {
        output += "Invalid move. Cell already occupied.";
        return output;
    }

    board[row][col] = player;
    if (checkWin()) {
        output += "Player ";
        output += player;
        output += " wins!\n";
        output += getBoardAsString();
        reset(currentPlayer);
    } else if (isBoardFull()) {
        output += "It's a draw!\n";
        output += getBoardAsString();
        reset(currentPlayer);
    } else {
        output += getBoardAsString();
    }
    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    return output;
}

std::string TicTacToe::getBoardAsString() const {
    std::string boardStr;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            boardStr += board[i][j];
            if (j < 2) boardStr += "|"; 
        }
        if (i < 2) boardStr += "\n-+-+-\n";
    }
    return boardStr;
}

bool TicTacToe::checkWin() const {
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][0] == board[i][2]) return true; // Row
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[0][i] == board[2][i]) return true; // Column
    }
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[0][0] == board[2][2]) return true; // Diagonal 1
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[0][2] == board[2][0]) return true; // Diagonal 2
    return false;
}

bool TicTacToe::isBoardFull() const {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') return false;
        }
    }
    return true;
}

void TicTacToe::reset(char startingPlayer) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            board[i][j] = ' ';
        }
    }
    currentPlayer = startingPlayer;
}

char TicTacToe::getCurrentPlayer() const {
    return currentPlayer;
}
