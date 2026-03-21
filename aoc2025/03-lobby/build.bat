@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\lobby.exe input_small.txt
	gifsicle -d7 lobby.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0lobby.c"
	popd
)

