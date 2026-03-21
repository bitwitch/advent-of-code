@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\cafeteria.exe input.txt
	gifsicle -d2 cafeteria.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0cafeteria.c"
	popd
)

