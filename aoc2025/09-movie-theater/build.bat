@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\theater.exe input.txt
	gifsicle -d2 theater.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0theater.c"
	popd
)

