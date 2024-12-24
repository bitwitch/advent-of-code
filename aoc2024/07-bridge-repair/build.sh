#!/bin/bash
if [ "$1" = "gif" ]; then
	./bridge $2
	gifsicle -d9 bridge.gif -o out.gif --colors 16 --loopcount=forever -O2
else 
	clang -g -Og -Wall -Wextra -pedantic bridge.c -o bridge
fi

