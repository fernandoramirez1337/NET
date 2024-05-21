#!/bin/bash

# Open the server executable in a new terminal
gnome-terminal -- bash -c "./server/build/server 1337; exec bash" &

# Open the client executables in new terminals
gnome-terminal -- bash -c "./client/build/client localhost 1337; exec bash" &
gnome-terminal -- bash -c "./client/build/client localhost 1337; exec bash" &
gnome-terminal -- bash -c "./client/build/client localhost 1337; exec bash" &
