@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\trash.exe input.txt
	gifsicle -d7 trash.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0trash.c"
	popd
)

