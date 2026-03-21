@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\printing.exe input.txt
	gifsicle printing.gif -d2 "#0--17" -d8 "#-16-" -o out.gif --colors 8 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0printing.c"
	popd
)

