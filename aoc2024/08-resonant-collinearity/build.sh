#!/bin/bash
if [ "$1" = "gif" ]; then
	./resonant $2
	gifsicle -d7 resonant.gif -o out.gif --colors 32 --loopcount=forever -O2
else 
	clang -g -Og -Wall -Wextra -pedantic resonant.c -o resonant
fi

