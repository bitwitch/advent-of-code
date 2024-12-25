@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\disk.exe %2
	gifsicle -d9 disk.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0disk.c"
	popd
)
