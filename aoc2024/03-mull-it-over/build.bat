@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\mull.exe %2
	gifsicle -d4 mull.gif -o out.gif --colors 64 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0mull.c"
	popd
)
