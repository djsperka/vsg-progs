rem 
rem call with command line in Custom Build Step
rem
rem call aaDeploy.bat "dir where bin is"  "relative path to exe files" "dir for output zip"
rem 
rem e.g.
rem call aaDeploy.bat "E:\work\vsg-progs\AlertRig" x64\Release e:\work\vsg-progs\AlertRig
rem 
rem I am only copying three exe files and demo/. To add another to deployment zip file
rem just change the 7z command line.
rem

@echo off
pushd %~dp0

rem get output of git describe
for /f %%i in ('git describe') do set VER=%%i
cd %1
copy bin\%2\fixstim.exe bin 
copy bin\%2\remote.exe bin 
copy bin\%2\CableTest.exe bin
7z a %3\alertrig-%VER%.zip bin\fixstim.exe bin\CableTest.exe bin\remote.exe
del bin\fixstim.exe 
del bin\remote.exe 
del bin\CableTest.exe
popd