@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\labs.exe input.txt
	gifsicle -d4 labs.gif -o out.gif --colors 8 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0labs.c"
	popd
)

