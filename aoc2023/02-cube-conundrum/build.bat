@echo off
if not exist build\ mkdir build
pushd build
cl /Zi /W4 /WX /nologo "%~dp0cubes.c"
popd build
