@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\hoof.exe %2
	gifsicle -d7 hoof.gif -o out.gif --colors 64 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0hoof.c"
	popd
)
