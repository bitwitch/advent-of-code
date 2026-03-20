@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\entrance.exe input_small.txt
	gifsicle -d7 entrance.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0entrance.c"
	popd
)

