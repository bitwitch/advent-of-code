#!/bin/bash
if [ "$1" = "gif" ]; then
	./disk $2
	gifsicle -d9 disk.gif -o out.gif --colors 16 --loopcount=forever -O2
else 
	clang -g -Og -Wall -Wextra -pedantic disk.c -o disk
fi

