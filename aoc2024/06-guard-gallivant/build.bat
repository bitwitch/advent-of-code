@echo off
if not exist build\ mkdir build
pushd build
	cl /Zi /W3 /WX /nologo "%~dp0guard.c"
popd
