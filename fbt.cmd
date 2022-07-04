@echo off

set SCONS_EP=%~dp0\lib\scons\scripts\scons.py

if [%FBT_NO_SYNC%] == [] (
	if exist ".git" (
		git submodule update --init
	) else (
		echo Not in a git repo, please clone with git clone --recursive
		exit /b 1
	)
)
set "flipper_toolchain_version=2"
set "toolchainRoot=%~dp0toolchain\i686-windows"
set "SCONS_DEFAULT_FLAGS=-Q --warn=target-not-built"

if not exist "%toolchainRoot%" (
    powershell -ExecutionPolicy Bypass -File %~dp0scripts\toolchain\windows-toolchain-download.ps1 "%flipper_toolchain_version%"
)
if not exist "%toolchainRoot%\VERSION" (
    powershell -ExecutionPolicy Bypass -File %~dp0scripts\toolchain\windows-toolchain-download.ps1 "%flipper_toolchain_version%"
)
set /p real_toolchain_version=<%toolchainRoot%\VERSION
if not "%real_toolchain_version%" == "%flipper_toolchain_version%" (
    powershell -ExecutionPolicy Bypass -File %~dp0scripts\toolchain\windows-toolchain-download.ps1 "%flipper_toolchain_version%"
)
cmd /V /C "set "PATH=%toolchainRoot%\python;%toolchainRoot%\bin;%toolchainRoot%\protoc\bin;%toolchainRoot%\openocd\bin;%PATH%" && python lib\scons\scripts\scons.py %SCONS_DEFAULT_FLAGS% %*"