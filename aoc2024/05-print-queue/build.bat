@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\print_queue.exe %2
	gifsicle -d4 print_queue.gif -o out.gif --colors 64 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0print_queue.c"
	popd
)
