@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\guard.exe %2
	gifsicle -d3 guard.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0guard.c"
	popd
)
