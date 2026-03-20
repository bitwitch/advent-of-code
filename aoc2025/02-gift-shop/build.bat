@echo off
if not exist build\ mkdir build
if "%1" == "gif" (
	build\gift_shop.exe input_small.txt
	gifsicle -d7 gift_shop.gif -o out.gif --colors 16 --loopcount=forever -O2
) else (
	pushd build
		cl /Zi /W3 /WX /nologo "%~dp0gift_shop.c"
	popd
)

