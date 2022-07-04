@echo off

set SCONS_EP=%~dp0\lib\scons\scripts\scons.py

if exist ".git" (
	git submodule update --init
)

set "SCONS_DEFAULT_FLAGS=-Q --warn=target-not-built"
python %SCONS_EP% %SCONS_DEFAULT_FLAGS% %*
