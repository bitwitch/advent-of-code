@echo off
if not exist build\ mkdir build
pushd build
if "%1" == "gif" (
	gifsicle -d3 "%~dp0race.gif" -o "%~dp0out.gif" -O3 --loopcount=forever
) else ( 
	cl /Zi /W3 /nologo "%~dp0race.c"
)
popd build
