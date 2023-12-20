@echo off
if not exist build\ mkdir build
pushd build

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


if "%1" == "game" (
	if not exist liborca.a (
		clang %wasm_flags% -Wl,--relocatable -o .\liborca.a %ORCA_DIR%\src\orca.c %STDLIB_DIR%\src\*.c
		IF %ERRORLEVEL% NEQ 0 (
			popd
			EXIT /B %ERRORLEVEL%
		)
	)

	clang -DORCA_MODE %wasm_flags% -o .\module.wasm  -L . -lorca "%src_dir%\race_game.c"
	IF %ERRORLEVEL% NEQ 0 (
		popd
		EXIT /B %ERRORLEVEL%
	)

	orca bundle --orca-dir %ORCA_DIR% --name "Game" --icon icon.png --resource-dir "%src_dir%\data" module.wasm

) else ( 
	cl /Zi /W3 /nologo "%~dp0race.c"
)
popd 
