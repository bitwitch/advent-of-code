#!/bin/bash
if [ "$1" = "gif" ]; then
	./guard $2
	gifsicle -d4 guard.gif -o out.gif --colors 16 --loopcount=forever -O2
else 
	clang -g -Og -Wall -Wextra -pedantic guard.c -o guard
fi

