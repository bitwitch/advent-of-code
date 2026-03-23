@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\playground.exe input.txt
	gifsicle -d7 playground.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0playground.c"
	popd
)

