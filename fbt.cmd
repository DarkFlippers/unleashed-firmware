@echo off
call "%~dp0scripts\toolchain\fbtenv.cmd" env || exit /b

set SCONS_EP=python -m SCons

if [%FBT_NO_SYNC%] == [] (
    set _FBT_CLONE_FLAGS=--jobs %NUMBER_OF_PROCESSORS%
    if not [%FBT_GIT_SUBMODULE_SHALLOW%] == [] (
        set _FBT_CLONE_FLAGS=%_FBT_CLONE_FLAGS% --depth 1
    )
    if exist ".git" (
        git submodule update --init --recursive %_FBT_CLONE_FLAGS%
        if %ERRORLEVEL% neq 0 (
            echo Failed to update submodules, set FBT_NO_SYNC to skip
            exit /b 1
        )
    ) else (
        echo .git not found, please clone repo with "git clone"
        exit /b 1
    )
)

set "SCONS_DEFAULT_FLAGS=--warn=target-not-built"

if not defined FBT_VERBOSE (
    set "SCONS_DEFAULT_FLAGS=%SCONS_DEFAULT_FLAGS% -Q"
)

%SCONS_EP% %SCONS_DEFAULT_FLAGS% %*
