#!/bin/bash
read -p "Enter file name: " name
valgrind --tool=massif ./exe $name
