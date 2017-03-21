#!/bin/bash
read -p "Enter file name: " name
valgrind --tool=massif --pages-as-heap=yes ./exe $name
