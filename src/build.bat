@echo off

pushd ../build/

clang-cl -o backupUtil.exe backupUtil.c -Wall -g -m64 -Wno-unused-parameter -Wno-unused-macros  -Wno-declaration-after-statement

popd

