@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\resonant.exe %2
	gifsicle -d9 resonant.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0resonant.c"
	popd
)
