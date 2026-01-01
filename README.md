# NET

A collection of network programming examples and projects demonstrating socket programming concepts in C, C++, and Python.

## Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Building and Running](#building-and-running)
  - [TCP Examples](#tcp-examples)
  - [UDP Examples](#udp-examples)
  - [NCURSES Examples](#ncurses-examples)
  - [CONTROL_LAB](#control_lab)

## Overview

This repository contains various network programming examples covering:

- **TCP Socket Programming**: Client-server implementations including a multi-client chat application and a networked TicTacToe game
- **UDP Socket Programming**: Datagram-based communication examples
- **NCURSES Integration**: Terminal UI applications with network capabilities
- **HTTP Basics**: Simple HTTP request/response handling

## Project Structure

```
NET/
├── TCP/
│   ├── 01/          # Basic TCP chat (C)
│   ├── 02/          # Basic TCP chat (C++)
│   ├── 03/          # TCP with HTTP examples
│   ├── 04/          # TCP client/server (CMake)
│   ├── 05/          # TCP client/server (CMake)
│   └── TicTacToe/   # Networked TicTacToe game
├── UDP/
│   └── 01/          # UDP client/server
├── NCURSES/
│   ├── 01/          # NCURSES client/server
│   ├── 01.1/        # NCURSES variant
│   └── 02/          # NCURSES client/server
└── CONTROL_LAB/     # Control lab client
```

## Prerequisites

- **Compiler**: GCC or G++ with C11/C++17 support
- **Build Tools**: CMake (for projects in TCP/04, TCP/05, TCP/TicTacToe, UDP, NCURSES)
- **Libraries**:
  - POSIX threads (`pthread`)
  - NCURSES (for NCURSES examples)
- **Python 3** (for HTTP scripts in TCP/03)

### Installing Dependencies

**Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libncurses5-dev
```

**Fedora:**
```bash
sudo dnf install gcc g++ cmake ncurses-devel
```

## Building and Running

### TCP Examples

#### TCP/01 - Basic Chat (C)
```bash
cd TCP/01
gcc server.c -lpthread -o server.exe
gcc client.c -lpthread -o client.exe

# Run server
./server.exe <port>

# Run client (in another terminal)
./client.exe <hostname> <port> <username>
```

#### TCP/02 - Basic Chat (C++)
```bash
cd TCP/02
g++ server.cpp -lpthread -o server.exe
g++ client.cpp -lpthread -o client.exe

# Run server
./server.exe <port>

# Run client (in another terminal)
./client.exe <hostname> <port> <username>
```

#### TCP/03 - TCP with HTTP
```bash
cd TCP/03
g++ server.cpp -lpthread -o server.exe
g++ client.cpp -lpthread -o client.exe
```

#### TCP/04, TCP/05, TCP/TicTacToe - CMake Projects

For each of these projects (04, 05, or TicTacToe), build both client and server:

```bash
# Build server
cd TCP/04/server  # or TCP/05/server, TCP/TicTacToe/server
mkdir build && cd build
cmake .. && make

# Build client
cd ../../client
mkdir build && cd build
cmake .. && make
```

### UDP Examples

#### UDP/01
```bash
cd UDP/01/server
mkdir build && cd build
cmake .. && make

cd ../../client
mkdir build && cd build
cmake .. && make

# Run using the test script
cd ../..
./run_test.sh
```

### NCURSES Examples

#### NCURSES/01, 01.1, 02
```bash
cd NCURSES/<version>/server  # or /client
mkdir build && cd build
cmake .. && make
```

### CONTROL_LAB

```bash
cd CONTROL_LAB
g++ main.cpp -lpthread -o client.exe

# Run
./client.exe
```

## Usage Examples

### Running a TCP Chat Server and Client

1. Start the server:
```bash
./server.exe 8080
# Output: server: waiting for connections...      PORT = 8080
```

2. Connect with clients:
```bash
./client.exe localhost 8080 Alice
./client.exe localhost 8080 Bob
```

3. Type messages in client terminals to chat. Type `end` to disconnect.

### TicTacToe Game

The TicTacToe project implements a networked two-player game. Build both client and server components, then:

1. Start the server on a chosen port
2. Connect two clients to play against each other
