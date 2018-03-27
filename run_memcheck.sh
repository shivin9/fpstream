#!/bin/bash
read -p "Enter command: " cmd
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $cmd
