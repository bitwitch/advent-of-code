EXE        := beacon
INPUT_FILE := input_small.txt
GIF_DELAY  := 4

.PHONY: $(EXE)

$(EXE): 
	gcc -g  -Wall -Wextra -pedantic -o $(EXE) $(EXE).c -lm

.PHONY: $(EXE).gif
$(EXE).gif: $(EXE)
	./$(EXE) $(INPUT_FILE)

.PHONY: everything
everything: $(EXE).gif
	gifsicle -d$(GIF_DELAY) $(EXE).gif -o out.gif -O3 --loopcount=forever

