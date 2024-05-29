#!/bin/bash

DIR=$(dirname "$0")

# Start the Node.js frontend
gnome-terminal -- bash -c "cd $DIR/opengl-visualizer; npm i; npm run dev; exec bash"

# Start the C++ backend
gnome-terminal -- bash -c "cd $DIR/cpp-backend/build; ./cpp-backend; exec bash"