# NET
UDP PROGRAMMING

how to compile (linux)

    mkdir build
    cd build
    cmake -G "Unix Makefiles" ../
    cmake --build . --config Release --target client
    ./client port username
    
how to compile (windows)

    www.google.com

how to use

    to send a message
    <port> <message>

    messages are received in this format
    (<username>@<ip>:<port>): <message>
