#!/bin/bash
read -p "Enter command: " cmd
valgrind --tool=massif $cmd
