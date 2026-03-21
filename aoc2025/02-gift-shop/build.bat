@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\gift_shop.exe input.txt
	gifsicle -d2 gift_shop.gif -o out.gif --colors 4 --loopcount=forever -O3
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0gift_shop.c"
	popd
)

