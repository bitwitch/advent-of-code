#!/bin/bash
if [ "$1" = "gif" ]; then
	./pluto $2
	gifsicle -d7 pluto.gif -o out.gif --colors 64 --loopcount=forever -O2
else 
	clang -g -Wall -Wextra -pedantic pluto.c -o pluto
fi

