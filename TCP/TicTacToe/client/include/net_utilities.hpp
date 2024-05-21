#ifndef NET_UTILITIES_HPP
#define NET_UTILITIES_HPP

#include <string>
#include <vector>

namespace net {
    std::string format_size(const int, int);
    std::vector<std::vector<unsigned char>> readAndDivideFile(const std::string&, size_t);
    class PROTOCOL{
        public:
        std::string ErrorMessage(int);
        std::string OkMessage();
        std::string LoginMessage(std::string, std::string);
        std::string LogoutMessage();
        std::string ListMessage();
        std::string BroadcastMessage(std::string);
        std::string PrivateMessage(std::string, std::string);
        std::vector<std::string> FileMessages(std::string, std::string);
        std::string TicTacToeMessage(std::string);
    };
    static std::string helpMessage = R"(
    Usage:
    - Broadcast message: <message>
    Example: Hello everyone!

    - Private message: @<receiver> <message>
    Example: @John Hey, how are you?

    - List of users: .list
    Example: .list

    - File transfer: .file <file_name> <receiver>
    Example: .file document.txt Alice

    - TicTacToe: .ttt <command>
    Example: .ttt X1

    - Help message: .help

    Note:
    - Make sure to use proper syntax for each command.
    - Replace <message>, <receiver>, and <file_name> with your actual message, recipient, and file name respectively.
    - Private messages should be addressed to a specific user using @<receiver>.
    - TicTacToe commands include player (X or O) and position (1 - 9).
    )";
}

#endif // NET_UTILITIES_HPP
