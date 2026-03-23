@echo off

pushd ..\build\

clang-cl -o backupUtil.exe .\..\src\backupUtil.c -Wall -g -m64 -Wno-unused-parameter -Wno-unused-macros  -Wno-declaration-after-statement -Wno-unsafe-buffer-usage -Wno-unused-variable

popd

