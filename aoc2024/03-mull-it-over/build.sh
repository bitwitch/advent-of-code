#!/bin/bash
if [ "$1" == "gif" ]; then
	./mull $2
	gifsicle -d7 mull.gif -o out.gif --colors 64 --loopcount=forever -O2
else
	clang -g -Og -Wall -o mull mull.c
fi
