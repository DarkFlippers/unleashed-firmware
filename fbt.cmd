@echo off
if exist ".git" (
	echo Prepairing git submodules
	git submodule update --init
)

set "SCONS_DEFAULT_FLAGS=-Q --warn=target-not-built"
python lib/scons/scripts/scons.py %SCONS_DEFAULT_FLAGS% %*
