#!/bin/bash
if [ "$1" = "gif" ]; then
	./hoof $2
	gifsicle -d7 hoof.gif -o out.gif --colors 128 --loopcount=forever -O2
else 
	clang -g -Og -Wall -Wextra -pedantic hoof.c -o hoof
fi

