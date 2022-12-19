EXE        := crt
INPUT_FILE := input.txt
GIF_DELAY  := 2

.PHONY: $(EXE)

$(EXE): 
	gcc -g -Og -Wall -Wextra -pedantic -o $(EXE) $(EXE).c -lm

.PHONY: $(EXE).gif
$(EXE).gif: $(EXE)
	./$(EXE) $(INPUT_FILE)

.PHONY: everything
everything: $(EXE).gif
	gifsicle -d$(GIF_DELAY) $(EXE).gif -o out.gif -O3 --loopcount=forever

