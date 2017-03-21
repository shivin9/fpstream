#!/bin/bash
read -p "Enter file name: " name
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./exe $name
