#!/bin/bash
if [ "$1" = "gif" ]; then
	./hoof $2
	gifsicle -d7 hoof.gif -o out.gif --colors 64 --loopcount=forever -O2
else 
	clang -g -Wall -Wextra -pedantic hoof.c -o hoof
fi

