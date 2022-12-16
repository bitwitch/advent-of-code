EXE := rope

.PHONY: $(EXE)

$(EXE): 
	gcc -g -Og -Wall -Wextra -pedantic -o $(EXE) $(EXE).c -lm

.PHONY: $(EXE).gif
$(EXE).gif: $(EXE)
	./$(EXE) input_mid.txt

.PHONY: everything
everything: $(EXE).gif
	gifsicle -d2 $(EXE).gif -o out.gif -O3 --loopcount=forever

