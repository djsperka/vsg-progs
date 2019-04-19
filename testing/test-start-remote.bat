@echo off
set mypath=%~dp0
set ROOT=%mypath%..\
set FIXSTIM_EXE=%ROOT%AlertRig\bin64\fixstim.exe
echo "script path is " %mypath%
echo Project root %ROOT%
%FIXSTIM_EXE% -b gray -d 555 -u 127.0.0.1:7000