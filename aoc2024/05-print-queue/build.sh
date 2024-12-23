#!/bin/bash
if [ "$1" = "gif" ]; then
	./print_queue $2
	gifsicle -d4 print_queue.gif -o out.gif --colors 64 --loopcount=forever -O2
else
	clang -g -Og -Wall -o print_queue print_queue.c
fi

