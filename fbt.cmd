@echo off
call "%~dp0scripts\toolchain\fbtenv.cmd" env

set SCONS_EP=python -m SCons

if [%FBT_NO_SYNC%] == [] (
    if exist ".git" (
        git submodule update --init
    ) else (
        echo Not in a git repo, please clone with git clone --recursive
        exit /b 1
    )
)

set "SCONS_DEFAULT_FLAGS=--warn=target-not-built"

if not defined FBT_VERBOSE (
    set "SCONS_DEFAULT_FLAGS=%SCONS_DEFAULT_FLAGS% -Q"
)

%SCONS_EP% %SCONS_DEFAULT_FLAGS% %*
