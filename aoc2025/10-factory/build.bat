@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\factory.exe input.txt
	gifsicle -d10 factory.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0factory.c"
	popd
)

