@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\reports.exe %2
	gifsicle -d4 reports.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /wd4146 /nologo "%~dp0reports.c"
	popd
)

