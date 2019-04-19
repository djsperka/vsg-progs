@echo off
set mypath=%~dp0
set ROOT=%mypath%..\
set REMOTE_EXE=%ROOT%AlertRig\bin64\remote.exe
echo "script path is " %mypath%
echo Project root %ROOT%
%REMOTE_EXE% 127.0.0.1 7000 quit