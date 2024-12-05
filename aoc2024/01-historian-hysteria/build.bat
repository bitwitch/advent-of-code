@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\historian.exe input.txt
	gifsicle -d2 historian_large.gif -o out.gif --colors 8 --loopcount=forever -O2
	REM build\historian.exe input_small.txt
	REM gifsicle -d2 historian_small.gif -o out.gif --colors 8 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0historian.c"
	popd
)

