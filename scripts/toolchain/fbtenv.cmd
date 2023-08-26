@echo off

if not ["%FBT_ROOT%"] == [""] (
    goto already_set
)

set "FBT_ROOT=%~dp0\..\..\"
pushd "%FBT_ROOT%"
set "FBT_ROOT=%cd%"
popd

if not ["%FBT_NOENV%"] == [""] (
    exit /b 0
)

set "FLIPPER_TOOLCHAIN_VERSION=23"

if ["%FBT_TOOLCHAIN_PATH%"] == [""] (
    set "FBT_TOOLCHAIN_PATH=%FBT_ROOT%"
)

set "FBT_TOOLCHAIN_ROOT=%FBT_TOOLCHAIN_PATH%\toolchain\x86_64-windows"

set "FBT_TOOLCHAIN_VERSION_FILE=%FBT_TOOLCHAIN_ROOT%\VERSION"

if not exist "%FBT_TOOLCHAIN_ROOT%" (
    powershell -ExecutionPolicy Bypass -File "%FBT_ROOT%\scripts\toolchain\windows-toolchain-download.ps1" %flipper_toolchain_version% "%FBT_TOOLCHAIN_ROOT%"
)

if not exist "%FBT_TOOLCHAIN_VERSION_FILE%" (
    powershell -ExecutionPolicy Bypass -File "%FBT_ROOT%\scripts\toolchain\windows-toolchain-download.ps1" %flipper_toolchain_version% "%FBT_TOOLCHAIN_ROOT%"
)

set /p REAL_TOOLCHAIN_VERSION=<"%FBT_TOOLCHAIN_VERSION_FILE%"
if not "%REAL_TOOLCHAIN_VERSION%" == "%FLIPPER_TOOLCHAIN_VERSION%" (
    echo FBT: starting toolchain upgrade process..
    powershell -ExecutionPolicy Bypass -File "%FBT_ROOT%\scripts\toolchain\windows-toolchain-download.ps1" %flipper_toolchain_version% "%FBT_TOOLCHAIN_ROOT%"
    set /p REAL_TOOLCHAIN_VERSION=<"%FBT_TOOLCHAIN_VERSION_FILE%"
)

if defined FBT_VERBOSE (
    echo FBT: using toolchain version %REAL_TOOLCHAIN_VERSION%
)

set "HOME=%USERPROFILE%"
set "PYTHONHOME=%FBT_TOOLCHAIN_ROOT%\python"
set "PYTHONPATH="
set "PYTHONNOUSERSITE=1"
set "PATH=%FBT_TOOLCHAIN_ROOT%\python;%FBT_TOOLCHAIN_ROOT%\bin;%FBT_TOOLCHAIN_ROOT%\protoc\bin;%FBT_TOOLCHAIN_ROOT%\openocd\bin;%PATH%"
set "PROMPT=(fbt) %PROMPT%"

:already_set

if not "%1" == "env" (
    echo *********************************
    echo *     fbt build environment     *
    echo *********************************
    cd "%FBT_ROOT%"
    cmd /k
)
