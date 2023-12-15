@echo off
setlocal enabledelayedexpansion

set ORCA_DIR=C:\Users\Ian\code\third-party\orca
set STDLIB_DIR=%ORCA_DIR%\src\libc-shim

set "src_dir=%~dp0"
set "src_dir=!src_dir:~0,-1!"

set wasm_flags=--target=wasm32^
   --no-standard-libraries ^
   -mbulk-memory ^
   -g -O2 ^
   -D__ORCA__ ^
   -Wl,--no-entry ^
   -Wl,--export-dynamic ^
   -isystem %STDLIB_DIR%\include ^
   -I%ORCA_DIR%\src ^
   -I%ORCA_DIR%\src\ext

if not exist build\ mkdir build
pushd build

if not exist liborca.a (
   clang %wasm_flags% -Wl,--relocatable -o .\liborca.a %ORCA_DIR%\src\orca.c %STDLIB_DIR%\src\*.c
   IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
)

clang -DORCA_MODE %wasm_flags% -o .\module.wasm  -L . -lorca "%src_dir%\cubes.c"
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

orca bundle --orca-dir %ORCA_DIR% --name "Cubes" --icon icon.png --resource-dir "%src_dir%\data" module.wasm
popd

