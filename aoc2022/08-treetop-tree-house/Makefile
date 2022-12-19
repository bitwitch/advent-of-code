EXE := tree_house

$(EXE): $(EXE).c
	gcc -g -Og -Wall -Wextra -pedantic -o $(EXE) $(EXE).c -lm

.PHONY: gif
gif: $(EXE)
	./$(EXE) input.txt

.PHONY: optimize
optimize: $(EXE).gif 
	gifsicle -d6 $(EXE).gif -o out.gif --colors 8 --loopcount=forever
