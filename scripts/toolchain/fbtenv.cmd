@echo off

if not [%FBT_ROOT%] == [] (
    goto already_set
)

set "FBT_ROOT=%~dp0\..\..\"
pushd %FBT_ROOT%
set "FBT_ROOT=%cd%"
popd

if not [%FBT_NOENV%] == [] (
    exit /b 0
)

set "FLIPPER_TOOLCHAIN_VERSION=8"
set "FBT_TOOLCHAIN_ROOT=%FBT_ROOT%\toolchain\i686-windows"


if not exist "%FBT_TOOLCHAIN_ROOT%" (
    powershell -ExecutionPolicy Bypass -File "%FBT_ROOT%\scripts\toolchain\windows-toolchain-download.ps1" "%flipper_toolchain_version%"
)
if not exist "%FBT_TOOLCHAIN_ROOT%\VERSION" (
    powershell -ExecutionPolicy Bypass -File "%FBT_ROOT%\scripts\toolchain\windows-toolchain-download.ps1" "%flipper_toolchain_version%"
)
set /p REAL_TOOLCHAIN_VERSION=<"%FBT_TOOLCHAIN_ROOT%\VERSION"
if not "%REAL_TOOLCHAIN_VERSION%" == "%FLIPPER_TOOLCHAIN_VERSION%" (
    powershell -ExecutionPolicy Bypass -File "%FBT_ROOT%\scripts\toolchain\windows-toolchain-download.ps1" "%flipper_toolchain_version%"
)


set "HOME=%USERPROFILE%"
set "PYTHONHOME=%FBT_TOOLCHAIN_ROOT%\python"
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
