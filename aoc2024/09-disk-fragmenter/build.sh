#!/bin/bash
if [ "$1" = "gif" ]; then
	./disk $2
	gifsicle -d7 disk.gif -o out.gif --colors 128 --loopcount=forever -O2
else 
	clang -g -Og -Wall -Wextra -pedantic disk.c -o disk
fi

