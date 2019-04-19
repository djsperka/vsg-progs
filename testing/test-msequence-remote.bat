@echo off
set mypath=%~dp0
set ROOT=%mypath%..\
set REMOTE_EXE=%ROOT%AlertRig\bin64\remote.exe
echo "script path is " %mypath%
echo Project root %ROOT%
rem -K for testing loop
rem %REMOTE_EXE% 127.0.0.1 7000 msequence -v -p 0,0 -d 24 -t 55 -o 15 -r 16 -c 16 -m c:/work/usrig/stim/MSequenceTerms -T 0,20 -a
%REMOTE_EXE% 127.0.0.1 7000 msequence -v -t 1 -d 16 -p 0,0 -R 1 -r 16 -c 16