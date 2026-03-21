@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\printing.exe input.txt
	gifsicle -d7 printing.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0printing.c"
	popd
)

